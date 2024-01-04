#include "powerSM.h"
#include <format>

PowerSM::PowerSM(NanoLibHelper *nanolibHelper, std::optional<nlc::DeviceHandle> *connectedDeviceHandle) :
	nanolibHelper(nanolibHelper),
	connectedDeviceHandle(connectedDeviceHandle)
{};

PowerSM::~PowerSM() {
	std::cout << "destructing power sm" << std::endl;
};

bool PowerSM::stateChanged(uint8_t& lastState, uint8_t& currentState)
{
	if (getCurrentState(currentState)) {
		return false;
	}
	return currentState != lastState;
}

int PowerSM::enableOperation() {
	uint8_t currentState = 0;
	uint8_t lastState = 255;
	uint8_t errorCounter = 0;
	if (getCurrentState(currentState))
		return EXIT_FAILURE;
	while (stateChanged(lastState, currentState)) {
		lastState = currentState;
		switch (currentState) {
		case States::NOT_READY_TO_SWITCH_ON:
			throw nanolib_exception(" Not ready to switch on: Error can't be solved by software.");
			return EXIT_FAILURE;
		case States::SWITCHED_ON_DISABLED:
			shutdown_2_6_8();
			break;
		case States::READY_TO_SWITCH_ON:
			switchOn_3();
			break;
		case States::SWITCHED_ON:
			enableOperation_4();
			break;
		case States::OPERATION_ENABLED:
			//already enabled
			break;
		case States::QUICK_STOP_ACTIVE:
			enableOperationAfterQuickStop_16();
			break;
		case States::FAULT:
			//try 2 times and then remain in error
			if (errorCounter >= 2) {
				faultReset_15();
				return EXIT_FAILURE;
			}
			errorCounter++;
			break;
		default:
			break;
		}
	}

	return EXIT_SUCCESS;
}

int PowerSM::disableOperation() {
	uint8_t currentState = 0;
	if (getCurrentState(currentState))
		return EXIT_FAILURE;
	if (currentState == States::NOT_READY_TO_SWITCH_ON) {
		throw(nanolib_exception("Not ready to switch on: Error can't be solved by software"));
		return EXIT_FAILURE;
	}
	if (currentState == States::FAULT)
		faultReset_15();
	if (currentState == States::OPERATION_ENABLED)
		disableOperation_5();
	getCurrentState(currentState);
	//still operation enabled?
	if (currentState == States::OPERATION_ENABLED) {
		throw(nanolib_exception("Couldn't disable operation"));
		return EXIT_FAILURE;
	}
	else
		return EXIT_SUCCESS;
}

int PowerSM::shutdown() {
	uint8_t currentState = 0;
	if (getCurrentState(currentState))
		return EXIT_FAILURE;
	if (currentState == States::NOT_READY_TO_SWITCH_ON) {
		throw(nanolib_exception("Not ready to switch on: Error can't be solved by software"));
		return EXIT_FAILURE;
	}
	if (currentState == States::FAULT)
		faultReset_15();
	if (currentState == States::QUICK_STOP_ACTIVE)
		disableVoltage_7_10_9_12();
	if (currentState == States::OPERATION_ENABLED || currentState == States::SWITCHED_ON || currentState == States::SWITCHED_ON_DISABLED)
		shutdown_2_6_8();
	return EXIT_SUCCESS;
}

