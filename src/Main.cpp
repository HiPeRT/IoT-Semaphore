#include <signal.h>
#include <pthread.h>
#include <cstdlib>
#include <string>
#include <fstream>
#include <vector>
#include <ctime>
#include <chrono>

//ITI - includes for UART
#define UART_BL5

#ifdef	UART_BL5
	#include <unistd.h> 
	#include <fcntl.h> 
	#include <termios.h>
	#include <errno.h>
#endif
//

#include </usr/include/rapidjson/document.h>

#include "Gpio.h"
#include "MasaProtocolHandler.h"

//#define MODBUS

#ifdef MODBUS
	#include "ModbusHandler.h"
#endif

using namespace rapidjson;


std::vector<Semaphore *> S;

pthread_t tlWorkers[4];
pthread_t internalWorker, serverWorker, pinsWorker, modbusWorker, masaWorker;
pthread_mutex_t lckStdout, lckValues;

#ifdef MODBUS
	ModbusHandler *mbh;
#endif

MasaProtocolHandler *mph;

uint32_t d_r, d_g,  d_y;

bool modbus_init;		// se modbus e inizializzato
uint16_t ctrl_type;		// tipo di controllo dei semafori
bool use_server;		// manda/ricevi info dal server
bool use_masa;

uint16_t tlStateSrv, ctrlTypeSrv;

uint16_t tlStateInternal[2] { SemaphoreState::Green, SemaphoreState::Red };
uint32_t tlCountdownInternal[2] = { d_g, d_r };

#ifdef UART_BL5
	//ITI///
	int uart0 = -1;
	/*
	* ITI UART initialization 
	*/
	int8_t InitUART(void) {
		uart0 = open("/dev/ttyACM0", O_RDWR | O_NOCTTY | O_NDELAY); 
		if (uart0 == -1){
			pthread_mutex_lock(&lckStdout);
			std::cout << "------------ UART ERROR ----------" << std::endl;
			pthread_mutex_unlock(&lckStdout);	
			return -1;
		}
		// UART configuration 
		struct termios options;
		tcgetattr(uart0, &options);
		options.c_cflag = B115200 | CS8 | CLOCAL | CREAD; // set baudrate
		options.c_iflag = IGNPAR;
		options.c_oflag = 0;
		options.c_lflag = 0;
		tcflush(uart0, TCIFLUSH);
		tcsetattr(uart0, TCSANOW, &options);
		return 0;
	}	
	/*
	* ITI - UART TX traffic light information
	*/
	void TxUART(uint16_t State,uint16_t TimeState){

		uint8_t tx_buffer[16];
		
		uint8_t *p_tx_buffer;	
		
		//////ITI///////
		/*
		* TX message 
		* {0x02} start byte
		* {0x1?} state byte {0x10}-green {0x11}-green+yellow {0x12}-red
		* {0x??} Time to change 
		* {0xFD} Byte to close 
		*/		
		float TrafficLightLatitude = S[0]->getLatitude();
		uint8_t *TFLaByte = (uint8_t *) &TrafficLightLatitude;
		float TrafficLightLongitude = S[0]->getLongitude();
		uint8_t *TFLoByte = (uint8_t *) &TrafficLightLongitude;
		int TrafficLightOrientation = S[0]->getOrientation();
		uint8_t *TFOByte = (uint8_t *) &TrafficLightOrientation;
		
		p_tx_buffer = &tx_buffer[0];
		*p_tx_buffer++ = 0x02;
		*p_tx_buffer++ = State + 0x0F; 
		*p_tx_buffer++ = TimeState;		
		*p_tx_buffer++ = TFLaByte[3];
		*p_tx_buffer++ = TFLaByte[2];
		*p_tx_buffer++ = TFLaByte[1];
		*p_tx_buffer++ = TFLaByte[0];
		*p_tx_buffer++ = TFLoByte[3];
		*p_tx_buffer++ = TFLoByte[2];
		*p_tx_buffer++ = TFLoByte[1];
		*p_tx_buffer++ = TFLoByte[0];
		*p_tx_buffer++ = TFOByte[1];
		*p_tx_buffer++ = TFOByte[0];
		*p_tx_buffer++ = 0xFD;
			
		if (uart0 != -1)
		{	
			int count = write(uart0, &tx_buffer[0], (p_tx_buffer - &tx_buffer[0]));		//Filestream, bytes to write, number of bytes to write
			if (count < 0)
			{
				pthread_mutex_lock(&lckStdout);
				std::cout << "------------ UART ERROR ----------" << std::endl;
				pthread_mutex_unlock(&lckStdout);
			}
		}
	}
#endif
/////////////////////

