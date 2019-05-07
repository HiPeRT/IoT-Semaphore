#ifndef GPIO_H
#define GPIO_H

#include <iostream>
#include <string>

class Gpio{

	public:

	enum Direction{IN, OUT};

	Gpio(int pin, Direction direction);
	~Gpio();
	void toggle();
	void writePin(bool status);
	int getPinNum();
	int readPin();
	
	private:

	int _pin;
	bool _status;
	Direction _direction;
	std::string _base;	
};
#endif
