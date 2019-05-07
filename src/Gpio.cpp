#include "Gpio.h"

Gpio::Gpio(int pin, Gpio::Direction direction){

	_pin 	   = pin;
	_direction = direction;
	_base 	   = "/sys/class/gpio/";
	_status    = 0;

	// crea gpio
	system(
		("echo " + std::to_string(_pin) + 
		 " > " 	 + _base + "export"
	).c_str());

	// imposta direzione
	
	std::string dir;

	if(_direction == Gpio::IN){
		dir = "in";	
	}
	else{
		dir = "out";
	}

	system(
		("echo " + dir + " > " + _base + "gpio" + 
		 std::to_string(_pin) + "/direction"
	).c_str());

	// resetta gpio
	system(
		("echo 0 > " + _base  + "gpio" + 
		 std::to_string(_pin) + "/value"
	).c_str());	
}	

Gpio::~Gpio(){

	//std::cout << "exiting" << std::endl;

	system(
		("echo " + std::to_string(_pin) + 
		 " > " + _base + "unexport"
	).c_str());
}

void Gpio::toggle(){
	//std::cout << "Status was " << std::to_string(_status) << std::endl;
	_status = !(_status);
	//std::cout << "Now Status is " << std::to_string(_status) << std::endl;

	system(
		("echo " + std::to_string(_status) + " > " + 
		 _base + "gpio" + std::to_string(_pin) + "/value"
	).c_str());	
}

void Gpio::writePin(bool status){

	_status = status;
	
	system(
		("echo " + std::to_string(_status) + " > " + 
		 _base + "gpio" + std::to_string(_pin) + "/value"
	).c_str());	
}

int Gpio::readPin(){
	return _status;
}

int Gpio::getPinNum(){
	return _pin;
}