/*
 * 0 PHASE
 * 1 TLA1_MODE
 * 2 TLA2_MODE
 * 3 TLB1_MODE
 * 4 TLB2_MODE
 * 5 TLA1_STATE
 * 6 TLA2_STATE
 * 7 TLB1_STATE
 * 8 TLB2_STATE
 */
std::vector<uint16_t> values;

#include "Utils.h"

/*
 * Gestisce l'interruzione del programma
 */
void exiting(int sig) {

	pthread_mutex_lock(&lckStdout);
	std::cout << "\nDisposing..." << std::endl;
	pthread_mutex_unlock(&lckStdout);

	// First, threads
	pthread_cancel(internalWorker);
	for(int i = 0; i < 4; ++i){
		pthread_cancel(tlWorkers[i]);
	}
	if(use_server)
		pthread_cancel(serverWorker);
	
	pthread_cancel(pinsWorker);
	pthread_cancel(modbusWorker);

	// Then, support structures
	pthread_mutex_destroy(&lckStdout);
	pthread_mutex_destroy(&lckValues);
	

	for(int i = 0; i < 4; ++i){
		delete S[i];
	}
	#ifdef MODBUS
		delete mbh;
	#endif

	if(use_masa) {
		delete mph;
	}

	sleep(1);

	pthread_mutex_lock(&lckStdout);
	std::cout << "\nExiting..." << std::endl;
	pthread_mutex_unlock(&lckStdout);

	exit(EXIT_SUCCESS);
}

void * masaHandler(void *args) {

	int sleeptime = 100; 
	
	/////ITI INIT UART//////
	#ifdef UART_BL5
		InitUART();
		pthread_mutex_lock(&lckStdout);
		std::cout << "------------Starting UART----------" << std::endl;
		pthread_mutex_unlock(&lckStdout);
	#endif
	////////////////////////////
	
	while(true) {
		uint16_t phase, tlMode, tlState, ctrlType, timeToChange;

		pthread_mutex_lock(&lckValues);
		ctrlType = ctrlTypeSrv;
		if(ctrlType == SemaphoreCtrl::Modbus) {
			decodeValues(0, &phase, &tlMode, &tlState, &timeToChange);
		}
		else if(ctrlType == SemaphoreCtrl::Server) {
			tlState = tlStateSrv;
			timeToChange = -1;
		}
		else if(ctrlType == SemaphoreCtrl::Internal) {
			tlState = tlStateInternal[0];
			timeToChange = tlCountdownInternal[0];
		}
		#ifdef UART_BL5
			TxUART(tlState, timeToChange);
		#endif
		
		pthread_mutex_unlock(&lckValues);

		Message *m = mph->prepare_message(
			S, tlCountdownInternal, tlStateInternal
		);

		mph->send_message(m);

		usleep(1000 * sleeptime);
	}
}

void * serverHandler(void *args) {
	int sleeptime = 400;

	while(true) {
		uint16_t phase, tlMode, tlState, ctrlType, timeToChange;

		pthread_mutex_lock(&lckValues);
		// We identify with TLA1, to minimize data exchanged
		ctrlType = ctrlTypeSrv;
		if(ctrlType == SemaphoreCtrl::Modbus) {
			decodeValues(0, &phase, &tlMode, &tlState, &timeToChange);
		}
		else if(ctrlType == SemaphoreCtrl::Server) {
			tlState = tlStateSrv;
			timeToChange = -1; // Server knows...
		}
		else if(ctrlType == SemaphoreCtrl::Internal) {
			tlState = tlStateInternal[0];
			timeToChange = tlCountdownInternal[0];
		}
		pthread_mutex_unlock(&lckValues);
		
		// This takes time: out of lock
		uint16_t c = ctrlTypeSrv, s = tlStateSrv;
		S[0]->sendStatus(phase, tlMode, tlState, ctrlType, modbus_init, timeToChange, &s, &c);
		//S[0]->getCtrl(&s, &c);
		
		pthread_mutex_lock(&lckValues);
		tlStateSrv = s;
		ctrlTypeSrv = c;
		
		// Server always decides
		ctrl_type = ctrlTypeSrv;

		// pthread_mutex_lock(&lckStdout);
		// std::cout 	<< "[serverHandler()]"
		// 			<< " ctrlTypeSrv is " << std::to_string(ctrlTypeSrv)
		// 			<< " tlStateSrv is " << std::to_string(tlStateSrv)
		// 			<< std::endl;
	 	// pthread_mutex_unlock(&lckStdout);
		pthread_mutex_unlock(&lckValues);
		
		//std::cout << "[serverHandler(id: " << id << ") Going to sleep for " << sleeptime << " ns" << std::endl;

		usleep(1000 * sleeptime);
	}
}

