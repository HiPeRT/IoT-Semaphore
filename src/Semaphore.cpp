#include "Semaphore.h"
#include "HttpClient.h"

#include <rapidjson/document.h>

using namespace rapidjson;

Semaphore::Semaphore(
	std::string name,
	int p_g, int p_r, int p_y,
	std::string server,
	float latitude, float longitude, int orientation) {

	_name = name;
	_R = new Gpio(p_r, Gpio::OUT);
	_G = new Gpio(p_g, Gpio::OUT);
	_Y = new Gpio(p_y, Gpio::OUT);

	_server = server;

	_latitude  = latitude;
	_longitude = longitude;
	_orientation = orientation;

}

Semaphore::~Semaphore(){
	delete _R;
	delete _G;
	delete _Y;
}

void Semaphore::printConfig(){

	std::cout << _name << std::endl;

	std::cout << "\tLed-R\tpin: " << _R->getPinNum() << std::endl;
	std::cout << "\tLed-G\tpin: " << _G->getPinNum() << std::endl; 
	std::cout << "\tLed-Y\tpin: " << _Y->getPinNum() << std::endl;
	std::cout << "\n";

	std::cout << "\tlatitude: "    << _latitude    << std::endl;
	std::cout << "\tlongitude: "   << _longitude   << std::endl;
	std::cout << "\torientation: " << _orientation << std::endl;
	std::cout << "\n";
}

float Semaphore::getLatitude() { return _latitude;   }
float Semaphore::getLongitude(){ return _longitude;  }
int Semaphore::getOrientation(){ return _orientation;}

Gpio* Semaphore::getLed(Color color){
	switch(color){
		case Semaphore::G:return _G;
		case Semaphore::R:return _R;
		case Semaphore::Y:return _Y;
	}
}

void Semaphore::sendStatus(	uint16_t PHASE, uint16_t TL_MODE, uint16_t TL_STATE, uint16_t ctrl_type,
							bool modbus_init, uint16_t timeToChange, uint16_t *tlState, uint16_t *ctrlType) {
	std::string _token = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJ1bmlxdWVfbmFtZSI6InNlbTEiLCJzdWIiOiJzZW0xIiwidmVyIjoiMyIsImlzcyI6Imh0dHBzOi8vYmFja2VuZC1kZXY1LmlvdHR5LmNvbS8iLCJhdWQiOiJodHRwczovL2JhY2tlbmQtZGV2NS5pb3R0eS5jb20vIiwiZXhwIjoxNTY2NzQyOTY3LCJuYmYiOjE1MzUyMDY5Njd9.gVQkykRGpeBmxEi5kagMd6DlqnIxwL_py0ah983Ptm4";

	SemaphoreState status = DecodeStatus((SemaphoreCtrl) ctrl_type, PHASE, TL_MODE, TL_STATE);
	// These to become params
	std::string endpoint = "api/semaphore/" + this->_name  + "/status";
	std::string postBody("{ "  
			"\"id\" : \"" + this->_name + "\", "
			"\"modbus_init\" : \"" + std::to_string(modbus_init) + "\", "
			"\"status\" : \"" + std::to_string(status) + "\", " 
			"\"phase\" : \"" + std::to_string(PHASE) + " \", "
			"\"tl_mode\" : \"" + std::to_string(TL_MODE) + " \", "
			"\"tl_state\" : \"" + std::to_string(TL_STATE) + " \", "
			"\"ctrl_type\" : \"" + std::to_string(ctrl_type) + " \", "
			"\"time_to_change\" : \"" + std::to_string(timeToChange) + "\""
			" }");

	std::cout << "Post body: " << postBody << std::endl;
	std::string url = _server + "/" + endpoint;

	std::string res = HttpClient::Post(url, _token, postBody);

	if(!res.empty() && res.compare("\"\"") != 0) {
		std::cout << "[getCtrl] Response is '" << res << "'" << std::endl;
		Document conf;
		conf.Parse(res.c_str());

		*tlState = conf["status"]["tl_state"].GetInt();
		*ctrlType = conf["status"]["ctrl_type"].GetInt();

		// std::cout 	<< "From server CTRL is " << *ctrlType
		// 			<< " STATE is " << *tlState << std::endl;
	}
	else {
	//	std::cout << "No ctrl received from server" << std::endl;
	}
}
extern uint16_t tlStateSrv;
extern uint16_t ctrlTypeSrv;

