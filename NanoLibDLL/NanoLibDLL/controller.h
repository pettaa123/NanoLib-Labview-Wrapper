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
	int connectDevice(unsigned int deviceToOpen);
	int autoSetupMotPams();

private:
	NanoLibHelper nanolibHelper;
	std::optional<nlc::BusHardwareId> openedBusHardware;
	std::optional<nlc::DeviceHandle> connectedDeviceHandle;
	nlc::DeviceHandle permConnectedDeviceHandle;

};

#include <Windows.h>
#include <iostream>
#define DBOUT( s )            \
{                             \
   std::wostringstream os_;    \
   os_ << s;                   \
   OutputDebugStringW( os_.str().c_str() );  \
}

