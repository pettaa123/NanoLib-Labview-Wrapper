#pragma once
#include <cstdint>
#include "nanolib_helper.hpp"

#include <Windows.h>
#include <iostream>
#ifndef DBOUT
#define DBOUT( s )            \
{                             \
   std::wostringstream os_;    \
   os_ << s;                   \
   OutputDebugStringW( os_.str().c_str() );  \
}

#endif

class PowerSM {
public:

	PowerSM(NanoLibHelper& nanolibHelper, nlc::BusHardwareId& openedBusHardware, nlc::DeviceHandle& connectedDeviceHandle);
	~PowerSM();

	int disableOperation();
	int enableOperation();
	int getCurrentState(uint8_t& state);

private:

	NanoLibHelper& nanolibHelper;
	nlc::BusHardwareId& openedBusHardware;
	nlc::DeviceHandle& connectedDeviceHandle;

	int shutdown_1_5_8();
	int switchOn_2();
	int disableVoltage_6_7_9_12();
	int quickStop_10();
	int disableOperation_4();
	int enableOperation_3();
	int faultReset_13();

	bool stateChanged(uint8_t& lastState, uint8_t& currentState);

	enum States
	{
		ST_NOT_READY_TO_SWITCH_ON,
		ST_SWITCHED_ON_DISABLED,
		ST_READY_TO_SWITCH_ON,
		ST_SWITCHED_ON,
		ST_OPERATION_ENABLED,
		ST_QUICK_STOP_ACTIVE,
		ST_FAULT_REACTION_ACTIVE,
		ST_FAULT,
		ST_MAX_STATES
	};
};