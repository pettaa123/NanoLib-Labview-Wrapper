#pragma once

#include <cstdint>

#include "nanolib_helper.hpp"
#include "power_sm.h"
#include "user_units.h"

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

//general motor class
class Motor402 {

public:

	//operation mode
	enum OperationMode : int8_t {
		AutoSetup = -2,
		ClockDirection = -1,
		No = 0,
		ProfilePosition = 1,
		Velocity = 2,
		ProfileVelocity = 3,
		ProfileTorque = 4,
		Reserved = 5,
		Homing = 6,
		InterpolatedPosition = 7,
		CyclicSynchronousPosition = 8,
		CyclicSynchronousVelocity = 9,
		CyclicSynchronousTorque = 10
	};

	enum DriveMode : uint32_t {
		STEPPER_OPEN_LOOP_WO_CURR_REDUCTION = 0,
		STEPPER_CLOSED_LOOP = 1,
		STEPPER_OPEN_LOOP_W_CURR_REDUCTION = 8,
		BLDC = 65
	};

	Motor402(NanoLibHelper *nanolibHelper, std::optional<nlc::DeviceHandle> *connectedDeviceHandle, PowerSM *powerSM);
	~Motor402() {
		DBOUT("destructing motor");
	};

	//works for modes: ProfilePosition,Velocity,Profile Velocity,Profile Torque,Interpolated Mode
	virtual int Halt();

	//actual velocity in user defined units
	int32_t GetSpeedActual();
	//actual position in user defined units
	int32_t GetPositionActual();
	void SetMaxMotorSpeed(uint32_t maxSpeed);

	//pams in mA and ms
	int SetMotorParameters(uint32_t polePairCount, uint32_t ratedCurrent, uint32_t maxCurrent, uint32_t maxCurrentDuration, uint32_t idleCurrent, DriveMode driveMode);
	void GetMotorParameters(uint32_t& polePairCount, uint32_t& ratedCurrent, uint32_t& maxCurrent, uint32_t& maxCurrentDuration, uint32_t& idleCurrent, DriveMode& driveMode);
	int SaveGroup(uint8_t group);
	int SetModeOfOperation(int8_t mode);
	int8_t GetModeOfOperation();

	//general user units
	int SetUserUnitsFeed(uint32_t feedPer, uint32_t shaftRevolutions);

	inline void GetVelocityDemanded(int16_t& demandedVel) {demandedVel = static_cast<int16_t>(nanolibHelper_->readInteger(connectedDeviceHandle_->value(), nlc::OdIndex(0x6043, 0x00)));}
	inline void GetVelocityActual(int16_t& velActual) {velActual = static_cast<int16_t>(nanolibHelper_->readInteger(connectedDeviceHandle_->value(), nlc::OdIndex(0x6044, 0x00)));}

protected:

	NanoLibHelper* nanolibHelper_;
	std::optional<nlc::DeviceHandle>* connectedDeviceHandle_;

	PowerSM* powerSM_;



private:

	virtual uint16_t getState() {
		return 0;
	};

};

