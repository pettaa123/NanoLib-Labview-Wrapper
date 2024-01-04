#pragma once

#include "motor.h"

class AutoSetupMotor : public Motor402 {

public:

	AutoSetupMotor(NanoLibHelper *nanolibHelper, std::optional<nlc::DeviceHandle> *connectedDeviceHandle, PowerSM *powerSM) :
		Motor402(nanolibHelper, connectedDeviceHandle, powerSM)
	{
		setModeOfOperation(OperationMode::AutoSetup);
	}


	int start() { return EXIT_FAILURE; };
	int halt() { return EXIT_FAILURE; };


	/*
	Solange sich der an der Steuerung angeschlossene Motor oder die Sensoren f�r die R�ckf�hrung
	(Encoder/Hall-Sensoren) nicht �ndern, ist das Auto-Setup nur einmal bei der Erstinbetriebnahme
	durchzuf�hren. ->Der Motor muss lastfrei sein.
	*/
	int autoSetupMotPams();

private:

	uint16_t getState() { return EXIT_FAILURE; };
};