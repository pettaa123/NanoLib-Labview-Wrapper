#include "pch.h"
#include "controller.h"

#include <chrono>
#include <thread>

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

int Controller::openPort(unsigned int portToOpen) {
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

		return EXIT_SUCCESS;
	}

	catch (const nanolib_exception& e) {
		//"Error occurred e.what();
		DBOUT("exception: " << e.what());
		return EXIT_FAILURE;
	}
}

int Controller::scanBus(std::vector<std::string>& devices) {
	try {
		if (!openedBusHardware.has_value()) {
			//"Closing the hardware bus."
			return EXIT_FAILURE;
		}
		// Scan the bus for available devices
		std::vector<nlc::DeviceId> deviceIds = nanolibHelper.scanBus(openedBusHardware.value());

		for (int i = 0; i < deviceIds.size(); i++) {
			std::stringstream ss;
			ss << deviceIds[i].getDeviceId() << " : " << deviceIds[i].getDescription();
			devices.push_back(ss.str());
		}
		return EXIT_SUCCESS;
	}
	catch (const nanolib_exception& e) {
		//"Error occurred e.what();
		DBOUT("exception: " << e.what());
		return EXIT_FAILURE;
	}
}

int Controller::connectDevice(unsigned int deviceToOpen) {
	try {
		if (!openedBusHardware.has_value() || connectedDeviceHandle.has_value()) {
			//"Closing the hardware bus."
			//return EXIT_FAILURE;
			return 9;
		}
		std::vector<nlc::DeviceId> deviceIds = nanolibHelper.scanBus(openedBusHardware.value());
		// Use selected device. 
		// We could also create device id manually, as follows: 
		// nlc::DeviceId deviceId = nlc::DeviceId(busHwId, 1, "");
		nlc::DeviceId deviceId = deviceIds[deviceToOpen];

		// Register the device id
		nlc::DeviceHandle deviceHandle = nanolibHelper.addDevice(deviceId);

		// Establishing a connection with the device
		nanolibHelper.connectDevice(deviceHandle);
		connectedDeviceHandle = deviceHandle;
		permConnectedDeviceHandle = deviceHandle;

		nlc::DeviceConnectionStateInfo cs = nanolibHelper.getConnectionState(*connectedDeviceHandle).getResult();
		nlc::DeviceConnectionStateInfo cs2 = nanolibHelper.getConnectionState(permConnectedDeviceHandle).getResult();
		DBOUT("exception: " << connectedDeviceHandle->toString().c_str() << " : " << (int)cs << permConnectedDeviceHandle.toString().c_str() <<" : " << (int)cs2);
		// Now we are ready to work with the controller.

		return EXIT_SUCCESS;
	}

	catch (const nanolib_exception& e) {
		//"Error occurred e.what();
		DBOUT("exception: " << e.what());
		return EXIT_FAILURE;
	}
}

int Controller::autoSetupMotPams() {
	try {
		if (!connectedDeviceHandle.has_value()) {
			return EXIT_FAILURE;
		}

		uint16_t uWord16;
		uint32_t uWord32;
		DBOUT("exception: " << connectedDeviceHandle->toString().c_str() << " : " << permConnectedDeviceHandle.toString().c_str());
		nlc::DeviceConnectionStateInfo cs = nanolibHelper.getConnectionState(*connectedDeviceHandle).getResult();
		nlc::DeviceConnectionStateInfo cs2 = nanolibHelper.getConnectionState(permConnectedDeviceHandle).getResult();
		DBOUT("exception: " << connectedDeviceHandle->toString().c_str() << " : " << (int)cs << permConnectedDeviceHandle.toString().c_str() << " : " << (int)cs2);
		
		DBOUT("exception: " << (int)cs);
		//Motor Stop (0x6040-0)
		nanolibHelper.writeInteger(permConnectedDeviceHandle, 6, nlc::OdIndex(0x6040, 0x00), 16);
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
		//sm to operation enabled
		uWord16 = static_cast<uint16_t>(nanolibHelper.readInteger(*connectedDeviceHandle, nlc::OdIndex(0x6041, 0x00)));
		//Ready to switch on?
		if (
			((uWord16 >> 0) & 1U) == 1 &&
			((uWord16 >> 1) & 1U) == 0 &&
			((uWord16 >> 2) & 1U) == 0 &&
			((uWord16 >> 3) & 1U) == 0 &&
			((uWord16 >> 5) & 1U) == 1 &&
			((uWord16 >> 6) & 1U) == 0
			) {
			//switch on
			nanolibHelper.writeInteger(*connectedDeviceHandle, 7, nlc::OdIndex(0x6040, 0x00), 16);
			// operation enabled
			nanolibHelper.writeInteger(*connectedDeviceHandle, 15, nlc::OdIndex(0x6040, 0x00), 16);
		}
		//start auto-setup
		uWord16 = static_cast<uint16_t>(nanolibHelper.readInteger(*connectedDeviceHandle, nlc::OdIndex(0x6040, 0x00)));
		uWord16 |= 1UL << 4;
		nanolibHelper.writeInteger(*connectedDeviceHandle, uWord16, nlc::OdIndex(0x6040, 0x00), 16);
		//wait till its done
		using namespace std::chrono_literals; // ns, us, ms, s, h, etc.

		do {
			uWord16 = static_cast<uint16_t>(
				nanolibHelper.readInteger(*connectedDeviceHandle, nlc::OdIndex(0x6041, 0x00)));
			//Auto Setup running
			std::this_thread::sleep_for(100ns);
		} while (
			((uWord16 >> 12) & 1U) == 0);

		uWord16 = 0;
		nanolibHelper.writeInteger(*connectedDeviceHandle, uWord16, nlc::OdIndex(0x6040, 0x00), 16);

		return EXIT_SUCCESS;
	}
	catch (const nanolib_exception& e) {
		//"Error occurred e.what();
		DBOUT("exception: " << e.what());
		return EXIT_FAILURE;
	}
}