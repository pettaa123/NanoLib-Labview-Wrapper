#include <chrono>
#include <thread>
#include "motor.h"

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


Motor402::Motor402(NanoLibHelper* nanolibHelper, std::optional<nlc::DeviceHandle>* connectedDeviceHandle, PowerSM* powerSM) :
	nanolibHelper_(nanolibHelper),
	connectedDeviceHandle_(connectedDeviceHandle),
	powerSM_(powerSM)
{
}


int Motor402::SetModeOfOperation(int8_t mode) {
	// get current mode of operat
	if (GetModeOfOperation() == mode)
		return EXIT_SUCCESS;
	//necessary? what happens when chaning operation mode to power sm
	if (powerSM_->DisableOperation())
		return EXIT_FAILURE;

	nanolibHelper_->writeInteger(connectedDeviceHandle_->value(), mode, nlc::OdIndex(0x6060, 0x00), 8);

	return EXIT_SUCCESS;
}

void Motor402::GetMotorParameters(uint32_t& polePairCount, uint32_t& ratedCurrent, uint32_t& maxCurrent, uint32_t& maxCurrentDuration, uint32_t& idleCurrent, DriveMode& driveMode) {
	//get polpaarzahl
	polePairCount = static_cast<uint32_t>(nanolibHelper_->readInteger(connectedDeviceHandle_->value(), nlc::OdIndex(0x2030, 0x00)));
	// get motorstrom  maximal zulässigen Motorstrom (Motorschutz) in mA
	maxCurrent = static_cast<uint32_t>(nanolibHelper_->readInteger(connectedDeviceHandle_->value(), nlc::OdIndex(0x2031, 0x00)));
	//  Nennstrom des Motors in mA (siehe Motordatenblatt) e
	ratedCurrent = static_cast<uint32_t>(nanolibHelper_->readInteger(connectedDeviceHandle_->value(), nlc::OdIndex(0x203B, 0x01)));
	//max duration of max current in ms
	maxCurrentDuration = static_cast<uint32_t>(nanolibHelper_->readInteger(connectedDeviceHandle_->value(), nlc::OdIndex(0x203B, 0x02)));
	//idle current in mA
	idleCurrent = static_cast<uint32_t>(nanolibHelper_->readInteger(connectedDeviceHandle_->value(), nlc::OdIndex(0x2037, 0x00)));

	uint32_t uWord32 = static_cast<uint32_t>(nanolibHelper_->readInteger(connectedDeviceHandle_->value(), nlc::OdIndex(0x3202, 0x00)));
	if (!(uWord32 & (1U << 0))) {//open loop
		if (uWord32 & (1U << 3))
			driveMode = DriveMode::STEPPER_OPEN_LOOP_W_CURR_REDUCTION;
		else
			driveMode = DriveMode::STEPPER_OPEN_LOOP_WO_CURR_REDUCTION;
	}
	else if (uWord32 & (1U << 6))
		driveMode = DriveMode::BLDC;
	else
		driveMode = DriveMode::STEPPER_CLOSED_LOOP;
	return;
}

int8_t Motor402::GetModeOfOperation() {
	// get current mode of operation
	return static_cast<int8_t>(nanolibHelper_->readInteger(connectedDeviceHandle_->value(), nlc::OdIndex(0x6061, 0x00)));
}

int Motor402::SetMotorParameters(uint32_t polePairCount, uint32_t ratedCurrent, uint32_t maxCurrent, uint32_t maxCurrentDuration, uint32_t idleCurrent, DriveMode driveMode) {

	if (powerSM_->DisableOperation())
		return EXIT_FAILURE;

	//write polpaarzahl
	nanolibHelper_->writeInteger(connectedDeviceHandle_->value(), polePairCount, nlc::OdIndex(0x2030, 0x00), 32);
	// write motorstrom  maximal zulässigen Motorstrom (Motorschutz) in mA
	nanolibHelper_->writeInteger(connectedDeviceHandle_->value(), maxCurrent, nlc::OdIndex(0x2031, 0x00), 32);
	//  Nennstrom des Motors in mA (siehe Motordatenblatt) e
	nanolibHelper_->writeInteger(connectedDeviceHandle_->value(), ratedCurrent, nlc::OdIndex(0x203B, 0x01), 32);
	//max duration of max current in ms
	nanolibHelper_->writeInteger(connectedDeviceHandle_->value(), maxCurrentDuration, nlc::OdIndex(0x203B, 0x02), 32);
	//open loop idle current
	nanolibHelper_->writeInteger(connectedDeviceHandle_->value(), idleCurrent, nlc::OdIndex(0x2037, 0x00), 32);
	//motor type
	uint32_t uWord32 = static_cast<uint32_t>(nanolibHelper_->readInteger(connectedDeviceHandle_->value(), nlc::OdIndex(0x3202, 0x00)));

	switch (driveMode) {
	case DriveMode::BLDC:
		uWord32 |= driveMode;
		break;
	case  DriveMode::STEPPER_CLOSED_LOOP:
		uWord32 &= ~(1UL << 6); //reset bldc
		uWord32 |= (1UL << 0); //set closed loop
		break;
	case DriveMode::STEPPER_OPEN_LOOP_WO_CURR_REDUCTION:
		uWord32 &= ~(1UL << 6); //reset bldc
		uWord32 &= ~(1UL << 0); //reset closed loop
		uWord32 &= ~(1UL << 3); //reset current reduction
		break;
	case DriveMode::STEPPER_OPEN_LOOP_W_CURR_REDUCTION:
		uWord32 &= ~(1UL << 6); //reset bldc
		uWord32 &= ~(1UL << 0); //reset closed loop
		uWord32 |= (1UL << 3); //set current reduction
		break;
	default:
		throw(nanolib_exception("unknown drive mode"));
		break;
	}
	nanolibHelper_->writeInteger(connectedDeviceHandle_->value(), uWord32, nlc::OdIndex(0x3202, 0x00), 32);

	return EXIT_SUCCESS;

}