int internalChangeState(int id, uint16_t *tlStatePtr) {
	int sleeptime = 0;

	pthread_mutex_unlock(&lckStdout);
	switch(*tlStatePtr) {
		case SemaphoreState::Green:
			*tlStatePtr = SemaphoreState::GreenYellow;
			sleeptime = d_y;
			break;
			
		case SemaphoreState::GreenYellow:
			*tlStatePtr = SemaphoreState::Red;
			sleeptime = d_r;
			break;
			
		case SemaphoreState::Red:
		default:
			*tlStatePtr = SemaphoreState::Green;
			sleeptime = d_g;
			break;
	} // switch

	return sleeptime;
}
// TODO Move theese inside Semaphore class?
void *internalCtrl4WaysCrossroadThrd(void * args) {
	int sleeptime = 1000;
	
	/* Initial state */
	tlCountdownInternal[0] = d_g;
	tlCountdownInternal[1] = d_r;
	tlStateInternal[0] = SemaphoreState::Green;
	tlStateInternal[1] = SemaphoreState::Red;

	pthread_mutex_lock(&lckStdout);
	std::cout << "[internalCtrlThrd()] Starting..." << std::endl;
	pthread_mutex_unlock(&lckStdout);
	
	while(true) {
		
		for(int i=0; i<2; i++) {
			if(tlCountdownInternal[i] == 0)
				tlCountdownInternal[i] = internalChangeState(i, &tlStateInternal[i]);
		}

		usleep(sleeptime*1000);

		for(int i=0; i<2; i++) {
			tlCountdownInternal[i] --;
		}
	}
} // internalCtrl4WaysCrossroadThrd

void *tlPinsThrd(void *args) {

	while(true)	{
		uint16_t phase, tl_mode, tl_state, fake, TL_MODE[4], TL_STATE[4],
			TIME_TO_CHANGE[4]; // This is unused atm
		int sleeptime = 100;

		//
		// tolto perche con questo lock il thread va in deadlock
		//
		//pthread_mutex_lock(&lckValues);

		/* ITI
		pthread_mutex_lock(&lckStdout);
		std::cout << "[tlPinsThrd()] ctrlTypeSrv is " << std::to_string(ctrlTypeSrv) << std::endl;
	 	pthread_mutex_unlock(&lckStdout);
		*/
		
		uint16_t temp;
		switch(ctrlTypeSrv) {
			case SemaphoreCtrl::Server:

				/*
				pthread_mutex_lock(&lckStdout);
				std::cout << "[tlPinsThrd()] tlStateSrv is " << std::to_string(tlStateSrv) << std::endl;
	 			pthread_mutex_unlock(&lckStdout);
				*/

				// We receive TLA1 from server
				Semaphore::WritePins4WayCrossroads(tlStateSrv);
				break;

			case SemaphoreCtrl::Modbus:
				for(int i=0; i<4; i++)
				{
					decodeValues(i, &phase, &TL_MODE[i], &TL_STATE[i], &TIME_TO_CHANGE[i]);
					//printValues(values);
				 	Semaphore::WritePins(i, phase, TL_MODE[i], TL_STATE[i]);
				}
				break;

			case SemaphoreCtrl::Internal:
				/*
				pthread_mutex_lock(&lckStdout);
				std::cout 	<< "[tlPinsThrd()] tlStateInternal is { " 
						<< std::to_string(tlStateInternal[0]) << ", "
						<< std::to_string(tlStateInternal[1]) << " }" << std::endl;
	 			pthread_mutex_unlock(&lckStdout);
				*/
				// 0-> TLAs; 1->TLBs
				Semaphore::WritePins4WayCrossroads(tlStateInternal[0], tlStateInternal[1]);
				break;
		} // switch

		//pthread_mutex_unlock(&lckValues);
		
		usleep(1000 * sleeptime);
	}
	
} // tlpinsthrd

// TODO FIXME do it in a smarter way
Document conf;

