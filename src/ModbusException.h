#ifndef MODB_EXC_H
#define MODB_EXC_H

#include <exception>

using namespace std;

class ModbusException : public exception
{
public:
	virtual const char * what () const throw ()
	{
		return "Modbus exception";
	}
};

class ModbusExceptionConnect : public ModbusException
{
public:
	virtual const char * what () const throw ()
	{
		return "Connect error";
	}
};

class ModbusExceptionNoInit : public ModbusException
{
public:
	virtual const char * what () const throw ()
	{
		return "Modbus not initialized";
	}
};

class ModbusExceptionWriteRegister : public ModbusException
{
public:
	virtual const char * what () const throw ()
	{
		return "Write register error";
	}
};

class ModbusExceptionNoConnected : public ModbusException
{
public:
	virtual const char * what () const throw ()
	{
		return "No connection";
	}
};
class ModbusExceptionNoReceive : public ModbusException
{
public:
	virtual const char * what () const throw ()
	{
		return "Nothing to receive";
	}
};
#endif
