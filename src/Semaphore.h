#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <unistd.h>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Exception.hpp>
#include <curlpp/Infos.hpp>
#include <sstream>

#include "Gpio.h"

static const int MapTLA1 = 0;
static const int MapTLA2 = 1;
static const int MapTLB1 = 2;
static const int MapTLB2 = 3;

enum SemaphoreCtrl { Unknown = 0, Server = 1, Modbus = 2, Internal = 3 };
enum SemaphoreState { Off = 0, Green = 1, GreenYellow = 2, Red = 3, RedYellow = 4, Yellow = 5,/* UNIMORE's W/O */ Blinking = 10 };
enum SemaphoreMode { Manual = 0, DrivenByPhase = 1, BlinkingYellow = 2 };

class Semaphore{

	public:
	
		enum Color{R, G, Y};


		Semaphore(
			std::string name,
			int p_g, int p_r, int p_y, 
			std::string server,
			float latitude, float longitude, int orientation
		);
		~Semaphore();

		void printConfig();
		void sendStatus(uint16_t PHASE, uint16_t TL_MODE, uint16_t TL_STATE, uint16_t ctrl_type, bool modbus_init, uint16_t timeToChange,uint16_t *tlState, uint16_t *ctrlType);
		void getCtrl(uint16_t *TL_STATE, uint16_t *ctrl_type);

		static SemaphoreState DecodeStatus(SemaphoreCtrl ctrl, uint16_t PHASE, uint16_t TL_MODE, uint16_t TL_STATE);
		
		static void WritePins4WayCrossroads(uint16_t tla1State);		
		static void WritePins4WayCrossroads(uint16_t tlaState, uint16_t tlbState);
		static void WritePins(int id, uint16_t PHASE, uint16_t TL_MODE, uint16_t TL_STATE);

		float getLatitude();
		float getLongitude();
		int getOrientation();

		Gpio *getLed(Color color);
	
	protected:
	
		static void WritePins(int id, uint16_t tlState);

	private:

		std::string _name;
		Gpio *_R;
		Gpio *_Y;
		Gpio *_G;

		std::string _server;

		float _latitude;
		float _longitude;
		int _orientation;
};
#endif
