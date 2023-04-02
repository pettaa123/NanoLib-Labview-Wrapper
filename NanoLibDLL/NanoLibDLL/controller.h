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

private:
	NanoLibHelper nanolibHelper;
	std::optional<nlc::BusHardwareId> openedBusHardware;
	std::optional<nlc::DeviceHandle> connectedDeviceHandle;

};


#pragma once

