#include "nanolibwrapper.h"
#include "controller.h"
#include <iostream>

//***GENERAL***

int32_t vecStrToLVStrArr(const std::vector<std::string>& s, LStrArrayHdl* arr) {
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

int32_t vecUint32ToLVuint32Arr(const std::vector<uint32_t>& s, LVuint32ArrayHdl* arr) {
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



int32_t stdStrToLVStr(const std::string& s, LStrHandle* str) {
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

int32_t rebootDevice() {
	Controller* c = Controller::getInstance();
	return c->rebootDevice();
}

int32_t getExceptions(std::vector<std::string>& exceptions) {
	Controller* c = Controller::getInstance();
	if (c->getExceptions(exceptions))
		return EXIT_FAILURE;
}

int32_t getExceptionsLV(LStrArrayHdl* LVAllocatedStrArray) {
	std::vector<std::string> exceptions;
	if (getExceptions(exceptions))
		return EXIT_FAILURE;
	return vecStrToLVStrArr(exceptions, LVAllocatedStrArray);
}


int32_t getPortsLV(LStrArrayHdl* arr) {

	int32_t err = 0;
	std::vector<std::string> ports;

	if (getPorts(ports))
		return EXIT_FAILURE;
	err = vecStrToLVStrArr(ports, arr);

	return err;
}

int32_t scanBusLV(LStrArrayHdl* LVAllocatedStrArray) {
	Controller* c = Controller::getInstance();
	std::vector<std::string> devices;
	if (c->scanBus(devices))
		return EXIT_FAILURE;

	return vecStrToLVStrArr(devices, LVAllocatedStrArray);
}

int32_t getModeOfOperationLV(LStrHandle* LVAllocatedString) {
	int32_t err;
	Controller* c = Controller::getInstance();
	std::string mode;
	if (c->getModeOfOperation(mode))
		return EXIT_FAILURE;
	err = stdStrToLVStr(mode, LVAllocatedString);
	return EXIT_SUCCESS;
}

int32_t getCiA402StateLV(LStrHandle* state, LVBoolean* fault, LVBoolean* voltageEnabled, LVBoolean* quickStop, LVBoolean* warning, LVBoolean* targetReached, LVBoolean* limitReached, LVBoolean* bit12, LVBoolean* bit13) {
	Controller* c = Controller::getInstance();
	std::string state_;
	bool fault_, voltageEnabled_, quickStop_, warning_, targetReached_, limitReached_, bit12_, bit13_;
	if (c->getCiA402State(state_, fault_, voltageEnabled_, quickStop_, warning_, targetReached_, limitReached_, bit12_, bit13_))
		return EXIT_FAILURE;
	if (stdStrToLVStr(state_, state))
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


int32_t getPorts(std::vector<std::string>& ports) {
	Controller* c = Controller::getInstance();
	return c->getAvailablePorts(ports);
}

int32_t getUserUnits(uint32_t& feed, uint32_t& shaftRevs, uint32_t &posUnit, uint32_t &posExp, uint32_t &velUnit, uint32_t &velExp, uint32_t &velTime ,uint32_t& gearRatioMotorRevs, uint32_t& gearRatioShaftRevs) {
	Controller* c = Controller::getInstance();
	if (c->getUserUnitsFeed(feed,shaftRevs))
		return EXIT_FAILURE;

	if (c->getUserUnitsPositioning(posUnit, posExp))
		return EXIT_FAILURE;

	if (c->getUserUnitsVelocity(velUnit,velExp, velTime))
		return EXIT_FAILURE;

	if (c->getUserUnitsGearRatio(gearRatioMotorRevs, gearRatioShaftRevs))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;;
}

int32_t setUserUnits(uint32_t feed, uint32_t shaftRevs, uint32_t posUnit, uint32_t posExp, uint32_t velUnit, uint32_t velExp, uint32_t velTime, uint32_t gearRatioMotorRevs, uint32_t gearRatioShaftRevs) {
	Controller* c = Controller::getInstance();
	if (c->setUserUnitsFeed(feed, shaftRevs))
		return EXIT_FAILURE;

	if (c->setUserUnitsPositioning(posUnit, posExp))
		return EXIT_FAILURE;

	if (c->setUserUnitsVelocity(velUnit, velExp, velTime))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

int32_t openPort(uint32_t portToOpen) {
	Controller* c = Controller::getInstance();
	return c->openPort(portToOpen);
}

int32_t halt() {
	Controller* c = Controller::getInstance();
	return c->halt();
}

int32_t quickStop() {
	Controller* c = Controller::getInstance();
	return c->quickStop();
}

int32_t getErrorStackLV(LStrArrayHdl* LVAllocatedStrArray) {
	Controller* c = Controller::getInstance();
	std::vector<std::string> errorStack;
	if (c->getDeviceErrorStack(errorStack))
		return EXIT_FAILURE;

	return vecStrToLVStrArr(errorStack, LVAllocatedStrArray);
}

int32_t closePort() {
	Controller* c = Controller::getInstance();
	return c->closePort();
}
int32_t autoSetupMotPams() {
	Controller* c = Controller::getInstance();
	return c->autoSetupMotPams();
}

int32_t scanBus(std::vector<std::string>& devices) {
	Controller* c = Controller::getInstance();
	return c->scanBus(devices);
}

int32_t disconnectDevice() {
	Controller* c = Controller::getInstance();
	return c->disconnectDevice();
}

int32_t connectDevice(uint32_t deviceToOpen) {
	Controller* c = Controller::getInstance();
	return c->connectDevice(deviceToOpen);
}

int32_t setMotorParameters(uint32_t polePairCount, uint32_t ratedCurrent, uint32_t maxCurrent, uint32_t maxCurrentDuration,uint32_t idleCurrent, uint32_t driveMode) {
	Controller* c = Controller::getInstance();
	return c->setMotorParameters(polePairCount, ratedCurrent, maxCurrent, maxCurrentDuration,idleCurrent, driveMode);
}

int32_t getMotorParameters(uint32_t& polePairCount, uint32_t& ratedCurrent, uint32_t& maxCurrent, uint32_t& maxCurrentDuration, uint32_t& idleCurrent, uint32_t& driveMode) {
	Controller* c = Controller::getInstance();
	return c->getMotorParameters(polePairCount, ratedCurrent, maxCurrent, maxCurrentDuration,idleCurrent, driveMode);
}

int32_t getPositioningParameters(uint32_t& profileVelocity, int32_t& setTarget) {
	Controller* c = Controller::getInstance();
	return c->getPositioningParameters(profileVelocity, setTarget);
}

int32_t saveMotorParameters() {
	Controller* c = Controller::getInstance();
	if (c->saveGroupTuning())
		return EXIT_FAILURE;
	return c->saveGroupMovement();
}

int32_t saveUserUnits() {
	Controller* c = Controller::getInstance();
	return c->saveGroupApplication();
}

int32_t getPositionActual(int32_t& pos) {
	Controller* c = Controller::getInstance();
	return c->getPositionActual(pos);
}


int32_t getFirmwareVersion(std::string& ver) {
	Controller* c = Controller::getInstance();
	return c->getDeviceFirmwareBuildId(ver);
}

int32_t getFirmwareVersionLV(LStrHandle* LVAllocatedStr) {
	Controller* c = Controller::getInstance();
	std::string ver;
	if (c->getDeviceFirmwareBuildId(ver))
		return EXIT_FAILURE;
	return stdStrToLVStr(ver, LVAllocatedStr);
}



//***POSITIONING***

int32_t setTargetPosition(int32_t val, uint32_t absRel) {
	Controller* c = Controller::getInstance();
	return c->setTargetPosition(val,absRel);
}

int32_t startPositioning() {
	Controller* c = Controller::getInstance();
	return c->startPositioning();
}

//profile positioning velocity
int32_t setProfileVelocity(uint32_t speed) {
	Controller* c = Controller::getInstance();
	return c->setProfileVelocity(speed);
}

int32_t setProfileAcceleration(uint32_t acc) {
	Controller* c = Controller::getInstance();
	return c->setProfileAcceleration(acc);
}


//***VELOCITY***

int32_t startVelocity() {
	Controller* c = Controller::getInstance();
	return c->startVelocity();
}

int32_t setVelocityPams(int16_t vel,uint32_t deltaSpeedAcc, uint16_t deltaTimeAcc,uint32_t deltaSpeedDec, uint16_t deltaTimeDec) {
	Controller* c = Controller::getInstance();
	if (c->setTargetVelocity(vel))
		return EXIT_FAILURE;
	if (c->setVelocityAcceleration(deltaSpeedAcc,deltaTimeAcc))
		return EXIT_FAILURE;
	return (c->setVelocityDeceleration(deltaSpeedDec, deltaTimeDec));
}

int32_t getVelocityPams(int16_t &vel, uint32_t &deltaSpeedAcc, uint16_t &deltaTimeAcc, uint32_t &deltaSpeedDec, uint16_t &deltaTimeDec) {
	Controller* c = Controller::getInstance();
	if (c->getTargetVelocity(vel))
		return EXIT_FAILURE;
	if (c->getVelocityAcceleration(deltaSpeedAcc,deltaTimeAcc))
		return EXIT_FAILURE;
	return (c->getVelocityDeceleration(deltaSpeedDec,deltaTimeDec));
}

int32_t getVelocityStatus(int16_t& demandedSpeed, int16_t& speedActual) {
	Controller* c = Controller::getInstance();
	if (c->getVelocityDemanded(demandedSpeed))
		return EXIT_FAILURE;
	return (c->getVelocityDemanded(speedActual));
}

//**HOMING***

int32_t setHomingAcceleration(uint32_t acc) {
	Controller* c = Controller::getInstance();
	return c->setHomingAcceleration(acc);
}

int32_t home(uint32_t speedZero, uint32_t speedSwitch) {
	Controller* c = Controller::getInstance();
	return c->home(speedZero, speedSwitch);
}



