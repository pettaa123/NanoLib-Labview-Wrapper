#pragma once

#include "pch.h"
#include "nanolib_helper.hpp"
#include <sstream>
#include <optional>
#include <vector>

class Controller{
public:

	Controller();
	~Controller();

	int getAvailablePorts(std::vector<std::string> &ports);
	int openPort(UINT portToOpen);
	int connectDevice(UINT deviceToOpen);
	int scanBus(std::vector<std::string>& devices);
	int autoSetupMotPams();
	int home(UINT32 rpm);
	int moveToPosition(INT32 zehntelMM);
	int moveVelocity(INT16 rpm);
	//behaviour: 2=quickstop 1=slow down ramp
	int halt(UINT stopMode=2);
	int stop();
	//spindelsteigung 
	int setFeedConstant(UINT32 pitchZehntelMM);
	int setProfileVelocity(UINT32 rpm);
	int setMaxMotorCurrent(INT32 maxCurrent);
	int setProfileAcceleration(UINT32 acc);
	int setHomingAcceleration(UINT32 acc);

private:
	//struct MyOpenedBusHardware {
	//	nlc::BusHardwareId value;
	//	bool isSet=false;
	//};
	//MyOpenedBusHardware openedBusHardware;
	NanoLibHelper nanolibHelper;
	std::optional<nlc::BusHardwareId> openedBusHardware;
	std::optional<nlc::DeviceHandle> connectedDeviceHandle;

	int disconnectDevice();
	int getFirmwareVersion(std::string &version);
	int readDigitalInputs(UINT8& states);
	int configureInputs();
};


