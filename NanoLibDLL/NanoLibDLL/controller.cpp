#include "pch.h"
#include "controller.h"

Controller::Controller() {
}

Controller::~Controller() {

	// Always finalize connected hardware

	if (connectedDeviceHandle.has_value()) {
		//"Disconnecting the device."
		nanolibHelper.disconnectDevice(*connectedDeviceHandle);
	}

	if (openedBusHardware.has_value()) {
		//"Closing the hardware bus."
		nanolibHelper.closeBusHardware(*openedBusHardware);
	}
}

int Controller::getAvailablePorts() {
	try {
		// its possible to set the logging level to a different level
		nanolibHelper.setLoggingLevel(nlc::LogLevel::Off);

		// list all hardware available
		std::vector<nlc::BusHardwareId> busHardwareIds = nanolibHelper.getBusHardware();

		if (busHardwareIds.empty()) {
			// "No hardware buses found." << std::endl;
			return EXIT_SUCCESS;
		}
	}

	catch (const nanolib_exception& e) {
		//"Error occurred e.what();
		return EXIT_FAILURE;
		
	}
}