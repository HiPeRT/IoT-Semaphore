#ifndef UTILS_H
#define UTILS_H

#include <signal.h>
#include <pthread.h>
#include <cstdlib>
#include <string>
#include <fstream>
#include <vector>

#include <rapidjson/document.h>

#include "Gpio.h"
#include "Semaphore.h"
#ifdef MODBUS
	#include "ModbusHandler.h"
#endif

using namespace rapidjson;

extern pthread_mutex_t lckStdout;
extern uint16_t tlStateSrv, ctrlTypeSrv, tlStateInternal[];
extern std::vector<uint16_t> values;

static void error(std::string msg){
	pthread_mutex_lock(&lckStdout);
	std::cout << "ERROR: " << msg << std::endl;	
	pthread_mutex_unlock(&lckStdout);
}

static inline bool isB(int id) {
	return id < 2;
}

/*
 * Calls to this functions MUST be blocked! (also, wrapped if more than one consecutive)
 */
static void decodeValues(int id, uint16_t *PHASE, uint16_t *TL_MODE, uint16_t *TL_STATE, uint16_t* TIME_TO_CHANGE) {
    
    switch(ctrlTypeSrv) {

        case SemaphoreCtrl::Server:
            *TL_STATE = tlStateSrv;
			std::cout << "!!!!!!!!!!!!!!!! THIS SHOULD NEVER APPEAR !!!!!!!!!!" << std::endl;
            break;

        case SemaphoreCtrl::Modbus:
            *PHASE = values[0];
            *TL_MODE  = values[id + 1];
            *TL_STATE = values[id + 1 + 4];
            *TIME_TO_CHANGE = values[id + 1 + 8];
            /*
            if(id == 0) {	
                pthread_mutex_lock(&lckStdout);
                std::cout 	<< "[decodeValues(id: " << std::to_string(id) << ")]"
                            << " PHASE is " << std::to_string(*PHASE)
                            << " TL_MODE is " << std::to_string(*TL_MODE)
                            << " TL_STATE is " << std::to_string(*TL_STATE)
                            << std::endl;
                pthread_mutex_unlock(&lckStdout);
            }
            */
            break;

        case SemaphoreCtrl::Internal:
            *TL_STATE = tlStateInternal[isB(id) ? 1 : 0];
			std::cout << "!!!!!!!!!!!!!!!! THIS SHOULD NEVER APPEAR !!!!!!!!!!" << std::endl;
            break;
    } // switch
} // decodeValues

static void printValues(std::vector<uint16_t> values) {
	pthread_mutex_lock(&lckStdout);

	int count = 0;
	std::cout 	<< " PHASE: " << values[count]  << " TL_MODE: { ";
	for(int i=0; i<4; i++, count++)
	{
		std::cout << values[count];
	        if(i != 4 -1)
			std::cout << ", ";
	}
	std::cout << " } TL_STATE: { ";
	for(int i=0; i<4; i++, count++)
	{
		std::cout << values[count];
	        if(i != 4 -1)
			std::cout << ", ";
	}
	
	std::cout << " }" << std::endl;
	pthread_mutex_unlock(&lckStdout);

} // printValues

static std::vector<uint16_t> encodeValues(int id, uint16_t PHASE, uint16_t TL_MODE, uint16_t TL_STATE) {
	/*
	if(id == 0){

		pthread_mutex_lock(&lckStdout);
		std::cout 	<< "[encodeValues(id: " << std::to_string(id) << ")]"
					<< " PHASE is " << std::to_string(PHASE)
					<< " TL_MODE is " << std::to_string(TL_MODE)
					<< " TL_STATE is " << std::to_string(TL_STATE)
					<< std::endl;
		pthread_mutex_unlock(&lckStdout);
	}
	*/
	std::vector<uint16_t> newvalues = values;
	newvalues[0] = PHASE;
	newvalues[id + 1] = TL_MODE;
	newvalues[id + 1 + 4] = TL_STATE;
	return newvalues;
} // encodeValues

static uint16_t revertTlState(uint16_t tlState) {
	if( tlState == SemaphoreState::Green || tlState == SemaphoreState::GreenYellow)
		return SemaphoreState::Red;
	else if(tlState == SemaphoreState::Red || tlState == SemaphoreState::RedYellow)
		return SemaphoreState::Green;
	else if(tlState == SemaphoreState::Off)
		return SemaphoreState::Off;
	else
		error("[revertTlState] tlState " + std::to_string(tlState) + " NOT SUPPORTED");
} // revertTlState

#endif
