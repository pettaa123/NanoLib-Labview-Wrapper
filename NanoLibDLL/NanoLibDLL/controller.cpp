#include "pch.h"
#include "controller.h"
#include "powerSM.h"

#include <chrono>
#include <thread>

Controller::Controller() {
}

Controller::~Controller() {

	// Always finalize connected hardware

	if (connectedDeviceHandle.has_value()) {
		//"Disconnecting the device."
		nanolibHelper.disconnectDevice(*connectedDeviceHandle);
		nanolibHelper.removeDevice(*connectedDeviceHandle);
	}

	if (openedBusHardware.has_value()) {
		//"Closing the hardware bus."
		nanolibHelper.closeBusHardware(*openedBusHardware);
	}
}

int Controller::getAvailablePorts(std::vector<std::string>& ports) {
	try {
		// its possible to set the logging level to a different level
		nanolibHelper.setLoggingLevel(nlc::LogLevel::Off);

		// list all hardware available
		std::vector<nlc::BusHardwareId> busHardwareIds = nanolibHelper.getBusHardware();

		if (busHardwareIds.empty()) {
			// "No hardware buses found." << std::endl;
			return EXIT_SUCCESS;
		}
		int lineNum = 0;
		// print out available hardware
		for (const nlc::BusHardwareId& busHwId : busHardwareIds) {
			std::stringstream ss;
			ss << lineNum << ". " << busHwId.getName()
				<< " protocol: " << busHwId.getProtocol();
			ports.push_back(ss.str());
			lineNum++;
		}
	}

	catch (const nanolib_exception& e) {
		//"Error occurred e.what();
		return EXIT_FAILURE;
		DBOUT("exception: " << e.what());
	}
	return EXIT_SUCCESS;
}

