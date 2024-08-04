#pragma once

#include <optional>
#include <vector>

#include "nanolib_helper.hpp"

#include "auto_setup_motor.h"
#include "homing_motor.h"
#include "profile_position_motor.h"
#include "velocity_motor.h"


class Controller {

public:

	// deleting copy constructor
	Controller(const Controller&) = delete;
	void operator=(const Controller &) = delete;

	~Controller();

	//***GENERAL***
	int QuickStop();
	int Halt();
	int RebootDevice();
	int GetDeviceFirmwareBuildId(std::string& ver);
	int GetAvailablePorts(std::vector<std::string>& ports);
	int ClosePort();
	int GetDeviceErrorStack(std::vector<std::string>& errorStack);


	//exceptions thrown from nanolib
	int GetExceptions(std::vector<std::string>& exceptions);

	int OpenPort(uint32_t portToOpen);
	int ConnectDevice(uint32_t deviceToOpen);
	int DisconnectDevice();
	int ScanBus(std::vector<std::string>& devices);

	int ConfigureInputs();

	int GetModeOfOperation(std::string& mode);

	int GetCiA402State(std::string& state, bool& fault, bool& voltageEnabled, bool& quickStop, bool& warning, bool& targetReached, bool& limitReached, bool& bit12, bool& bit13);

	//motor specific
	int SetMotorParameters(uint32_t polePairCount, uint32_t ratedCurrent, uint32_t maxCurrent, uint32_t maxCurrentDuration,uint32_t openLoopIdleCurrent, uint32_t driveMode);
	int GetMotorParameters(uint32_t &polePairCount, uint32_t &ratedCurrent, uint32_t &maxCurrent, uint32_t &maxCurrentDuration, uint32_t& openLoopIdleCurrent, uint32_t &driveMode);
	int SaveGroupMovement();
	int SaveGroupApplication();
	int SaveGroupTuning();
	int AutoSetupMotPams();

	//***HOMING***
	int Home(uint32_t speedZeroUserUnit = 10, uint32_t speedSwitchUserUnit = 50);

	//***POSITIONING***
	int StartPositioning();
	int SetTargetPosition(int32_t val, uint32_t absRel);
	//The current actual position in user - defined units.
	int GetPositionActual(int32_t& val);
	int SetProfileVelocity(uint32_t speed);
	int GetPositioningParameters(uint32_t &profileVelocity, int32_t &targetPosition);

	//***VELOCITY***
	int StartVelocity();
	int SetTargetVelocity(int16_t vel);
	int SetVelocityAcceleration(uint32_t deltaSpeed, uint16_t deltaTime);
	int SetVelocityDeceleration(uint32_t deltaSpeed, uint16_t deltaTime);
	int GetTargetVelocity(int16_t &vel);
	int GetVelocityAcceleration(uint32_t &deltaSpeed, uint16_t &deltaTime);
	int GetVelocityDeceleration(uint32_t &deltaSpeed, uint16_t &deltaTime);
	int GetVelocityDemanded(int16_t &demandedSpeed);
	int GetVelocityActual(int16_t &velActual);

	//getter user defined units
	int GetUserUnitsPositioning(uint32_t &unit, uint32_t &exp);
	int GetUserUnitsVelocity(uint32_t& unit, uint32_t &exp, uint32_t &time );
	int GetUserUnitsFeed(uint32_t& feed, uint32_t& shaftsRevolutions);
	int GetUserUnitsGearRatio(uint32_t& gearRatioMotorRevs, uint32_t& gearRatioShaftRevs);

	//spindelsteigung 
	int SetUserUnitsFeed(uint32_t feed, uint32_t shaftsRevolutions);
	int SetProfileAcceleration(uint32_t acc);
	int SetHomingAcceleration(uint32_t acc);

	int SetUserUnitsPositioning(uint32_t unit, uint32_t exp);
	int SetUserUnitsVelocity(uint32_t velUnit, uint32_t exp, uint32_t time);

	static Controller* GetInstance()
	{
		static Controller instance;	
		return &instance;
	}


private:

	Controller();

	static Controller* instancePtr_;

	std::unique_ptr<PowerSM> powerSM_;

	NanoLibHelper nanolibHelper_;
	std::optional<nlc::BusHardwareId> openedBusHardware_;
	std::optional<nlc::DeviceHandle> connectedDeviceHandle_;

	std::vector<nanolib_exception> exceptions_;

	int CheckConnection();

	int ReadDigitalInputs(uint8_t& states);

};