#ifdef MODBUS
void *modbusThrd(void *args) {

	mbh = NULL;
	modbus_init  = false; // Is Modbus on?

	try{
		std::cout << "Initializing modbus" << std::endl;
		
		mbh = new ModbusHandler(
			conf["modbus-master"]["ip-addr"].GetString(),
			conf["modbus-master"]["port"]   .GetInt(),
			conf["modbus-master"]["debug"]  .GetInt()
		);
		mbh->printConfig();

		modbus_init = true;		
	}
	catch(std::exception e){
		std::cout << "Modbus connection not started" << std::endl;
	}
	
	while(true) {
		if(mbh != NULL) {
			try {
				if(modbus_init){
					
					std::vector<uint16_t> temp=mbh->receiveRegisters();
					pthread_mutex_lock(&lckValues);
					values = temp;
					pthread_mutex_unlock(&lckValues);
				}
			}
			//
			// Il simulatore e' uscito
			catch(ModbusExceptionNoReceive e) {
				//pthread_mutex_unlock(&lckValues);
				// restart connection
				delete mbh;
				mbh = new ModbusHandler(
					conf["modbus-master"]["ip-addr"].GetString(),
					conf["modbus-master"]["port"] .GetInt(),
					conf["modbus-master"]["debug"].GetInt()
				);
				//mbh->printConfig();
				//pthread_mutex_lock(&lckStdout);
				std::cout << "Modbus connection dropped. Restarted server." << std::endl;
				//pthread_mutex_unlock(&lckStdout);
				// TODO manage if Modbus is disconnected
			}
		}
		else {
			error("Modbus is offline");
		}
	}
} // modbusThrd
#endif


int main(int argc, char **argv) {

	signal(SIGINT, exiting);
	
	if(argc != 2){
		std::cout << "usage: "      << argv[0] 
				  << " <conf.json>" << std::endl;

		return EXIT_FAILURE;
	}

	//
	// Read config file
	std::ifstream jsonFile(argv[1]);
	std::string jsonString = "";
	std::string tmpString;

	while(std::getline(jsonFile, tmpString)){
		jsonString += tmpString;
	}

	conf.Parse(jsonString.c_str());

	Value & sems = conf["semaphores"];

	//
	// Init delays
	d_r = conf["delays"]["R"].GetInt();
	d_g = conf["delays"]["G"].GetInt();
	d_y = conf["delays"]["Y"].GetInt();

	//
	// Init pins
	for (SizeType i = 0; i < sems.Size(); ++i){
		S.push_back(new Semaphore(
			conf["semname"].GetString(),
			sems[i]["g-pin"].GetInt(),
			sems[i]["r-pin"].GetInt(),
			sems[i]["y-pin"].GetInt(),
			conf["http_server.url"].GetString(),
			sems[i]["position"]["latitude"].GetFloat(),
			sems[i]["position"]["longitude"].GetFloat(),
			sems[i]["position"]["orientation"].GetInt()
		));
	}

	for(int i = 0; i < S.size(); ++i) {
		S[i]->printConfig();
	}

	//
	//Default ctrl vars
		
	// Ctrl type (ctrlTypeSrv same as ctrl_type)
	ctrl_type = conf["ctrl_type"].GetInt();

#ifndef MODBUS
	if(ctrl_type == 2) {
		std::cout << "Error: Modbus not implemented" << std::endl;
		return EXIT_FAILURE;
	}
#endif

	ctrlTypeSrv = ctrl_type;

	// Send/receive from server
	use_server = conf["http_server.enable"].GetBool();
	use_masa   = conf["masa_server.enable"].GetBool();

	// PHASE
	values.push_back(5);

	// TL_MODEs (1/sem)
	for (int i = 0; i < sems.Size(); ++i){
		values.push_back(SemaphoreMode::DrivenByPhase);
	}

	// TL_STATEs (1/sem)
	for (int i = 0; i < sems.Size(); ++i){
		values.push_back(SemaphoreState::Off);
	}

	pthread_mutex_init(&lckStdout, NULL);
	pthread_mutex_init(&lckValues, NULL);

	std::vector<int> taskIds;

	for(int i = 0; i < 4; ++i) {
		taskIds.push_back(i);
	}

	// One for TLAs, one for TLBs
	pthread_create(&internalWorker, NULL, internalCtrl4WaysCrossroadThrd, NULL);
	pthread_detach(internalWorker);

	// Modbus thread
	#ifdef MODBUS
	pthread_create(&modbusWorker, NULL, modbusThrd, NULL);
	pthread_detach(modbusWorker);
	#endif

	if(use_server) {
		pthread_create(&serverWorker, NULL, serverHandler, NULL);
		pthread_detach(serverWorker);
	}

	if(use_masa) {

		mph = new MasaProtocolHandler(conf["masa_server.url"].GetString(), conf["masa_server.port"].GetInt());

		pthread_create(&masaWorker, NULL, masaHandler, NULL);
		pthread_detach(masaWorker);
	}

	// Pins ctrl (TODO put it below here?)
	pthread_create(&pinsWorker, NULL, tlPinsThrd, NULL);
	pthread_detach(pinsWorker);

	while(true) {
		usleep(1000 * 1000);
	}
	
	return EXIT_SUCCESS;
} // Main

