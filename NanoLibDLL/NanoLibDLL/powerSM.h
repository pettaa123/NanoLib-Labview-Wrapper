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



//Die Steuerung erreicht nach Einschalten und erfolgreichem Selbsttest den Zustand Switch on disabled.

class PowerSM {
public:

	PowerSM(NanoLibHelper *nanolibHelper, std::optional<nlc::DeviceHandle> *connectedDeviceHandle);
	~PowerSM();

	int shutdown();
	int disableOperation();
	int enableOperation();
	int getCurrentState(uint8_t& state);
	int quickStop();

	enum States : int8_t
	{
		NOT_READY_TO_SWITCH_ON=0,
		SWITCHED_ON_DISABLED,
		READY_TO_SWITCH_ON,
		SWITCHED_ON,
		OPERATION_ENABLED,
		QUICK_STOP_ACTIVE,
		FAULT_REACTION_ACTIVE,
		FAULT,
		MAX_STATES
	};

private:

	NanoLibHelper *nanolibHelper;
	std::optional<nlc::DeviceHandle> *connectedDeviceHandle;

	void shutdown_2_6_8();
	void switchOn_3();
	void disableVoltage_7_10_9_12();
	/*	Übergang in den Zustand Quick stop active(quick stop option):
	In diesem Fall wird die in Objekt 605Ah hinterlegte Aktion ausgeführt.	*/
	void quickStop_11();
	void disableOperation_5();
	void enableOperation_4();
	void enableOperationAfterQuickStop_16();
	void faultReset_15();

	bool stateChanged(uint8_t& lastState, uint8_t& currentState);

};