int Motor402::SaveGroup(uint8_t group) {
	if (powerSM_->DisableOperation())
		return EXIT_FAILURE;

	nanolibHelper_->writeInteger(connectedDeviceHandle_->value(), 1702257011, nlc::OdIndex(0x1010, group), 32);

	uint16_t iterationsDone = 0;
	uint16_t maxIterations = 300;

	uint32_t uWord32;

	using namespace std::chrono_literals; // ns, us, ms, s, h, etc.

	do {
		uWord32 = static_cast<uint32_t>(
			nanolibHelper_->readInteger(connectedDeviceHandle_->value(), nlc::OdIndex(0x1010, group)));
		DBOUT("Save running\n");
		std::this_thread::sleep_for(100ms);
		iterationsDone += 1;
		if (iterationsDone == maxIterations) {
			throw(nanolib_exception("Saving timeout"));
			return EXIT_FAILURE;
		}
	} while (uWord32 != 1);
	DBOUT("Saving done\n");

	return EXIT_SUCCESS;
}


int Motor402::SetUserUnitsFeed(uint32_t feedPer, uint32_t shaftRevolutions) {
	//make sure operation is disabled before changing user defined units
	if (powerSM_->DisableOperation()) {
		throw(nanolib_exception("Couldn't disable operation"));
		return EXIT_FAILURE;
	}

	nanolibHelper_->writeInteger(connectedDeviceHandle_->value(), feedPer, nlc::OdIndex(0x6092, 0x01), 32);
	nanolibHelper_->writeInteger(connectedDeviceHandle_->value(), shaftRevolutions, nlc::OdIndex(0x6092, 0x02), 32);
	return EXIT_SUCCESS;
}

int32_t Motor402::GetSpeedActual() {
	return static_cast<int32_t>(nanolibHelper_->readInteger(connectedDeviceHandle_->value(), nlc::OdIndex(0x606C, 0x00)));
}

int32_t Motor402::GetPositionActual() {
	return static_cast<int32_t>(nanolibHelper_->readInteger(connectedDeviceHandle_->value(), nlc::OdIndex(0x6064, 0x00)));
}

void Motor402::SetMaxMotorSpeed(uint32_t maxSpeed) {
	nanolibHelper_->writeInteger(connectedDeviceHandle_->value(), maxSpeed, nlc::OdIndex(0x6080, 0x00), 32);
}

int Motor402::Halt() {
	auto mode=GetModeOfOperation();
	if (
		(mode != OperationMode::ProfilePosition) && (mode != OperationMode::Velocity) && (mode != OperationMode::ProfileVelocity) && (mode != OperationMode::ProfileTorque) && (mode != OperationMode::InterpolatedPosition)) {
		throw(nanolib_exception("Can't halt this mode"));
		return EXIT_FAILURE;
	}
	//halt option: slow down ramp
	nanolibHelper_->writeInteger(connectedDeviceHandle_->value(), 1, nlc::OdIndex(0x605D, 0x00), 16);

	int16_t word16 = static_cast<int16_t>(nanolibHelper_->readInteger(connectedDeviceHandle_->value(), nlc::OdIndex(0x605D, 0x00)));
	word16 |= (1 << 8);
	nanolibHelper_->writeInteger(connectedDeviceHandle_->value(), word16, nlc::OdIndex(0x6040, 0x00), 16);
	return EXIT_SUCCESS;
}



