#include "ModbusHandler.h"

ModbusHandler::ModbusHandler(std::string ip_addr, int port, int dbg){

	_ip_addr = ip_addr;
	_port	 = port;

	std::cout << "\nModbus TCP connection started..." << std::endl;

	_ctx = modbus_new_tcp(_ip_addr.c_str(), _port);

	struct timeval tmo;

	tmo.tv_sec  = 2;
	tmo.tv_usec = 0;
	
	modbus_set_response_timeout(_ctx, &tmo);
	
	if(_ctx == NULL){
		std::cout << "Unable to allocate context" << std::endl;
		throw ModbusExceptionNoInit();
	}
	_mb_mapping = modbus_mapping_new(500, 500, 500, 500);
	if(_mb_mapping == NULL){
		std::cout << "Failed to allocate modbus mapping" << std::endl;
		throw ModbusExceptionNoInit();
	}
	modbus_set_debug(_ctx, dbg);
	
	_socket = modbus_tcp_listen(_ctx, 10);
	if(_socket == -1){
		std::cout << "Failed to allocate socket" << std::endl;
		throw ModbusExceptionConnect();
	}
	
	if(modbus_tcp_accept(_ctx, &_socket) == -1){
		std::cout << "Failed to initialize socket" << std::endl;
		throw ModbusExceptionConnect();
	}
}

void ModbusHandler::sendRegisters(std::vector<Semaphore*> sems){

	uint16_t *status_sems = new uint16_t[sems.size()];

	for(int i = 0; i < sems.size(); ++i){

		status_sems[i] = 0;

		if(sems[i]->getLed(Semaphore::R)->readPin()) 
			status_sems[i] |= 1;
		else if(sems[i]->getLed(Semaphore::Y)->readPin())
			status_sems[i] |= (1 << 1);
		else if(sems[i]->getLed(Semaphore::G)->readPin())
			status_sems[i] |= (1 << 2);

		// TODO add timeToChange?
		//std::cout << "status_sem" << i << " "
		//	  << status_sems[i] 
		//	  << std::endl;
	}
	
	int rc = modbus_write_registers(_ctx, 0, sems.size(), status_sems);

	delete[] status_sems;
}

std::vector<uint16_t> ModbusHandler::receiveRegisters(){

	uint8_t recv[MODBUS_TCP_MAX_ADU_LENGTH];

	std::vector<uint16_t> values;

	int nrecv = modbus_receive(_ctx, recv);
	if( nrecv > 0){
		/*
		for(int i = 13; i < nrecv; i += 2){
			values.push_back((((uint16_t)recv[i]) << 8 ) | recv[i + 1]);	
		}
		*/
		modbus_reply(_ctx, recv, nrecv, _mb_mapping);
		values.push_back( _mb_mapping->tab_registers[0] );
		values.push_back( _mb_mapping->tab_registers[1] );
		values.push_back( _mb_mapping->tab_registers[2] );
		values.push_back( _mb_mapping->tab_registers[3] );
		values.push_back( _mb_mapping->tab_registers[4] );
		values.push_back( _mb_mapping->tab_registers[5] );
		values.push_back( _mb_mapping->tab_registers[6] );
		values.push_back( _mb_mapping->tab_registers[7] );
		values.push_back( _mb_mapping->tab_registers[8] );
		values.push_back( _mb_mapping->tab_registers[9] );
	}
	else{
		throw ModbusExceptionNoReceive();
	}
	return values;
}

void ModbusHandler::printConfig(){
	std::cout << "\nModbus Configuration:" 	   << std::endl;
	std::cout << "\tserver-ip: "   << _ip_addr << std::endl;
	std::cout << "\tserver-port: " << _port    << std::endl;
}

ModbusHandler::~ModbusHandler(){

	std::cout << "Cleaning modbus connection" << std::endl;
	
	if(_socket != -1){
		close(_socket);
	}
	modbus_mapping_free(_mb_mapping);
	modbus_close(_ctx);
	modbus_free(_ctx);
}