void Semaphore::getCtrl(uint16_t *tlState, uint16_t *ctrlType) {
	
	// These to become params
	std::string endpoint = "api/semaphore/" + this->_name  + "/ctrl";
	
	std::string url = _server + "/" + endpoint;

	std::string res = HttpClient::Get(url);

	if(!res.empty() && res.compare("\"\"") != 0) {
		//std::cout << "[getCtrl] Response is '" << res << "'" << std::endl;
		Document conf;
		conf.Parse(res.c_str());

		*tlState = conf["status"]["tl_state"].GetInt();
		*ctrlType = conf["status"]["ctrl_type"].GetInt();

		// std::cout 	<< "From server CTRL is " << *ctrlType
		// 			<< " STATE is " << *tlState << std::endl;
	}
	else {
	//	std::cout << "No ctrl received from server" << std::endl;
	}
}

extern pthread_mutex_t lckStdout;
void dump(int id, std::string msg) {
//	return;
	if(id == 0) {
		pthread_mutex_lock(&lckStdout);
		std::cout << " -------------------------------------------------- TRAFFIC LIGHTS WILL BE " << msg << std::endl;
		pthread_mutex_unlock(&lckStdout);
	}
}

#include "Utils.h"
void Semaphore::WritePins4WayCrossroads(uint16_t tla1State)
{
	uint16_t tlbState = revertTlState(tla1State);
	Semaphore::WritePins4WayCrossroads(tla1State, tlbState);
} // WritePins4WayCrossroads

void Semaphore::WritePins4WayCrossroads(uint16_t tlaState, uint16_t tlbState)
{
	Semaphore::WritePins(MapTLA1, tlaState);
	Semaphore::WritePins(MapTLA2, tlaState);
	Semaphore::WritePins(MapTLB1, tlbState);
	Semaphore::WritePins(MapTLB2, tlbState);
} // WritePins4WayCrossroads


#include <vector>
extern std::vector<Semaphore *> S;
void Semaphore::WritePins(int id, uint16_t tlState) {
	  switch(tlState) {
		  case SemaphoreState::Off:
	 		S[id]->getLed(Semaphore::R)->writePin(0);
	 		S[id]->getLed(Semaphore::Y)->writePin(0);
			S[id]->getLed(Semaphore::G)->writePin(0);
	 		dump(id, "OFF");
	 		break;
		  case SemaphoreState::Green:
	 		S[id]->getLed(Semaphore::R)->writePin(0);
	 		S[id]->getLed(Semaphore::Y)->writePin(0);
			S[id]->getLed(Semaphore::G)->writePin(1);
	 		dump(id, "GREEN");
	 		break;
		  case SemaphoreState::GreenYellow:
			S[id]->getLed(Semaphore::R)->writePin(0);
	 		S[id]->getLed(Semaphore::Y)->writePin(1);
	 		S[id]->getLed(Semaphore::G)->writePin(1);
	 		dump(id, "YELLOW + GREEN");
	 		break;
		  case SemaphoreState::Red:
	 		S[id]->getLed(Semaphore::R)->writePin(1);
	 		S[id]->getLed(Semaphore::Y)->writePin(0);
	 		S[id]->getLed(Semaphore::G)->writePin(0);
	 		dump(id, "RED");
	 		break;
		  case SemaphoreState::RedYellow:
	 		S[id]->getLed(Semaphore::R)->writePin(1);
	 		S[id]->getLed(Semaphore::Y)->writePin(1);
	 		S[id]->getLed(Semaphore::G)->writePin(0);
	 		dump(id, "RED + YELLOW");
	 		break;
	 }
}

