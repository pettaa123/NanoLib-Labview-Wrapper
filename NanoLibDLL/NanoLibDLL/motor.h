#pragma once

#include <cstdint>

#include "nanolib_helper.hpp"
#include "powerSM.h"
#include "userUnits.h"

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
		std::cout << "destructing motor";
	};

	//works for modes: ProfilePosition,Velocity,Profile Velocity,Profile Torque,Interpolated Mode
	virtual int halt();

	//actual velocity in user defined units
	int32_t getSpeedActual();
	//actual position in user defined units
	int32_t getPositionActual();
	void setMaxMotorSpeed(uint32_t maxSpeed);

	//pams in mA and ms
	int setMotorParameters(uint32_t polePairCount, uint32_t ratedCurrent, uint32_t maxCurrent, uint32_t maxCurrentDuration, uint32_t idleCurrent, DriveMode driveMode);
	void getMotorParameters(uint32_t& polePairCount, uint32_t& ratedCurrent, uint32_t& maxCurrent, uint32_t& maxCurrentDuration, uint32_t& idleCurrent, DriveMode& driveMode);
	int saveGroup(uint8_t group);
	int setModeOfOperation(int8_t mode);
	int8_t getModeOfOperation();

	//general user units
	int setUserUnitsFeed(uint32_t feedPer, uint32_t shaftRevolutions);

	inline void getVelocityDemanded(int16_t& demandedVel) {demandedVel = static_cast<int16_t>(m_nanolibHelper->readInteger(m_connectedDeviceHandle->value(), nlc::OdIndex(0x6043, 0x00)));}
	inline void getVelocityActual(int16_t& velActual) {velActual = static_cast<int16_t>(m_nanolibHelper->readInteger(m_connectedDeviceHandle->value(), nlc::OdIndex(0x6044, 0x00)));}

protected:

	NanoLibHelper *m_nanolibHelper;
	std::optional<nlc::DeviceHandle> *m_connectedDeviceHandle;

	PowerSM *m_powerSM;



private:

	virtual uint16_t getState() {
		return 0;
	};

};

