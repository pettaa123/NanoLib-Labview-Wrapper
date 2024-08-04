#pragma once
#include <cstdint>
#include "nanolib_helper.hpp"


//Die Steuerung erreicht nach Einschalten und erfolgreichem Selbsttest den Zustand Switch on disabled.

class PowerSM {
public:

	PowerSM(NanoLibHelper *nanolibHelper, std::optional<nlc::DeviceHandle> *connectedDeviceHandle);
	~PowerSM();

	int Shutdown();
	int DisableOperation();
	int EnableOperation();
	int GetCurrentState(uint8_t& state);
	int QuickStop();

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

	NanoLibHelper* nanolibHelper;
	std::optional<nlc::DeviceHandle>* connectedDeviceHandle;

	void Shutdown_2_6_8();
	void SwitchOn_3();
	void DisableVoltage_7_10_9_12();
	/*	Übergang in den Zustand Quick stop active(quick stop option):
	In diesem Fall wird die in Objekt 605Ah hinterlegte Aktion ausgeführt.	*/
	void QuickStop_11();
	void DisableOperation_5();
	void EnableOperation_4();
	void EnableOperationAfterQuickStop_16();
	void FaultReset_15();

	bool StateChanged(uint8_t& lastState, uint8_t& currentState);

};

