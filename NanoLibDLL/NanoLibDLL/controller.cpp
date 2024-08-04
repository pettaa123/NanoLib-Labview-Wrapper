#include <format>
#include <cstdint>

#include "controller.h"
#include "user_units.h"
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
	nanolibHelper_.setLoggingLevel(nlc::LogLevel::Error);

	powerSM_ = std::make_unique<PowerSM>(&nanolibHelper_, &connectedDeviceHandle_);
}

Controller::~Controller() {
	
	if (openedBusHardware_.has_value() && connectedDeviceHandle_.has_value()) {
		powerSM_->Shutdown();
	}

	// Always finalize connected hardware
	if (connectedDeviceHandle_.has_value()) {
		//"Disconnecting the device."
		nanolibHelper_.disconnectDevice(connectedDeviceHandle_.value());
	}

	if (openedBusHardware_.has_value()) {
		//"Closing the hardware bus."
		nanolibHelper_.closeBusHardware(openedBusHardware_.value());
	}
}

//***GENERALS***

int Controller::CheckConnection() {
	if (!openedBusHardware_.has_value() || !connectedDeviceHandle_.has_value()) {
		throw nanolib_exception("No connected device");
		return EXIT_FAILURE;
	}
	if (nanolibHelper_.checkedResult("getDeviceState", nanolibHelper_->getConnectionState(*connectedDeviceHandle_)).getResult() != nlc::DeviceConnectionStateInfo::Connected) {
		throw nanolib_exception("No connected device");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::ClosePort() {
	try {
		CheckConnection();
		powerSM_->DisableOperation();

		// Always finalize connected hardware

		if (connectedDeviceHandle_.has_value()) {
			//"Disconnecting the device."
			nanolibHelper_.disconnectDevice(*connectedDeviceHandle_);
			nanolibHelper_.removeDevice(*connectedDeviceHandle_);
			connectedDeviceHandle_.reset();
		}

		if (openedBusHardware_.has_value()) {
			//"Closing the hardware bus."
			nanolibHelper_.closeBusHardware(*openedBusHardware_);
			openedBusHardware_.reset();
		}
	}
	catch (const nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::RebootDevice() {
	try {
		CheckConnection();
		nanolibHelper_.checkedResult("rebootDevice", nanolibHelper_->rebootDevice(*connectedDeviceHandle_));
	}
	catch (const nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::GetDeviceFirmwareBuildId(std::string& version) {
	try {
		CheckConnection();
		version = nanolibHelper_.checkedResult("getDeviceFirmwareBuildId", nanolibHelper_->getDeviceFirmwareBuildId(*connectedDeviceHandle_)).getResult();
	}
	catch (const nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}


int Controller::GetExceptions(std::vector<std::string>& exceptions) {
	for (auto e : exceptions_)
		exceptions.push_back(e.what());
	return EXIT_SUCCESS;
}

int Controller::GetAvailablePorts(std::vector<std::string>& ports) {
	try {
		ports.clear();

		// list all hardware available
		std::vector<nlc::BusHardwareId> busHardwareIds = nanolibHelper_.getBusHardware();

		if (busHardwareIds.empty()) {
			// "No hardware buses found." << std::endl;
			throw nanolib_exception("No bus found");
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
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::OpenPort(uint32_t portToOpen) {
	try {
		if (openedBusHardware_.has_value()) {
			ClosePort();
		}
		// its possible to set the logging level to a different level
		nanolibHelper_.setLoggingLevel(nlc::LogLevel::Error);

		// list all hardware available
		std::vector<nlc::BusHardwareId> busHardwareIds = nanolibHelper_.getBusHardware();

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
		nlc::BusHardwareOptions busHwOptions = nanolibHelper_.createBusHardwareOptions(busHwId);
		// now able to open the hardware itself
		nanolibHelper_.openBusHardware(busHwId, busHwOptions);
		openedBusHardware_ = busHwId;

	}

	catch (const nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}


int Controller::ScanBus(std::vector<std::string>& devices) {
	try {
		if (!openedBusHardware_.has_value()) {
			throw nanolib_exception("No Port opened");
			return EXIT_FAILURE;
		}
		// Scan the bus for available devices
		std::vector<nlc::DeviceId> deviceIds = nanolibHelper_.scanBus(*openedBusHardware_);

		devices.clear();

		for (int i = 0; i < deviceIds.size(); i++) {
			std::stringstream ss;
			ss << i << ". " << deviceIds[i].getDescription();
			devices.push_back(ss.str());
		}
	}
	catch (const nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::ConnectDevice(uint32_t deviceToOpen) {
	try {
		if (!openedBusHardware_.has_value()) {
			throw(nanolib_exception("can't connect: no port opened."));
			return EXIT_FAILURE;
		}
		if (connectedDeviceHandle_.has_value()) {
			DisconnectDevice();
		}

		std::vector<nlc::DeviceId> deviceIds = nanolibHelper_.scanBus(openedBusHardware_.value());
		if (deviceToOpen >= deviceIds.size()) {
			throw(nanolib_exception("Invalid bus hardware number."));
			return EXIT_FAILURE;
		}
		nlc::DeviceHandle deviceHandle;
		// Register the device id

		deviceHandle = nanolibHelper_.addDevice(deviceIds[deviceToOpen]);

		// Establishing a connection with the device
		nanolibHelper_.connectDevice(deviceHandle);

		connectedDeviceHandle_ = deviceHandle;

	}
	catch (const nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int Controller::DisconnectDevice() {
	try {
		CheckConnection();
		nanolibHelper_.disconnectDevice(*connectedDeviceHandle_);
		nanolibHelper_.removeDevice(*connectedDeviceHandle_);
		connectedDeviceHandle_.reset();
	}
	catch (const nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int Controller::SetMotorParameters(uint32_t polePairCount, uint32_t ratedCurrent, uint32_t maxCurrent,uint32_t maxCurrentDuration, uint32_t idleCurrent, uint32_t driveMode) {
	try {
		CheckConnection();
		//take whichever motor for this
		Motor402 mot(&nanolibHelper_, &connectedDeviceHandle_, &(*powerSM_));
		if (mot.SetMotorParameters(polePairCount, ratedCurrent, maxCurrent, maxCurrentDuration,idleCurrent, static_cast<Motor402::DriveMode>(driveMode)))
			return EXIT_FAILURE;
	}
	catch (const nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::GetMotorParameters(uint32_t& polePairCount, uint32_t& ratedCurrent, uint32_t& maxCurrent, uint32_t& maxCurrentTime, uint32_t &idleCurrent, uint32_t& driveMode) {
	try{ 
		CheckConnection();
		Motor402 mot(&nanolibHelper_, &connectedDeviceHandle_, &(*powerSM_));
		Motor402::DriveMode driveMode_t;
		mot.GetMotorParameters(polePairCount, ratedCurrent, maxCurrent, maxCurrentTime,idleCurrent, driveMode_t);
		driveMode = driveMode_t;
	}
	catch (nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::SaveGroupMovement() {
	try {
		CheckConnection();
		Motor402 mot(&nanolibHelper_, &connectedDeviceHandle_, &(*powerSM_));
		mot.SaveGroup(0x05);
	}
	catch (nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::SaveGroupApplication() {
	try {
		CheckConnection();
		Motor402 mot(&nanolibHelper_, &connectedDeviceHandle_, &(*powerSM_));
		mot.SaveGroup(0x03);
	}
	catch (nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::SaveGroupTuning() {
	try {
		CheckConnection();
		Motor402 mot(&nanolibHelper_, &connectedDeviceHandle_, &(*powerSM_));
		mot.SaveGroup(0x06);
	}
	catch (nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::Halt() {
	try {
		CheckConnection();
		Motor402 mot(&nanolibHelper_, &connectedDeviceHandle_, &(*powerSM_));
		mot.Halt();
	}
	catch (const nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}



int Controller::GetDeviceErrorStack(std::vector<std::string>& errorStackStrings) {
	try {
		CheckConnection();
		std::vector<int64_t> errorStack = nanolibHelper_.readArray(*connectedDeviceHandle_, 0x1003);
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
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;


}

int Controller::QuickStop() {
	try {
		CheckConnection();
		//quick stop
		if (powerSM_->QuickStop())
			return EXIT_FAILURE;
	}
	catch (const nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::GetPositionActual(int32_t& position) {
	try {
		CheckConnection();
		Motor402 mot(&nanolibHelper_, &connectedDeviceHandle_, &(*powerSM_));
		position = mot.GetPositionActual();
	}
	catch (const nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int Controller::SetUserUnitsFeed(uint32_t feedPer,uint32_t shaftRevolutions) {
	try {
		CheckConnection();
		Motor402 mot(&nanolibHelper_, &connectedDeviceHandle_, &(*powerSM_));
		mot.SetUserUnitsFeed(feedPer,shaftRevolutions);
	}
	catch (const nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::GetUserUnitsFeed(uint32_t& feedPer,uint32_t &shaftRevolutions) {
	try {
		CheckConnection();
		feedPer = static_cast<uint32_t>(nanolibHelper_.readInteger(*connectedDeviceHandle_, nlc::OdIndex(0x6092, 0x01)));
		shaftRevolutions = static_cast<uint32_t>(nanolibHelper_.readInteger(*connectedDeviceHandle_, nlc::OdIndex(0x6092, 0x02)));
	}
	catch (const nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
int Controller::GetUserUnitsGearRatio(uint32_t& gearRatioMotorRevs,uint32_t& gearRatioShaftRevs) {
	try {
		CheckConnection();
		gearRatioMotorRevs = static_cast<uint32_t>(nanolibHelper_.readInteger(*connectedDeviceHandle_, nlc::OdIndex(0x6091, 0x01)));
		gearRatioShaftRevs = static_cast<uint32_t>(nanolibHelper_.readInteger(*connectedDeviceHandle_, nlc::OdIndex(0x6091, 0x02)));
	}
	catch (const nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

//read all 5 inputs
int Controller::ReadDigitalInputs(uint8_t& states) {
	try {
		CheckConnection();
		uint32_t uWord32 = static_cast<uint32_t>(nanolibHelper_.readInteger(*connectedDeviceHandle_, nlc::OdIndex(0x60FD, 0x00)));
		states = (uWord32 >> 16) & 0xFF;

	}
	catch (const nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}


int Controller::ConfigureInputs() {
	try {
		CheckConnection();
		uint32_t uWord32 = 0;
		//set digital inputs to range 24V
		uWord32 |= (1UL << 0);
		uWord32 |= (1UL << 1);
		nanolibHelper_.writeInteger(*connectedDeviceHandle_, uWord32, nlc::OdIndex(0x3240, 0x06), 32);

		//set opener logic
		nanolibHelper_.writeInteger(*connectedDeviceHandle_, uWord32, nlc::OdIndex(0x3240, 0x02), 32);

		//set input 1 to negative
		//set input 2 to positive endswitch
		nanolibHelper_.writeInteger(*connectedDeviceHandle_, uWord32, nlc::OdIndex(0x3240, 0x01), 32);
		//Limit Switch Error Option Code
		/*
		keine Reaktion (um z. B. eine Referenzfahrt durchzuführen), außer
		Vermerken der Endschalterposition
		*/
		nanolibHelper_.writeInteger(*connectedDeviceHandle_, -1, nlc::OdIndex(0x3701, 0x00), 16);

	}
	catch (const nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::GetModeOfOperation(std::string& mode) {
	try {
		CheckConnection();
		Motor402 mot(&nanolibHelper_, &connectedDeviceHandle_, &(*powerSM_));
		Motor402::OperationMode opMode=static_cast<Motor402::OperationMode>(mot.GetModeOfOperation());
		auto enum_name = magic_enum::enum_name(opMode);
		mode = enum_name;
	}
	catch (const nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::GetCiA402State(std::string& state, bool &fault,bool &voltageEnabled,bool &quickStop,bool &warning, bool &targetReached, bool &limitReached, bool &bit12, bool &bit13) {
	try {
		CheckConnection();
		uint8_t opState;
		if (powerSM_->GetCurrentState(opState))
			return EXIT_FAILURE;
		auto enum_name = magic_enum::enum_name(static_cast<PowerSM::States>(opState));
		state = enum_name;
		uint16_t uWord16 = static_cast<uint16_t>(nanolibHelper_.readInteger(*connectedDeviceHandle_, nlc::OdIndex(0x6041, 0x00)));
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
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

//**AUTO-SETUP***

int Controller::AutoSetupMotPams() {
	try {
		CheckConnection();
		AutoSetupMotor mot(&nanolibHelper_, &connectedDeviceHandle_, &(*powerSM_));
		mot.AutoSetupMotPams();
	}
	catch (const nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

//***HOMING***

int Controller::Home(uint32_t speedZero, uint32_t speedSwitch) {
	try {
		CheckConnection();

		//enable reference switches, set inputs to 24V range
		if (ConfigureInputs())
			EXIT_FAILURE;

		HomingMotor mot(&nanolibHelper_, &connectedDeviceHandle_, &(*powerSM_));
		if (mot.home(speedZero, speedSwitch))
			return EXIT_FAILURE;

	}
	catch (nanolib_exception e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}


int Controller::SetHomingAcceleration(uint32_t acc) {
	try {
		CheckConnection();
		HomingMotor mot(&nanolibHelper_, &connectedDeviceHandle_, &(*powerSM_));
		mot.setHomingAcceleration(acc);
	}
	catch (const nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

//***POSITIONING***


int Controller::SetTargetPosition(int32_t value, uint32_t absRel) {
	try {
		CheckConnection();
		ProfilePositionMotor mot(&nanolibHelper_, &connectedDeviceHandle_, &(*powerSM_));
		mot.setTargetPosition(value,absRel);
	}
	catch (const nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::SetProfileAcceleration(uint32_t acc) {
	try {
		CheckConnection();
		ProfilePositionMotor mot(&nanolibHelper_, &connectedDeviceHandle_, &(*powerSM_));
		mot.setProfileAcceleration(acc);
	}
	catch (const nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::SetProfileVelocity(uint32_t speed) {
	try {
		CheckConnection();
		ProfilePositionMotor mot(&nanolibHelper_, &connectedDeviceHandle_, &(*powerSM_));
		mot.setProfileVelocity(speed);
	}
	catch (const nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::GetPositioningParameters(uint32_t& profileVelocity, int32_t& targetPosition) {
	try {
		CheckConnection();
		ProfilePositionMotor mot(&nanolibHelper_, &connectedDeviceHandle_, &(*powerSM_));
		mot.getPositioningParameters(profileVelocity, targetPosition);
	}
	catch (const nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::StartPositioning() {
	try {
		CheckConnection();
		ProfilePositionMotor mot(&nanolibHelper_, &connectedDeviceHandle_, &(*powerSM_));
		mot.startPositioning();
	}
	catch (const nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::GetUserUnitsPositioning(uint32_t& unit, uint32_t& exp) {
	try {
		CheckConnection();
		ProfilePositionMotor mot(&nanolibHelper_, &connectedDeviceHandle_, &(*powerSM_));
		mot.getUserUnitsPositioning(unit,exp);
	}
	catch (const nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::SetUserUnitsPositioning(uint32_t posUnit, uint32_t posExp) {
	try {
		CheckConnection();
		ProfilePositionMotor mot(&nanolibHelper_, &connectedDeviceHandle_, &(*powerSM_));
		mot.setUserUnitsPositioning(posUnit, posExp);
	}
	catch (const nanolib_exception& e) {
		DBOUT("exception: " << e.what());
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}


//***VELOCITY***

int Controller::SetTargetVelocity(int16_t vel) {
	try {
		CheckConnection();
		VelocityMotor mot(&nanolibHelper_, &connectedDeviceHandle_, &(*powerSM_));
		mot.SetTargetVelocity(vel);
	}
	catch (const nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::StartVelocity() {
	try {
		CheckConnection();
		VelocityMotor mot(&nanolibHelper_, &connectedDeviceHandle_, &(*powerSM_));
		mot.StartVelocity();
	}
	catch (const nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
int Controller::SetVelocityAcceleration(uint32_t deltaSpeed, uint16_t deltaTime) {
try {
	CheckConnection();
	VelocityMotor mot(&nanolibHelper_, &connectedDeviceHandle_, &(*powerSM_));
	mot.SetVelocityAcceleration(deltaSpeed,deltaTime );
}
catch (const nanolib_exception& e) {
	exceptions_.push_back(e);
	return EXIT_FAILURE;
}
return EXIT_SUCCESS;
}

int Controller::GetVelocityDemanded(int16_t &velDemanded) {
	try {
		CheckConnection();
		Motor402 mot(&nanolibHelper_, &connectedDeviceHandle_, &(*powerSM_));
		mot.GetVelocityDemanded(velDemanded);
	}
	catch (const nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::GetVelocityActual(int16_t& velActual) {
	try {
		CheckConnection();
		Motor402 mot(&nanolibHelper_, &connectedDeviceHandle_, &(*powerSM_));
		mot.GetVelocityActual(velActual);
	}
	catch (const nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::SetVelocityDeceleration(uint32_t deltaSpeed, uint16_t deltaTime) {
	try {
		CheckConnection();
		VelocityMotor mot(&nanolibHelper_, &connectedDeviceHandle_, &(*powerSM_));
		mot.SetVelocityDeceleration(deltaSpeed, deltaTime);
	}
	catch (const nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int Controller::GetTargetVelocity(int16_t& vel) {
	try {
		CheckConnection();
		VelocityMotor mot(&nanolibHelper_, &connectedDeviceHandle_, &(*powerSM_));
		mot.GetTargetVelocity(vel);
	}
	catch (const nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
int Controller::GetVelocityAcceleration(uint32_t& deltaSpeed, uint16_t& deltaTime){
	try {
		CheckConnection();
		VelocityMotor mot(&nanolibHelper_, &connectedDeviceHandle_, &(*powerSM_));
		mot.GetVelocityAcceleration(deltaSpeed,deltaTime);
	}
	catch (const nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
int Controller::GetVelocityDeceleration(uint32_t& deltaSpeed, uint16_t& deltaTime){
	try {
		CheckConnection();
		VelocityMotor mot(&nanolibHelper_, &connectedDeviceHandle_, &(*powerSM_));
		mot.GetVelocityDeceleration(deltaSpeed,deltaTime);
	}
	catch (const nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}


int Controller::SetUserUnitsVelocity(uint32_t velUnit, uint32_t velExp, uint32_t velTime) {
	try {
		CheckConnection();
		VelocityMotor mot(&nanolibHelper_, &connectedDeviceHandle_, &(*powerSM_));
		mot.SetUserUnitsVelocity(velUnit, velExp,velTime);
	}
	catch (const nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}



int Controller::GetUserUnitsVelocity(uint32_t &unit, uint32_t& exp, uint32_t& time) {
	try {
		CheckConnection();
		VelocityMotor mot(&nanolibHelper_, &connectedDeviceHandle_, &(*powerSM_));
		mot.GetUserUnitsVelocity(unit,exp, time);
	}
	catch (const nanolib_exception& e) {
		exceptions_.push_back(e);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}











