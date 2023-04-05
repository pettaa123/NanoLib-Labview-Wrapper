#pragma once

#include "pch.h"
#include "nanolib_helper.hpp"
#include <sstream>
#include <optional>

class Controller{
public:

	Controller();
	~Controller();


	int getAvailablePorts(std::vector<std::string> &ports);
	int openPort(unsigned int portToOpen);
	int scanBus(std::vector<std::string>& devices);
	int autoSetupMotPams(unsigned int deviceToOpen);


private:
	//struct MyOpenedBusHardware {
	//	nlc::BusHardwareId value;
	//	bool isSet=false;
	//};
	//MyOpenedBusHardware openedBusHardware;
	NanoLibHelper nanolibHelper;
	std::optional<nlc::BusHardwareId> openedBusHardware;
	std::optional<nlc::DeviceHandle> connectedDeviceHandle;

};

#include <Windows.h>
#include <iostream>
#define DBOUT( s )            \
{                             \
   std::wostringstream os_;    \
   os_ << s;                   \
   OutputDebugStringW( os_.str().c_str() );  \
}

