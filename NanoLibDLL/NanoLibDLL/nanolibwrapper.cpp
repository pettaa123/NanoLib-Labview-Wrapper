#include "nanolibwrapper.h"
#include "controller.h"
#include <iostream>

namespace NanoLibWrapper {

	//***GENERAL***

	int32_t VecStrToLVStrArr(const std::vector<std::string>& s, LStrArrayHdl* arr) {
		int32_t err = 0;
		/* If the incoming array is bigger than what we need, we first need to loop through
		the superfluous items to see if they contain valid string handles and if so we
		need to deallocate them to avoid memory leaks */
		if (arr)
		{
			for (int i = static_cast<int>(std::size(s)); i < (**arr)->dimSize; i++)
			{
				LStrHandle* elt = &((**arr)->elt[i]);
				if (*elt)
				{
					err = DSDisposeHandle(*elt);
					*elt = NULL;
				}
			}
		}

		/* Now we need to make sure the StringArrayHandle is large enough to hold all ports */
		err = NumericArrayResize(uPtr, 1, (UHandle*)arr, std::size(s));
		for (int i = 0; !err && (i < std::size(s)); i++)
		{
			LStrHandle* elt = &((**arr)->elt[i]);
			// Resize string handle to be large enough to store the string data
			err = NumericArrayResize(uB, 1, (UHandle*)elt, s[i].length());
			if (!err)
			{
				MoveBlock(s[i].c_str(), LStrBuf(**elt), s[i].length());
				LStrLen(**elt) = static_cast<int32_t>(s[i].length());
			}
		}
		if (err)
			return err;

		(**arr)->dimSize = static_cast<int32_t>(std::size(s));

		return err;
	}

	int32_t VecUint32ToLVuint32Arr(const std::vector<uint32_t>& s, LVuint32ArrayHdl* arr) {
		int32_t err;
		int32_t size = static_cast<int32_t>(s.size());

		// The first 32 bits are used for size information, and then the array comes after the size
		err = DSSetHandleSize(*arr, sizeof(int32_t) + s.size() * sizeof(uint32_t));
		if (err != mFullErr && err != mZoneErr)
		{
			(**arr)->dimSize = size;

			for (int32_t i = 0; i < size; i++)
			{
				(**arr)->elt[i] = s.at(i);
			}
			return EXIT_SUCCESS;
		}
		return err;
	}



	int32_t StdStrToLVStr(const std::string& s, LStrHandle* str) {
		int32_t err = 0;
		// Resize string handle to be large enough to store the string data
		err = NumericArrayResize(uB, 1, (UHandle*)str, s.length());
		if (!err)
		{
			MoveBlock(s.c_str(), LStrBuf(**str), s.length());
			LStrLen(**str) = static_cast<int>(s.length());
			return EXIT_SUCCESS;
		}
		return err;
	}

	int32_t RebootDevice() {
		Controller* c = Controller::GetInstance();
		return c->RebootDevice();
	}

	int32_t GetExceptions(std::vector<std::string>& exceptions) {
		Controller* c = Controller::GetInstance();
		if (c->GetExceptions(exceptions))
			return EXIT_FAILURE;
	}

	int32_t GetExceptionsLV(LStrArrayHdl* LVAllocatedStrArray) {
		std::vector<std::string> exceptions;
		if (GetExceptions(exceptions))
			return EXIT_FAILURE;
		return VecStrToLVStrArr(exceptions, LVAllocatedStrArray);
	}


	int32_t GetPorts(std::vector<std::string>& ports) {
		Controller* c = Controller::GetInstance();
		return c->GetAvailablePorts(ports);
	}

	int32_t GetPortsLV(LStrArrayHdl* arr) {

		int32_t err = 0;
		std::vector<std::string> ports;

		if (GetPorts(ports))
			return EXIT_FAILURE;
		err = VecStrToLVStrArr(ports, arr);

		return err;
	}


	int32_t GetModeOfOperationLV(LStrHandle* LVAllocatedString) {
		int32_t err;
		Controller* c = Controller::GetInstance();
		std::string mode;
		if (c->GetModeOfOperation(mode))
			return EXIT_FAILURE;
		err = StdStrToLVStr(mode, LVAllocatedString);
		return EXIT_SUCCESS;
	}

