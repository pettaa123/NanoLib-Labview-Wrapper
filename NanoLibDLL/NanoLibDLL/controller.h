#pragma once

#include <optional>
#include <vector>

#include "nanolib_helper.hpp"

#include "autoSetupMotor.h"
#include "homingMotor.h"
#include "profilePositionMotor.h"
#include "velocityMotor.h"


class Controller {

public:

	// deleting copy constructor
	Controller(const Controller&) = delete;
	void operator=(const Controller &) = delete;

	~Controller();

	//***GENERAL***
	int quickStop();
	int halt();
	int rebootDevice();
	int getDeviceFirmwareBuildId(std::string& ver);
	int getAvailablePorts(std::vector<std::string>& ports);
	int closePort();
	int getDeviceErrorStack(std::vector<std::string>& errorStack);


	//exceptions thrown from nanolib
	int getExceptions(std::vector<std::string>& exceptions);

	int openPort(uint32_t portToOpen);
	int connectDevice(uint32_t deviceToOpen);
	int disconnectDevice();
	int scanBus(std::vector<std::string>& devices);

	int configureInputs();

	int getModeOfOperation(std::string& mode);

	int getCiA402State(std::string& state, bool& fault, bool& voltageEnabled, bool& quickStop, bool& warning, bool& targetReached, bool& limitReached, bool& bit12, bool& bit13);

	//motor specific
	int setMotorParameters(uint32_t polePairCount, uint32_t ratedCurrent, uint32_t maxCurrent, uint32_t maxCurrentDuration,uint32_t openLoopIdleCurrent, uint32_t driveMode);
	int getMotorParameters(uint32_t &polePairCount, uint32_t &ratedCurrent, uint32_t &maxCurrent, uint32_t &maxCurrentDuration, uint32_t& openLoopIdleCurrent, uint32_t &driveMode);
	int saveGroupMovement();
	int saveGroupApplication();
	int saveGroupTuning();
	int autoSetupMotPams();

	//***HOMING***
	int home(uint32_t speedZeroUserUnit = 10, uint32_t speedSwitchUserUnit = 50);

	//***POSITIONING***
	int startPositioning();
	int setTargetPosition(int32_t val, uint32_t absRel);
	//The current actual position in user - defined units.
	int getPositionActual(int32_t& val);
	int setProfileVelocity(uint32_t speed);
	int getPositioningParameters(uint32_t &profileVelocity, int32_t &targetPosition);

	//***VELOCITY***
	int startVelocity();
	int setTargetVelocity(int16_t vel);
	int setVelocityAcceleration(uint32_t deltaSpeed, uint16_t deltaTime);
	int setVelocityDeceleration(uint32_t deltaSpeed, uint16_t deltaTime);
	int getTargetVelocity(int16_t &vel);
	int getVelocityAcceleration(uint32_t &deltaSpeed, uint16_t &deltaTime);
	int getVelocityDeceleration(uint32_t &deltaSpeed, uint16_t &deltaTime);
	int getVelocityDemanded(int16_t &demandedSpeed);
	int getVelocityActual(int16_t &velActual);

	//getter user defined units
	int getUserUnitsPositioning(uint32_t &unit, uint32_t &exp);
	int getUserUnitsVelocity(uint32_t& unit, uint32_t &exp, uint32_t &time );
	int getUserUnitsFeed(uint32_t& feed, uint32_t& shaftsRevolutions);
	int getUserUnitsGearRatio(uint32_t& gearRatioMotorRevs, uint32_t& gearRatioShaftRevs);

	//spindelsteigung 
	int setUserUnitsFeed(uint32_t feed, uint32_t shaftsRevolutions);
	int setProfileAcceleration(uint32_t acc);
	int setHomingAcceleration(uint32_t acc);

	int setUserUnitsPositioning(uint32_t unit, uint32_t exp);
	int setUserUnitsVelocity(uint32_t velUnit, uint32_t exp, uint32_t time);

	static Controller* getInstance()
	{
		static Controller instance;	
		return &instance;
	}


private:

	Controller();

	static Controller* m_instancePtr;

	std::unique_ptr<PowerSM> m_powerSM;

	NanoLibHelper m_nanolibHelper;
	std::optional<nlc::BusHardwareId> m_openedBusHardware;
	std::optional<nlc::DeviceHandle> m_connectedDeviceHandle;

	std::vector<nanolib_exception> m_exceptions;

	int checkConnection();

	int readDigitalInputs(uint8_t& states);

};



