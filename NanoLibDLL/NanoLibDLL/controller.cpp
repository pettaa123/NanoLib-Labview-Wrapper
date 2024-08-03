#include <format>
#include <cstdint>

#include "controller.h"
#include "userUnits.h"
#include "magic_enum.hpp"


#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
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

Controller::Controller() {
	// its possible to set the logging level to a different level
	m_nanolibHelper.setLoggingLevel(nlc::LogLevel::Error);

	m_powerSM = std::make_unique<PowerSM>(&m_nanolibHelper, &m_connectedDeviceHandle);
}

Controller::~Controller() {
	
	if (m_openedBusHardware.has_value() && m_connectedDeviceHandle.has_value()) {
		m_powerSM->shutdown();
	}

	// Always finalize connected hardware
	if (m_connectedDeviceHandle.has_value()) {
		//"Disconnecting the device."
		m_nanolibHelper.disconnectDevice(m_connectedDeviceHandle.value());
	}

	if (m_openedBusHardware.has_value()) {
		//"Closing the hardware bus."
		m_nanolibHelper.closeBusHardware(m_openedBusHardware.value());
	}
}

//***GENERALS***

int Controller::checkConnection() {
	if (!m_openedBusHardware.has_value() || !m_connectedDeviceHandle.has_value()) {
		throw nanolib_exception("No connected device");
		return EXIT_FAILURE;
	}
	if (m_nanolibHelper.checkedResult("getDeviceState", m_nanolibHelper->getConnectionState(*m_connectedDeviceHandle)).getResult() != nlc::DeviceConnectionStateInfo::Connected) {
		throw nanolib_exception("No connected device");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::closePort() {
	try {
		checkConnection();
		m_powerSM->disableOperation();

		// Always finalize connected hardware

		if (m_connectedDeviceHandle.has_value()) {
			//"Disconnecting the device."
			m_nanolibHelper.disconnectDevice(*m_connectedDeviceHandle);
			m_nanolibHelper.removeDevice(*m_connectedDeviceHandle);
			m_connectedDeviceHandle.reset();
		}

		if (m_openedBusHardware.has_value()) {
			//"Closing the hardware bus."
			m_nanolibHelper.closeBusHardware(*m_openedBusHardware);
			m_openedBusHardware.reset();
		}
	}
	catch (const nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::rebootDevice() {
	try {
		checkConnection();
		m_nanolibHelper.checkedResult("rebootDevice", m_nanolibHelper->rebootDevice(*m_connectedDeviceHandle));
	}
	catch (const nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::getDeviceFirmwareBuildId(std::string& version) {
	try {
		checkConnection();
		version = m_nanolibHelper.checkedResult("getDeviceFirmwareBuildId", m_nanolibHelper->getDeviceFirmwareBuildId(*m_connectedDeviceHandle)).getResult();
	}
	catch (const nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}


int Controller::getExceptions(std::vector<std::string>& exceptions) {
	for (auto e : m_exceptions)
		exceptions.push_back(e.what());
	return EXIT_SUCCESS;
}

int Controller::getAvailablePorts(std::vector<std::string>& ports) {
	try {
		// list all hardware available
		std::vector<nlc::BusHardwareId> busHardwareIds = m_nanolibHelper.getBusHardware();

		if (busHardwareIds.empty()) {
			// "No hardware buses found." << std::endl;
			throw nanolib_exception("No bus found");
		}

		ports.clear();

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
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::openPort(uint32_t portToOpen) {
	try {
		if (m_openedBusHardware.has_value()) {
			closePort();
		}
		// its possible to set the logging level to a different level
		m_nanolibHelper.setLoggingLevel(nlc::LogLevel::Error);

		// list all hardware available
		std::vector<nlc::BusHardwareId> busHardwareIds = m_nanolibHelper.getBusHardware();

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
		nlc::BusHardwareOptions busHwOptions = m_nanolibHelper.createBusHardwareOptions(busHwId);
		// now able to open the hardware itself
		m_nanolibHelper.openBusHardware(busHwId, busHwOptions);
		m_openedBusHardware = busHwId;

	}

	catch (const nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}


int Controller::scanBus(std::vector<std::string>& devices) {
	try {
		if (!m_openedBusHardware.has_value()) {
			throw nanolib_exception("No Port opened");
			return EXIT_FAILURE;
		}
		// Scan the bus for available devices
		std::vector<nlc::DeviceId> deviceIds = m_nanolibHelper.scanBus(*m_openedBusHardware);

		devices.clear();

		for (int i = 0; i < deviceIds.size(); i++) {
			std::stringstream ss;
			ss << i << ". " << deviceIds[i].getDescription();
			devices.push_back(ss.str());
		}
	}
	catch (const nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::connectDevice(uint32_t deviceToOpen) {
	try {
		if (!m_openedBusHardware.has_value()) {
			throw(nanolib_exception("can't connect: no port opened."));
			return EXIT_FAILURE;
		}
		if (m_connectedDeviceHandle.has_value()) {
			disconnectDevice();
		}

		std::vector<nlc::DeviceId> deviceIds = m_nanolibHelper.scanBus(m_openedBusHardware.value());
		if (deviceToOpen >= deviceIds.size()) {
			throw(nanolib_exception("Invalid bus hardware number."));
			return EXIT_FAILURE;
		}
		nlc::DeviceHandle deviceHandle;
		// Register the device id

		deviceHandle = m_nanolibHelper.addDevice(deviceIds[deviceToOpen]);

		// Establishing a connection with the device
		m_nanolibHelper.connectDevice(deviceHandle);

		m_connectedDeviceHandle = deviceHandle;

	}
	catch (const nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int Controller::disconnectDevice() {
	try {
		checkConnection();
		m_nanolibHelper.disconnectDevice(*m_connectedDeviceHandle);
		m_nanolibHelper.removeDevice(*m_connectedDeviceHandle);
		m_connectedDeviceHandle.reset();
	}
	catch (const nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int Controller::setMotorParameters(uint32_t polePairCount, uint32_t ratedCurrent, uint32_t maxCurrent,uint32_t maxCurrentDuration, uint32_t idleCurrent, uint32_t driveMode) {
	try {
		checkConnection();
		//take whichever motor for this
		Motor402 mot(&m_nanolibHelper, &m_connectedDeviceHandle, &(*m_powerSM));
		if (mot.setMotorParameters(polePairCount, ratedCurrent, maxCurrent, maxCurrentDuration,idleCurrent, static_cast<Motor402::DriveMode>(driveMode)))
			return EXIT_FAILURE;
	}
	catch (const nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::getMotorParameters(uint32_t& polePairCount, uint32_t& ratedCurrent, uint32_t& maxCurrent, uint32_t& maxCurrentTime, uint32_t &idleCurrent, uint32_t& driveMode) {
	try{ 
		checkConnection();
		Motor402 mot(&m_nanolibHelper, &m_connectedDeviceHandle, &(*m_powerSM));
		Motor402::DriveMode driveMode_t;
		mot.getMotorParameters(polePairCount, ratedCurrent, maxCurrent, maxCurrentTime,idleCurrent, driveMode_t);
		driveMode = driveMode_t;
	}
	catch (nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::saveGroupMovement() {
	try {
		checkConnection();
		Motor402 mot(&m_nanolibHelper, &m_connectedDeviceHandle, &(*m_powerSM));
		mot.saveGroup(0x05);
	}
	catch (nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::saveGroupApplication() {
	try {
		checkConnection();
		Motor402 mot(&m_nanolibHelper, &m_connectedDeviceHandle, &(*m_powerSM));
		mot.saveGroup(0x03);
	}
	catch (nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::saveGroupTuning() {
	try {
		checkConnection();
		Motor402 mot(&m_nanolibHelper, &m_connectedDeviceHandle, &(*m_powerSM));
		mot.saveGroup(0x06);
	}
	catch (nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::halt() {
	try {
		checkConnection();
		Motor402 mot(&m_nanolibHelper, &m_connectedDeviceHandle, &(*m_powerSM));
		mot.halt();
	}
	catch (const nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}



int Controller::getDeviceErrorStack(std::vector<std::string>& errorStackStrings) {
	try {
		checkConnection();
		std::vector<int64_t> errorStack = m_nanolibHelper.readArray(*m_connectedDeviceHandle, 0x1003);
		uint8_t numberOfErrors = static_cast<uint8_t>(errorStack.at(0));
		DBOUT("Elements in error stack: " << std::to_string(numberOfErrors).c_str() << std::endl);
		for (size_t i = 1; i <= numberOfErrors; i++) {
			uint32_t error = static_cast<uint32_t>(errorStack.at(i));

			std::string errorString = std::string(std::format("Error Number: {} Code: {}", std::to_string(error >> 24), std::format("{:x}", error & 0xFFFF)));
			DBOUT(errorString.c_str());
			errorStackStrings.push_back(errorString);
		}
	}
	catch (const nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;


}

int Controller::quickStop() {
	try {
		checkConnection();
		//quick stop
		if (m_powerSM->quickStop())
			return EXIT_FAILURE;
	}
	catch (const nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::getPositionActual(int32_t& position) {
	try {
		checkConnection();
		Motor402 mot(&m_nanolibHelper, &m_connectedDeviceHandle, &(*m_powerSM));
		position = mot.getPositionActual();
	}
	catch (const nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int Controller::setUserUnitsFeed(uint32_t feedPer,uint32_t shaftRevolutions) {
	try {
		checkConnection();
		Motor402 mot(&m_nanolibHelper, &m_connectedDeviceHandle, &(*m_powerSM));
		mot.setUserUnitsFeed(feedPer,shaftRevolutions);
	}
	catch (const nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::getUserUnitsFeed(uint32_t& feedPer,uint32_t &shaftRevolutions) {
	try {
		checkConnection();
		feedPer = static_cast<uint32_t>(m_nanolibHelper.readInteger(*m_connectedDeviceHandle, nlc::OdIndex(0x6092, 0x01)));
		shaftRevolutions = static_cast<uint32_t>(m_nanolibHelper.readInteger(*m_connectedDeviceHandle, nlc::OdIndex(0x6092, 0x02)));
	}
	catch (const nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
int Controller::getUserUnitsGearRatio(uint32_t& gearRatioMotorRevs,uint32_t& gearRatioShaftRevs) {
	try {
		checkConnection();
		gearRatioMotorRevs = static_cast<uint32_t>(m_nanolibHelper.readInteger(*m_connectedDeviceHandle, nlc::OdIndex(0x6091, 0x01)));
		gearRatioShaftRevs = static_cast<uint32_t>(m_nanolibHelper.readInteger(*m_connectedDeviceHandle, nlc::OdIndex(0x6091, 0x02)));
	}
	catch (const nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

//read all 5 inputs
int Controller::readDigitalInputs(uint8_t& states) {
	try {
		checkConnection();
		uint32_t uWord32 = static_cast<uint32_t>(m_nanolibHelper.readInteger(*m_connectedDeviceHandle, nlc::OdIndex(0x60FD, 0x00)));
		states = (uWord32 >> 16) & 0xFF;

	}
	catch (const nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}


int Controller::configureInputs() {
	try {
		checkConnection();
		uint32_t uWord32 = 0;
		//set digital inputs to range 24V
		uWord32 |= (1UL << 0);
		uWord32 |= (1UL << 1);
		m_nanolibHelper.writeInteger(*m_connectedDeviceHandle, uWord32, nlc::OdIndex(0x3240, 0x06), 32);

		//set opener logic
		m_nanolibHelper.writeInteger(*m_connectedDeviceHandle, uWord32, nlc::OdIndex(0x3240, 0x02), 32);

		//set input 1 to negative
		//set input 2 to positive endswitch
		m_nanolibHelper.writeInteger(*m_connectedDeviceHandle, uWord32, nlc::OdIndex(0x3240, 0x01), 32);
		//Limit Switch Error Option Code
		/*
		keine Reaktion (um z. B. eine Referenzfahrt durchzuführen), außer
		Vermerken der Endschalterposition
		*/
		m_nanolibHelper.writeInteger(*m_connectedDeviceHandle, -1, nlc::OdIndex(0x3701, 0x00), 16);

	}
	catch (const nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::getModeOfOperation(std::string& mode) {
	try {
		checkConnection();
		Motor402 mot(&m_nanolibHelper, &m_connectedDeviceHandle, &(*m_powerSM));
		Motor402::OperationMode opMode=static_cast<Motor402::OperationMode>(mot.getModeOfOperation());
		auto enum_name = magic_enum::enum_name(opMode);
		mode = enum_name;
	}
	catch (const nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::getCiA402State(std::string& state, bool &fault,bool &voltageEnabled,bool &quickStop,bool &warning, bool &targetReached, bool &limitReached, bool &bit12, bool &bit13) {
	try {
		checkConnection();
		uint8_t opState;
		if (m_powerSM->getCurrentState(opState))
			return EXIT_FAILURE;
		auto enum_name = magic_enum::enum_name(static_cast<PowerSM::States>(opState));
		state = enum_name;
		uint16_t uWord16 = static_cast<uint16_t>(m_nanolibHelper.readInteger(*m_connectedDeviceHandle, nlc::OdIndex(0x6041, 0x00)));
		fault = uWord16 & (1U << 3);
		voltageEnabled = uWord16 & (1U << 4);
		quickStop = uWord16 & (1U << 5);
		warning = uWord16 & (1U << 7);
		targetReached = uWord16 & (1U << 10);
		limitReached = uWord16 & (1U << 11);
		bit12 = uWord16 & (1U << 12);
		bit13 = uWord16 & (1U << 13);
	}
	catch (const nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

//**AUTO-SETUP***

int Controller::autoSetupMotPams() {
	try {
		checkConnection();
		AutoSetupMotor mot(&m_nanolibHelper, &m_connectedDeviceHandle, &(*m_powerSM));
		mot.autoSetupMotPams();
	}
	catch (const nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

//***HOMING***

int Controller::home(uint32_t speedZero, uint32_t speedSwitch) {
	try {
		checkConnection();

		//enable reference switches, set inputs to 24V range
		if (configureInputs())
			EXIT_FAILURE;

		HomingMotor mot(&m_nanolibHelper, &m_connectedDeviceHandle, &(*m_powerSM));
		if (mot.home(speedZero, speedSwitch))
			return EXIT_FAILURE;

	}
	catch (nanolib_exception e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}


int Controller::setHomingAcceleration(uint32_t acc) {
	try {
		checkConnection();
		HomingMotor mot(&m_nanolibHelper, &m_connectedDeviceHandle, &(*m_powerSM));
		mot.setHomingAcceleration(acc);
	}
	catch (const nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

//***POSITIONING***


int Controller::setTargetPosition(int32_t value, uint32_t absRel) {
	try {
		checkConnection();
		ProfilePositionMotor mot(&m_nanolibHelper, &m_connectedDeviceHandle, &(*m_powerSM));
		mot.setTargetPosition(value,absRel);
	}
	catch (const nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::setProfileAcceleration(uint32_t acc) {
	try {
		checkConnection();
		ProfilePositionMotor mot(&m_nanolibHelper, &m_connectedDeviceHandle, &(*m_powerSM));
		mot.setProfileAcceleration(acc);
	}
	catch (const nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::setProfileVelocity(uint32_t speed) {
	try {
		checkConnection();
		ProfilePositionMotor mot(&m_nanolibHelper, &m_connectedDeviceHandle, &(*m_powerSM));
		mot.setProfileVelocity(speed);
	}
	catch (const nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::getPositioningParameters(uint32_t& profileVelocity, int32_t& targetPosition) {
	try {
		checkConnection();
		ProfilePositionMotor mot(&m_nanolibHelper, &m_connectedDeviceHandle, &(*m_powerSM));
		mot.getPositioningParameters(profileVelocity, targetPosition);
	}
	catch (const nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::startPositioning() {
	try {
		checkConnection();
		ProfilePositionMotor mot(&m_nanolibHelper, &m_connectedDeviceHandle, &(*m_powerSM));
		mot.startPositioning();
	}
	catch (const nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::getUserUnitsPositioning(uint32_t& unit, uint32_t& exp) {
	try {
		checkConnection();
		ProfilePositionMotor mot(&m_nanolibHelper, &m_connectedDeviceHandle, &(*m_powerSM));
		mot.getUserUnitsPositioning(unit,exp);
	}
	catch (const nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::setUserUnitsPositioning(uint32_t posUnit, uint32_t posExp) {
	try {
		checkConnection();
		ProfilePositionMotor mot(&m_nanolibHelper, &m_connectedDeviceHandle, &(*m_powerSM));
		mot.setUserUnitsPositioning(posUnit, posExp);
	}
	catch (const nanolib_exception& e) {
		DBOUT("exception: " << e.what());
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}


//***VELOCITY***

int Controller::setTargetVelocity(int16_t vel) {
	try {
		checkConnection();
		VelocityMotor mot(&m_nanolibHelper, &m_connectedDeviceHandle, &(*m_powerSM));
		mot.setTargetVelocity(vel);
	}
	catch (const nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::startVelocity() {
	try {
		checkConnection();
		VelocityMotor mot(&m_nanolibHelper, &m_connectedDeviceHandle, &(*m_powerSM));
		mot.startVelocity();
	}
	catch (const nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
int Controller::setVelocityAcceleration(uint32_t deltaSpeed, uint16_t deltaTime) {
try {
	checkConnection();
	VelocityMotor mot(&m_nanolibHelper, &m_connectedDeviceHandle, &(*m_powerSM));
	mot.setVelocityAcceleration(deltaSpeed,deltaTime );
}
catch (const nanolib_exception& e) {
	m_exceptions.push_back(e);
	return EXIT_FAILURE;
}
return EXIT_SUCCESS;
}

int Controller::getVelocityDemanded(int16_t &velDemanded) {
	try {
		checkConnection();
		Motor402 mot(&m_nanolibHelper, &m_connectedDeviceHandle, &(*m_powerSM));
		mot.getVelocityDemanded(velDemanded);
	}
	catch (const nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::getVelocityActual(int16_t& velActual) {
	try {
		checkConnection();
		Motor402 mot(&m_nanolibHelper, &m_connectedDeviceHandle, &(*m_powerSM));
		mot.getVelocityActual(velActual);
	}
	catch (const nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::setVelocityDeceleration(uint32_t deltaSpeed, uint16_t deltaTime) {
	try {
		checkConnection();
		VelocityMotor mot(&m_nanolibHelper, &m_connectedDeviceHandle, &(*m_powerSM));
		mot.setVelocityDeceleration(deltaSpeed, deltaTime);
	}
	catch (const nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::getTargetVelocity(int16_t& vel) {
	try {
		checkConnection();
		VelocityMotor mot(&m_nanolibHelper, &m_connectedDeviceHandle, &(*m_powerSM));
		mot.getTargetVelocity(vel);
	}
	catch (const nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
int Controller::getVelocityAcceleration(uint32_t& deltaSpeed, uint16_t& deltaTime){
	try {
		checkConnection();
		VelocityMotor mot(&m_nanolibHelper, &m_connectedDeviceHandle, &(*m_powerSM));
		mot.getVelocityAcceleration(deltaSpeed,deltaTime);
	}
	catch (const nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
int Controller::getVelocityDeceleration(uint32_t& deltaSpeed, uint16_t& deltaTime){
	try {
		checkConnection();
		VelocityMotor mot(&m_nanolibHelper, &m_connectedDeviceHandle, &(*m_powerSM));
		mot.getVelocityDeceleration(deltaSpeed,deltaTime);
	}
	catch (const nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}


int Controller::setUserUnitsVelocity(uint32_t velUnit, uint32_t velExp, uint32_t velTime) {
	try {
		checkConnection();
		VelocityMotor mot(&m_nanolibHelper, &m_connectedDeviceHandle, &(*m_powerSM));
		mot.setUserUnitsVelocity(velUnit, velExp,velTime);
	}
	catch (const nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}



int Controller::getUserUnitsVelocity(uint32_t &unit, uint32_t& exp, uint32_t& time) {
	try {
		checkConnection();
		VelocityMotor mot(&m_nanolibHelper, &m_connectedDeviceHandle, &(*m_powerSM));
		mot.getUserUnitsVelocity(unit,exp, time);
	}
	catch (const nanolib_exception& e) {
		m_exceptions.push_back(e);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}