int Controller::openPort(UINT portToOpen) {
	try {
		if (openedBusHardware.has_value()) {
			//"Closing the hardware bus."
			return EXIT_FAILURE;
		}
		// its possible to set the logging level to a different level
		nanolibHelper.setLoggingLevel(nlc::LogLevel::Off);

		// list all hardware available
		std::vector<nlc::BusHardwareId> busHardwareIds = nanolibHelper.getBusHardware();

		if (busHardwareIds.empty()) {
			return EXIT_FAILURE;
		}

		if (portToOpen >= busHardwareIds.size()) {
			//Invalid bus hardware number."
			return EXIT_FAILURE;
		}

		// Use the selected bus hardware
		nlc::BusHardwareId busHwId = busHardwareIds[portToOpen];

		// create bus hardware options for opening the hardware
		nlc::BusHardwareOptions busHwOptions = nanolibHelper.createBusHardwareOptions(busHwId);
		// now able to open the hardware itself
		nanolibHelper.openBusHardware(busHwId, busHwOptions);
		openedBusHardware = busHwId;
	}

	catch (const nanolib_exception& e) {
		//"Error occurred e.what();
		DBOUT("exception: " << e.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::scanBus(std::vector<std::string>& devices) {
	try {
		if (!openedBusHardware.has_value()) {
			return EXIT_FAILURE;
		}
		// Scan the bus for available devices
		std::vector<nlc::DeviceId> deviceIds = nanolibHelper.scanBus(*openedBusHardware);

		for (int i = 0; i < deviceIds.size(); i++) {
			std::stringstream ss;
			ss << deviceIds[i].getDeviceId() << " : " << deviceIds[i].getDescription();
			devices.push_back(ss.str());
		}
	}
	catch (const nanolib_exception& e) {
		//"Error occurred e.what();
		DBOUT("exception: " << e.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::moveToPosition(INT32 zehntelMM) {
	try {
		if (!openedBusHardware.has_value() || !connectedDeviceHandle.has_value()) {
			//"Closing the hardware bus."
			return EXIT_FAILURE;
		}

		//make sure operation is disabled before changing user defined units
		PowerSM powerSM(nanolibHelper, openedBusHardware.value(), connectedDeviceHandle.value());
		if (powerSM.disableOperation())
			return EXIT_FAILURE;

		UINT16 uWord16;

		//Mode of operation to profile position mode
		nanolibHelper.writeInteger(*connectedDeviceHandle, 1, nlc::OdIndex(0x6060, 0x00), 8);
		uWord16 = static_cast<UINT16>(nanolibHelper.readInteger(*connectedDeviceHandle, nlc::OdIndex(0x6040, 0x00)));
		//immediate start after bit 4 set
		uWord16 |= 1U << 5;
		//target position abosolut
		uWord16 &= ~(1U << 6);
		nanolibHelper.writeInteger(*connectedDeviceHandle, uWord16, nlc::OdIndex(0x6040, 0x00), 16);

		//set target position
		nanolibHelper.writeInteger(*connectedDeviceHandle, zehntelMM, nlc::OdIndex(0x607A, 0x00), 32);

		//power sm to ready tp switch on

		if (powerSM.enableOperation())
			return EXIT_FAILURE;
		//go
		uWord16 = static_cast<UINT16>(nanolibHelper.readInteger(*connectedDeviceHandle, nlc::OdIndex(0x6040, 0x00)));
		//reset bit 4
		uWord16 &= ~(1U << 4);
		uWord16 &= ~(1U << 8);
		nanolibHelper.writeInteger(*connectedDeviceHandle, uWord16, nlc::OdIndex(0x6040, 0x00), 16);
		uWord16 = static_cast<UINT16>(nanolibHelper.readInteger(*connectedDeviceHandle, nlc::OdIndex(0x6040, 0x00)));
		uWord16 |= 1U << 4;
		nanolibHelper.writeInteger(*connectedDeviceHandle, uWord16, nlc::OdIndex(0x6040, 0x00), 16);

	}
	catch (const nanolib_exception& e) {
		//"Error occurred e.what();
		DBOUT("exception: " << e.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::setFeedConstant( UINT32 pitchZehntelMM) {
	try {
		if (!openedBusHardware.has_value() || !connectedDeviceHandle.has_value()) {
			//"Closing the hardware bus."
			return EXIT_FAILURE;
		}

		uint32_t uWord32;

		//make sure operation is disabled before changing user defined units
		PowerSM powerSM(nanolibHelper, openedBusHardware.value(), connectedDeviceHandle.value());
		if (powerSM.disableOperation())
			return EXIT_FAILURE;

		//write virtual position encoder to 1/rev
		nanolibHelper.writeInteger(*connectedDeviceHandle, 1, nlc::OdIndex(0x608F, 0x01), 32);


		//feed per rev
		nanolibHelper.writeInteger(*connectedDeviceHandle, pitchZehntelMM, nlc::OdIndex(0x6092, 0x01), 32);

	}
	catch (const nanolib_exception& e) {
		//"Error occurred e.what();
		DBOUT("exception: " << e.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::setProfileVelocity(UINT32 rpm) {
	try {
		if (!openedBusHardware.has_value() || !connectedDeviceHandle.has_value()) {
			//"Closing the hardware bus."
			return EXIT_FAILURE;
		}

		//make sure operation is disabled before changing user defined units
		PowerSM powerSM(nanolibHelper, openedBusHardware.value(), connectedDeviceHandle.value());
		if (powerSM.disableOperation())
			return EXIT_FAILURE;

		uint32_t uWord32;

		//set profile velocity
		nanolibHelper.writeInteger(*connectedDeviceHandle, rpm, nlc::OdIndex(0x6081, 0x00), 32);
	}
	catch (const nanolib_exception& e) {
		//"Error occurred e.what();
		DBOUT("exception: " << e.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::halt(UINT decelreationMode) {
	try {
		if (!openedBusHardware.has_value() || !connectedDeviceHandle.has_value()) {
			//"Closing the hardware bus."
			return EXIT_FAILURE;
		}
		//write stopmode
		nanolibHelper.writeInteger(*connectedDeviceHandle, decelreationMode, nlc::OdIndex(0x604D, 0x00), 16);
		//halt

		UINT16 uWord16 = static_cast<UINT16>(nanolibHelper.readInteger(*connectedDeviceHandle, nlc::OdIndex(0x6040, 0x00)));
		uWord16 |= 1U << 8;
		nanolibHelper.writeInteger(*connectedDeviceHandle, uWord16, nlc::OdIndex(0x6040, 0x00), 16);
	}
	catch (const nanolib_exception& e) {
		//"Error occurred e.what();
		DBOUT("exception: " << e.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::stop() {
	try {
		if (!openedBusHardware.has_value() || !connectedDeviceHandle.has_value()) {
			//"Closing the hardware bus."
			return EXIT_FAILURE;
		}

		//motor stop
		nanolibHelper.writeInteger(*connectedDeviceHandle, 6, nlc::OdIndex(0x6040, 0x00), 8);
	}
	catch (const nanolib_exception& e) {
		//"Error occurred e.what();
		DBOUT("exception: " << e.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::home() {
	try {
		if (!openedBusHardware.has_value() || !connectedDeviceHandle.has_value()) {
			//"Closing the hardware bus."
			return EXIT_FAILURE;
		}

		UINT16 uWord16;

		//make sure operation is disabled before changing user defined units
		PowerSM powerSM(nanolibHelper, openedBusHardware.value(), connectedDeviceHandle.value());
		if (powerSM.disableOperation())
			return EXIT_FAILURE;

		//enable reference switches, set inputs to 24V range
		if (configureInputs())
			return EXIT_FAILURE;

		//Mode of operation to Homing
		nanolibHelper.writeInteger(*connectedDeviceHandle, 6, nlc::OdIndex(0x6060, 0x00), 8);


		//reference is negative end switch method 17
		nanolibHelper.writeInteger(*connectedDeviceHandle, 17, nlc::OdIndex(0x6098, 0x00), 8);
		if (powerSM.enableOperation())
			return EXIT_FAILURE;
		//go
		uWord16 = static_cast<UINT16>(nanolibHelper.readInteger(*connectedDeviceHandle, nlc::OdIndex(0x6040, 0x00)));
		uWord16 |= 1U << 4;
		nanolibHelper.writeInteger(*connectedDeviceHandle, uWord16, nlc::OdIndex(0x6040, 0x00), 16);
	}
	catch (const nanolib_exception& e) {
		//"Error occurred e.what();
		DBOUT("exception: " << e.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::moveVelocity(INT16 rpm) {
	try {
		if (!openedBusHardware.has_value() || !connectedDeviceHandle.has_value()) {
			//"Closing the hardware bus."
			return EXIT_FAILURE;
		}


		//make sure operation is disabled before changing user defined units
		PowerSM powerSM(nanolibHelper, openedBusHardware.value(), connectedDeviceHandle.value());
		if (powerSM.disableOperation())
			return EXIT_FAILURE;


		//Mode of operation to Velocity
		nanolibHelper.writeInteger(*connectedDeviceHandle, 2, nlc::OdIndex(0x6060, 0x00), 8);
		//Revs
		nanolibHelper.writeInteger(*connectedDeviceHandle, rpm, nlc::OdIndex(0x6042, 0x00), 16);

		//power sm to operation enabled
		if (powerSM.enableOperation())
			return EXIT_FAILURE;

	}
	catch (const nanolib_exception& e) {
		//"Error occurred e.what();
		DBOUT("exception: " << e.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::disconnectDevice() {
	try {
		if (!openedBusHardware.has_value() || !connectedDeviceHandle.has_value()) {
			//"Closing the hardware bus."
			return EXIT_FAILURE;
		}

		nanolibHelper.disconnectDevice(*connectedDeviceHandle);
		nanolibHelper.removeDevice(*connectedDeviceHandle);
		connectedDeviceHandle.reset();
	}
	catch (const nanolib_exception& e) {
		//"Error occurred e.what();
		DBOUT("exception: " << e.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int Controller::connectDevice(UINT deviceToOpen) {
	try {
		if (!openedBusHardware.has_value()) {
			return EXIT_FAILURE;
		}
		nlc::DeviceId deviceId = nlc::DeviceId(*openedBusHardware, deviceToOpen, "");
		std::vector<nlc::DeviceId> deviceIds = nanolibHelper.scanBus(openedBusHardware.value());
		if (deviceToOpen >= deviceIds.size()) {
			//Invalid bus hardware number."
			return EXIT_FAILURE;
		}

		// Register the device id
		nlc::DeviceHandle deviceHandle = nanolibHelper.addDevice(deviceIds[deviceToOpen]);
		// Establishing a connection with the device
		nanolibHelper.connectDevice(deviceHandle);

		connectedDeviceHandle = deviceHandle;

	}
	catch (const nanolib_exception& e) {
		//"Error occurred e.what();
		DBOUT("exception: " << e.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int Controller::autoSetupMotPams() {
	try {
		if (!openedBusHardware.has_value() || !connectedDeviceHandle.has_value()) {
			//"Closing the hardware bus."
			return EXIT_FAILURE;
		}

		uint16_t uWord16;
		uint32_t uWord32;

		//Motor Stop (0x6040-0)
		nanolibHelper.writeInteger(*connectedDeviceHandle, 6, nlc::OdIndex(0x6040, 0x00), 16);
		//write polpaarzahl
		nanolibHelper.writeInteger(*connectedDeviceHandle, 50, nlc::OdIndex(0x2030, 0x00), 32);
		// write motorstrom
		nanolibHelper.writeInteger(*connectedDeviceHandle, 1000, nlc::OdIndex(0x2031, 0x00), 32);

		uWord32 = static_cast<uint32_t>(nanolibHelper.readInteger(*connectedDeviceHandle, nlc::OdIndex(0x3202, 0x00)));
		// set open loop mode
		uWord32 &= ~(1UL << 0);
		//set current reduction
		uWord32 |= 1UL << 3;
		nanolibHelper.writeInteger(*connectedDeviceHandle, uWord32, nlc::OdIndex(0x3202, 0x00), 32);

		//Parameter Ermittlung
		nanolibHelper.writeInteger(*connectedDeviceHandle, -2, nlc::OdIndex(0x6060, 0x00), 8);
		//power sm to operation enabled
		PowerSM powerSM(nanolibHelper, openedBusHardware.value(), connectedDeviceHandle.value());
		if (powerSM.enableOperation())
			return EXIT_FAILURE;
		//start auto-setup
		uWord16 = static_cast<uint16_t>(nanolibHelper.readInteger(*connectedDeviceHandle, nlc::OdIndex(0x6040, 0x00)));
		uWord16 |= 1UL << 4;
		nanolibHelper.writeInteger(*connectedDeviceHandle, uWord16, nlc::OdIndex(0x6040, 0x00), 16);
		//wait till its done
		using namespace std::chrono_literals; // ns, us, ms, s, h, etc.

		do {
			uWord16 = static_cast<uint16_t>(
				nanolibHelper.readInteger(*connectedDeviceHandle, nlc::OdIndex(0x6041, 0x00)));
			DBOUT("Auto Setup running");
			std::this_thread::sleep_for(100ns);
		} while (
			((uWord16 >> 12) & 1U) == 0);

		uWord16 = 0;
		nanolibHelper.writeInteger(*connectedDeviceHandle, uWord16, nlc::OdIndex(0x6040, 0x00), 16);


	}
	catch (const nanolib_exception& e) {
		//"Error occurred e.what();
		DBOUT("exception: " << e.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
//read all 5 inputs
int Controller::readDigitalInputs(UINT8 &state) {
	try {
		if (!openedBusHardware.has_value() || !connectedDeviceHandle.has_value()) {
			//"Closing the hardware bus."
			return EXIT_FAILURE;
		}
		UINT32 uWord32 = static_cast<UINT32>(nanolibHelper.readInteger(*connectedDeviceHandle, nlc::OdIndex(0x60FD, 0x00)));
		state = (uWord32 >> 16) & 0xFF;

	}
	catch (const nanolib_exception& e) {
		//"Error occurred e.what();
		DBOUT("exception: " << e.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::configureInputs() {
	try {
		if (!openedBusHardware.has_value() || !connectedDeviceHandle.has_value()) {
			//"Closing the hardware bus."
			return EXIT_FAILURE;
		}
		//set digital inputs to range 24V
		nanolibHelper.writeInteger(*connectedDeviceHandle, 31, nlc::OdIndex(0x3240, 0x06), 32);
		//set input 1 and 2 to negative, positive endswitch
		nanolibHelper.writeInteger(*connectedDeviceHandle, 3, nlc::OdIndex(0x3240, 0x01), 32);

	}
	catch (const nanolib_exception& e) {
		//"Error occurred e.what();
		DBOUT("exception: " << e.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::getFirmwareVersion(std::string& version) {
	try {
		if (!openedBusHardware.has_value() || !connectedDeviceHandle.has_value()) {
			//"Closing the hardware bus."
			return EXIT_FAILURE;
		}

		version = nanolibHelper.readString(*connectedDeviceHandle, nlc::OdIndex(0x100A, 0x00));

	}
	catch (const nanolib_exception& e) {
		//"Error occurred e.what();
		DBOUT("exception: " << e.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}