void Semaphore::WritePins(int id, uint16_t PHASE, uint16_t TL_MODE, uint16_t TL_STATE) {
	
	//
	// si tratta di un TLB
	/*if(isB(id)){
		switch(PHASE){
			case 1: PHASE = 3; break;
			case 2: PHASE = 4; break;
			case 6: PHASE = 7; break;
			case 3: PHASE = 1; break;
			case 4: PHASE = 2; break;
			case 7: PHASE = 6; break;
		}
	}*/

	switch(TL_MODE) {
		//
		// Controllo individuale
		case SemaphoreMode::Manual:
			// TODO TO TEST!
			WritePins(id, TL_STATE);
			break;

		//
		// Comandato da Phase, controllo manuale
		case SemaphoreMode::DrivenByPhase :
			switch(PHASE) {
				case 1:
					S[id]->getLed(Semaphore::G)->writePin(1);
					S[id]->getLed(Semaphore::Y)->writePin(0);
					S[id]->getLed(Semaphore::R)->writePin(0);
					dump(id, "GREEN");
					break;
				case 2:
					//S[id]->getLed(Semaphore::G)->writePin(1);
					S[id]->getLed(Semaphore::G)->writePin(0);
					S[id]->getLed(Semaphore::Y)->writePin(1);
					S[id]->getLed(Semaphore::R)->writePin(0);
					//dump(id, "YELLOW + GREEN");
					dump(id, "YELLOW");
					break;
				case 3:
					S[id]->getLed(Semaphore::G)->writePin(0);
					S[id]->getLed(Semaphore::Y)->writePin(0);
					S[id]->getLed(Semaphore::R)->writePin(1);
					dump(id, "RED");
					break;
				case 4:
					S[id]->getLed(Semaphore::G)->writePin(0);
					S[id]->getLed(Semaphore::Y)->writePin(0);
					S[id]->getLed(Semaphore::R)->writePin(1);
					dump(id, "RED");
					break;
				case 5:
					S[id]->getLed(Semaphore::G)->writePin(0);
					S[id]->getLed(Semaphore::R)->writePin(0);
					S[id]->getLed(Semaphore::Y)->toggle();
					if(S[id]->getLed(Semaphore::Y)->readPin() == 1)
						dump(id, "BLINKING YELLOW");
					else
						dump(id, "OFF");
					break;
				case 6:
					S[id]->getLed(Semaphore::G)->writePin(1);
					S[id]->getLed(Semaphore::Y)->writePin(1);
					S[id]->getLed(Semaphore::R)->writePin(0);
					dump(id, "YELLOW + GREEN");
					break;
				case 7:
					S[id]->getLed(Semaphore::G)->writePin(0);
					S[id]->getLed(Semaphore::Y)->writePin(1);
					S[id]->getLed(Semaphore::R)->writePin(1);
					dump(id, "YELLOW + RED");
					break;
			}
			break;

		//
		// Giallo lampeggiante
		case SemaphoreMode::BlinkingYellow:
			S[id]->getLed(Semaphore::G)->writePin(0);
			S[id]->getLed(Semaphore::R)->writePin(0);
			S[id]->getLed(Semaphore::Y)->toggle();
			if(S[id]->getLed(Semaphore::Y)->readPin() == 1)
				dump(id, "BLINKING YELLOW");
			else
				dump(id, "OFF");

			usleep(1000 * 1000);
			break;
	}
} // WritePins

SemaphoreState Semaphore::DecodeStatus(SemaphoreCtrl ctrl, uint16_t PHASE, uint16_t TL_MODE, uint16_t TL_STATE) {

	if(ctrl == SemaphoreCtrl::Internal || ctrl == SemaphoreCtrl::Server)
		return (SemaphoreState) TL_STATE;

	switch(TL_MODE) {

		case SemaphoreMode::Manual:
			return (SemaphoreState) TL_STATE;

		case SemaphoreMode::DrivenByPhase :
			switch(PHASE) {
				case 1:
					return SemaphoreState::Green;
				case 2:
					// deve diventare yellow
					// sistemare colorazione grafica semaforo lato server
					//return SemaphoreState::GreenYellow;
					return SemaphoreState::Yellow;
				case 3:
					return SemaphoreState::Red;
				case 4:
					return SemaphoreState::Red;
				case 5:
					return SemaphoreState::Blinking;
				case 6:
					return SemaphoreState::GreenYellow;
				case 7:
					return SemaphoreState::RedYellow;
			}
			break;

		case SemaphoreMode::BlinkingYellow:
			return SemaphoreState::Blinking;
	}
} // WritePins