	int32_t GetCiA402StateLV(LStrHandle* state, LVBoolean* fault, LVBoolean* voltageEnabled, LVBoolean* quickStop, LVBoolean* warning, LVBoolean* targetReached, LVBoolean* limitReached, LVBoolean* bit12, LVBoolean* bit13) {
		Controller* c = Controller::GetInstance();
		std::string state_;
		bool fault_, voltageEnabled_, quickStop_, warning_, targetReached_, limitReached_, bit12_, bit13_;
		if (c->GetCiA402State(state_, fault_, voltageEnabled_, quickStop_, warning_, targetReached_, limitReached_, bit12_, bit13_))
			return EXIT_FAILURE;
		if (StdStrToLVStr(state_, state))
			return EXIT_FAILURE;
		*fault = static_cast<LVBoolean>(fault_);
		*voltageEnabled = static_cast<LVBoolean>(voltageEnabled_);
		*quickStop = static_cast<LVBoolean>(quickStop_);
		*warning = static_cast<LVBoolean>(warning_);
		*targetReached = static_cast<LVBoolean>(targetReached_);
		*limitReached = static_cast<LVBoolean>(limitReached_);
		*bit12 = static_cast<LVBoolean>(bit12_);
		*bit13 = static_cast<LVBoolean>(bit13_);

		return EXIT_SUCCESS;
	}

	int32_t GetUserUnits(uint32_t& feed, uint32_t& shaftRevs, uint32_t& posUnit, uint32_t& posExp, uint32_t& velUnit, uint32_t& velExp, uint32_t& velTime, uint32_t& gearRatioMotorRevs, uint32_t& gearRatioShaftRevs) {
		Controller* c = Controller::GetInstance();
		if (c->GetUserUnitsFeed(feed, shaftRevs))
			return EXIT_FAILURE;

		if (c->GetUserUnitsPositioning(posUnit, posExp))
			return EXIT_FAILURE;

		if (c->GetUserUnitsVelocity(velUnit, velExp, velTime))
			return EXIT_FAILURE;

		if (c->GetUserUnitsGearRatio(gearRatioMotorRevs, gearRatioShaftRevs))
			return EXIT_FAILURE;

		return EXIT_SUCCESS;;
	}

	int32_t SetUserUnits(uint32_t feed, uint32_t shaftRevs, uint32_t posUnit, uint32_t posExp, uint32_t velUnit, uint32_t velExp, uint32_t velTime, uint32_t gearRatioMotorRevs, uint32_t gearRatioShaftRevs) {
		Controller* c = Controller::GetInstance();
		if (c->SetUserUnitsFeed(feed, shaftRevs))
			return EXIT_FAILURE;

		if (c->SetUserUnitsPositioning(posUnit, posExp))
			return EXIT_FAILURE;

		if (c->SetUserUnitsVelocity(velUnit, velExp, velTime))
			return EXIT_FAILURE;

		return EXIT_SUCCESS;
	}

	int32_t OpenPort(uint32_t portToOpen) {
		Controller* c = Controller::GetInstance();
		return c->OpenPort(portToOpen);
	}

	int32_t Halt() {
		Controller* c = Controller::GetInstance();
		return c->Halt();
	}

	int32_t QuickStop() {
		Controller* c = Controller::GetInstance();
		return c->QuickStop();
	}

	int32_t GetErrorStackLV(LStrArrayHdl* LVAllocatedStrArray) {
		Controller* c = Controller::GetInstance();
		std::vector<std::string> errorStack;
		if (c->GetDeviceErrorStack(errorStack))
			return EXIT_FAILURE;

		return VecStrToLVStrArr(errorStack, LVAllocatedStrArray);
	}

	int32_t ClosePort() {
		Controller* c = Controller::GetInstance();
		return c->ClosePort();
	}
	int32_t AutoSetupMotPams() {
		Controller* c = Controller::GetInstance();
		return c->AutoSetupMotPams();
	}

	int32_t ScanBus(std::vector<std::string>& devices) {
		Controller* c = Controller::GetInstance();
		return c->ScanBus(devices);
	}

	int32_t ScanBusLV(LStrArrayHdl* LVAllocatedStrArray) {
		std::vector<std::string> devices;
		if (ScanBus(devices))
			return EXIT_FAILURE;

		return VecStrToLVStrArr(devices, LVAllocatedStrArray);
	}

	int32_t DisconnectDevice() {
		Controller* c = Controller::GetInstance();
		return c->DisconnectDevice();
	}

	int32_t ConnectDevice(uint32_t deviceToOpen) {
		Controller* c = Controller::GetInstance();
		return c->ConnectDevice(deviceToOpen);
	}