int PowerSM::quickStop() {
	uint8_t currentState = 0;
	if (getCurrentState(currentState))
		return EXIT_FAILURE;
	if (currentState == States::OPERATION_ENABLED) {
		quickStop_11();
	}
	getCurrentState(currentState);
	//still operation enabled?
	if (currentState == States::OPERATION_ENABLED) {
		throw(nanolib_exception("Quickstop failed"));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int PowerSM::getCurrentState(uint8_t& state) {
	uint16_t uWord16 = static_cast<uint16_t>(nanolibHelper->readInteger(connectedDeviceHandle->value(), nlc::OdIndex(0x6041, 0x00)));
	//figure out state
	if (
		((uWord16 >> 0) & 1U) == 0 &&
		((uWord16 >> 1) & 1U) == 0 &&
		((uWord16 >> 2) & 1U) == 0 &&
		((uWord16 >> 3) & 1U) == 0 &&
		((uWord16 >> 6) & 1U) == 0
		)
	{
		state = States::NOT_READY_TO_SWITCH_ON;
	}
	else if (
		((uWord16 >> 0) & 1U) == 0 &&
		((uWord16 >> 1) & 1U) == 0 &&
		((uWord16 >> 2) & 1U) == 0 &&
		((uWord16 >> 3) & 1U) == 0 &&
		((uWord16 >> 6) & 1U) == 1
		)
	{
		state = States::SWITCHED_ON_DISABLED;
	}
	else if (
		((uWord16 >> 0) & 1U) == 1 &&
		((uWord16 >> 1) & 1U) == 0 &&
		((uWord16 >> 2) & 1U) == 0 &&
		((uWord16 >> 3) & 1U) == 0 &&
		((uWord16 >> 5) & 1U) == 1 &&
		((uWord16 >> 6) & 1U) == 0
		)
	{
		state = States::READY_TO_SWITCH_ON;
	}
	else if (
		((uWord16 >> 0) & 1U) == 1 &&
		((uWord16 >> 1) & 1U) == 1 &&
		((uWord16 >> 2) & 1U) == 0 &&
		((uWord16 >> 3) & 1U) == 0 &&
		((uWord16 >> 5) & 1U) == 1 &&
		((uWord16 >> 6) & 1U) == 0
		)
	{
		state = States::SWITCHED_ON;
	}
	else if (
		((uWord16 >> 0) & 1U) == 1 &&
		((uWord16 >> 1) & 1U) == 1 &&
		((uWord16 >> 2) & 1U) == 1 &&
		((uWord16 >> 3) & 1U) == 0 &&
		((uWord16 >> 5) & 1U) == 1 &&
		((uWord16 >> 6) & 1U) == 0
		)
	{
		state = States::OPERATION_ENABLED;
	}
	else if (
		((uWord16 >> 0) & 1U) == 1 &&
		((uWord16 >> 1) & 1U) == 1 &&
		((uWord16 >> 2) & 1U) == 1 &&
		((uWord16 >> 3) & 1U) == 0 &&
		((uWord16 >> 5) & 1U) == 0 &&
		((uWord16 >> 6) & 1U) == 0
		)
	{
		state = States::QUICK_STOP_ACTIVE;
	}
	else if (
		((uWord16 >> 0) & 1U) == 1 &&
		((uWord16 >> 1) & 1U) == 1 &&
		((uWord16 >> 2) & 1U) == 1 &&
		((uWord16 >> 3) & 1U) == 1 &&
		((uWord16 >> 6) & 1U) == 0
		)
	{
		state = States::FAULT_REACTION_ACTIVE;
	}
	else if (
		((uWord16 >> 0) & 1U) == 0 &&
		((uWord16 >> 1) & 1U) == 0 &&
		((uWord16 >> 2) & 1U) == 0 &&
		((uWord16 >> 3) & 1U) == 1 &&
		((uWord16 >> 6) & 1U) == 0
		)
	{
		state = States::FAULT;
	}
	else {
		throw(nanolib_exception("Unknown CIA402 state"));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

void PowerSM::shutdown_2_6_8() {
	uint16_t uWord16 = static_cast<uint16_t>(nanolibHelper->readInteger(connectedDeviceHandle->value(), nlc::OdIndex(0x6040, 0x00)));
	uWord16 &= ~(1U << 0);
	uWord16 |= (1U << 1);
	uWord16 |= (1U << 2);
	uWord16 &= ~(1U << 7);
	nanolibHelper->writeInteger(connectedDeviceHandle->value(), uWord16, nlc::OdIndex(0x6040, 0x00), 16);
}

void PowerSM::switchOn_3()
{
		uint16_t uWord16 = static_cast<uint16_t>(nanolibHelper->readInteger(connectedDeviceHandle->value(), nlc::OdIndex(0x6040, 0x00)));
		uWord16 |= (1U << 0);
		uWord16 |= (1U << 1);
		uWord16 |= (1U << 2);
		uWord16 &= ~(1U << 3);
		uWord16 &= ~(1U << 7);
		nanolibHelper->writeInteger(connectedDeviceHandle->value(), uWord16, nlc::OdIndex(0x6040, 0x00), 16);
}

void PowerSM::disableVoltage_7_10_9_12()
{
		uint16_t uWord16 = static_cast<uint16_t>(nanolibHelper->readInteger(connectedDeviceHandle->value(), nlc::OdIndex(0x6040, 0x00)));
		uWord16 &= ~(1U << 1);
		uWord16 &= ~(1U << 7);
		nanolibHelper->writeInteger(connectedDeviceHandle->value(), uWord16, nlc::OdIndex(0x6040, 0x00), 16);
}
void PowerSM::quickStop_11(){
		uint16_t uWord16 = static_cast<uint16_t>(nanolibHelper->readInteger(connectedDeviceHandle->value(), nlc::OdIndex(0x6040, 0x00)));
		uWord16 |= (1U << 1);
		uWord16 &= ~(1U << 2);
		uWord16 &= ~(1U << 7);
		nanolibHelper->writeInteger(connectedDeviceHandle->value(), uWord16, nlc::OdIndex(0x6040, 0x00), 16);
}

void PowerSM::disableOperation_5()
{
	uint16_t uWord16 = static_cast<uint16_t>(nanolibHelper->readInteger(connectedDeviceHandle->value(), nlc::OdIndex(0x6040, 0x00)));
	uWord16 |= (1U << 0);
	uWord16 |= (1U << 1);
	uWord16 |= (1U << 2);
	uWord16 &= ~(1U << 3);
	uWord16 &= ~(1U << 7);
	nanolibHelper->writeInteger(connectedDeviceHandle->value(), uWord16, nlc::OdIndex(0x6040, 0x00), 16);
}

void PowerSM::enableOperation_4()
{
	uint16_t uWord16 = static_cast<uint16_t>(nanolibHelper->readInteger(connectedDeviceHandle->value(), nlc::OdIndex(0x6040, 0x00)));
	uWord16 |= (1U << 0);
	uWord16 |= (1U << 1);
	uWord16 |= (1U << 2);
	uWord16 |= (1U << 3);
	uWord16 &= ~(1U << 7);
	nanolibHelper->writeInteger(connectedDeviceHandle->value(), uWord16, nlc::OdIndex(0x6040, 0x00), 16);
}

void PowerSM::enableOperationAfterQuickStop_16() {
	uint16_t uWord16 = static_cast<uint16_t>(nanolibHelper->readInteger(connectedDeviceHandle->value(), nlc::OdIndex(0x6040, 0x00)));
	uWord16 &= ~(1U << 2);
	nanolibHelper->writeInteger(connectedDeviceHandle->value(), uWord16, nlc::OdIndex(0x6040, 0x00), 16);
	uWord16 = static_cast<uint16_t>(nanolibHelper->readInteger(connectedDeviceHandle->value(), nlc::OdIndex(0x6040, 0x00)));
	uWord16 |= (1U << 0);
	uWord16 |= (1U << 1);
	uWord16 |= (1U << 2);
	uWord16 |= (1U << 3);
	uWord16 &= ~(1U << 7);
	nanolibHelper->writeInteger(connectedDeviceHandle->value(), uWord16, nlc::OdIndex(0x6040, 0x00), 16);
}

void PowerSM::faultReset_15()
{
	uint16_t uWord16 = static_cast<uint16_t>(nanolibHelper->readInteger(connectedDeviceHandle->value(), nlc::OdIndex(0x6040, 0x00)));
	if ((uWord16 >> 7) & 1U) {
		uWord16 &= ~(1U << 7);
		nanolibHelper->writeInteger(connectedDeviceHandle->value(), uWord16, nlc::OdIndex(0x6040, 0x00), 16);
	}
	uWord16 |= 1U << 7;
}
