cmake_minimum_required(VERSION 3.5)

project(IoT-Semaphore)

add_definitions(-std=c++11)

include_directories(/usr/local/include/modbus)

find_library(CURL_LIB curl curlpp libmodbus)

add_executable(
	Semaphore
	src/Main.cpp
	src/Gpio.cpp
	src/Semaphore.cpp	
	src/HttpClient.cpp
	src/MasaProtocolHandler.cpp
	src/ModbusHandler.cpp
)
target_link_libraries(
	Semaphore
	curl
	curlpp
	modbus
	pthread
)