	int32_t SetMotorParameters(uint32_t polePairCount, uint32_t ratedCurrent, uint32_t maxCurrent, uint32_t maxCurrentDuration, uint32_t idleCurrent, uint32_t driveMode) {
		Controller* c = Controller::GetInstance();
		return c->SetMotorParameters(polePairCount, ratedCurrent, maxCurrent, maxCurrentDuration, idleCurrent, driveMode);
	}

	int32_t GetMotorParameters(uint32_t& polePairCount, uint32_t& ratedCurrent, uint32_t& maxCurrent, uint32_t& maxCurrentDuration, uint32_t& idleCurrent, uint32_t& driveMode) {
		Controller* c = Controller::GetInstance();
		return c->GetMotorParameters(polePairCount, ratedCurrent, maxCurrent, maxCurrentDuration, idleCurrent, driveMode);
	}

	int32_t GetPositioningParameters(uint32_t& profileVelocity, int32_t& setTarget) {
		Controller* c = Controller::GetInstance();
		return c->GetPositioningParameters(profileVelocity, setTarget);
	}

	int32_t SaveMotorParameters() {
		Controller* c = Controller::GetInstance();
		if (c->SaveGroupTuning())
			return EXIT_FAILURE;
		return c->SaveGroupMovement();
	}

	int32_t SaveUserUnits() {
		Controller* c = Controller::GetInstance();
		return c->SaveGroupApplication();
	}

	int32_t GetPositionActual(int32_t& pos) {
		Controller* c = Controller::GetInstance();
		return c->GetPositionActual(pos);
	}


	int32_t GetFirmwareVersion(std::string& ver) {
		Controller* c = Controller::GetInstance();
		return c->GetDeviceFirmwareBuildId(ver);
	}

	int32_t GetFirmwareVersionLV(LStrHandle* LVAllocatedStr) {
		Controller* c = Controller::GetInstance();
		std::string ver;
		if (c->GetDeviceFirmwareBuildId(ver))
			return EXIT_FAILURE;
		return StdStrToLVStr(ver, LVAllocatedStr);
	}



	//***POSITIONING***

	int32_t SetTargetPosition(int32_t val, uint32_t absRel) {
		Controller* c = Controller::GetInstance();
		return c->SetTargetPosition(val, absRel);
	}

	int32_t StartPositioning() {
		Controller* c = Controller::GetInstance();
		return c->StartPositioning();
	}

	//profile positioning velocity
	int32_t SetProfileVelocity(uint32_t speed) {
		Controller* c = Controller::GetInstance();
		return c->SetProfileVelocity(speed);
	}

	int32_t SetProfileAcceleration(uint32_t acc) {
		Controller* c = Controller::GetInstance();
		return c->SetProfileAcceleration(acc);
	}


	//***VELOCITY***

	int32_t StartVelocity() {
		Controller* c = Controller::GetInstance();
		return c->StartVelocity();
	}

	int32_t SetVelocityPams(int16_t vel, uint32_t deltaSpeedAcc, uint16_t deltaTimeAcc, uint32_t deltaSpeedDec, uint16_t deltaTimeDec) {
		Controller* c = Controller::GetInstance();
		if (c->SetTargetVelocity(vel))
			return EXIT_FAILURE;
		if (c->SetVelocityAcceleration(deltaSpeedAcc, deltaTimeAcc))
			return EXIT_FAILURE;
		return (c->SetVelocityDeceleration(deltaSpeedDec, deltaTimeDec));
	}

	int32_t GetVelocityPams(int16_t& vel, uint32_t& deltaSpeedAcc, uint16_t& deltaTimeAcc, uint32_t& deltaSpeedDec, uint16_t& deltaTimeDec) {
		Controller* c = Controller::GetInstance();
		if (c->GetTargetVelocity(vel))
			return EXIT_FAILURE;
		if (c->GetVelocityAcceleration(deltaSpeedAcc, deltaTimeAcc))
			return EXIT_FAILURE;
		return (c->GetVelocityDeceleration(deltaSpeedDec, deltaTimeDec));
	}

	int32_t GetVelocityStatus(int16_t& demandedSpeed, int16_t& speedActual) {
		Controller* c = Controller::GetInstance();
		if (c->GetVelocityDemanded(demandedSpeed))
			return EXIT_FAILURE;
		return (c->GetVelocityDemanded(speedActual));
	}

	//**HOMING***

	int32_t SetHomingAcceleration(uint32_t acc) {
		Controller* c = Controller::GetInstance();
		return c->SetHomingAcceleration(acc);
	}

	int32_t Home(uint32_t speedZero, uint32_t speedSwitch) {
		Controller* c = Controller::GetInstance();
		return c->Home(speedZero, speedSwitch);
	}

}