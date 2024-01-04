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
	Solange sich der an der Steuerung angeschlossene Motor oder die Sensoren für die Rückführung
	(Encoder/Hall-Sensoren) nicht ändern, ist das Auto-Setup nur einmal bei der Erstinbetriebnahme
	durchzuführen. ->Der Motor muss lastfrei sein.
	*/
	int autoSetupMotPams();

private:

	uint16_t getState() { return EXIT_FAILURE; };
};