#ifndef MODBUSHANDLER_H
#define MODBUSHANDLER_H

#include <string>
#include <vector>
#include <iostream>
#include <modbus.h>

#include "ModbusException.h"
#include "Semaphore.h"

class ModbusHandler{

	public:

	ModbusHandler(std::string ip_addr, int port, int dbg);
	~ModbusHandler();
	std::vector<uint16_t> receiveRegisters();
	void sendRegisters(std::vector<Semaphore*> sems);
	void printConfig();

	private:
	
	std::string _ip_addr;
	int _port;
	int _socket;
	modbus_t *_ctx;
	modbus_mapping_t *_mb_mapping;
};
#endif
