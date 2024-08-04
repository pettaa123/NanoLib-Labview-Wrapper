#include "power_sm.h"
#include <format>

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

PowerSM::PowerSM(NanoLibHelper *nanolibHelper, std::optional<nlc::DeviceHandle> *connectedDeviceHandle) :
	nanolibHelper(nanolibHelper),
	connectedDeviceHandle(connectedDeviceHandle)
{};

PowerSM::~PowerSM() {
	DBOUT("destructing power sm");
};

bool PowerSM::StateChanged(uint8_t& lastState, uint8_t& currentState)
{
	if (GetCurrentState(currentState)) {
		return false;
	}
	return currentState != lastState;
}

int PowerSM::EnableOperation() {
	uint8_t currentState = 0;
	uint8_t lastState = 255;
	uint8_t errorCounter = 0;
	if (GetCurrentState(currentState))
		return EXIT_FAILURE;
	while (StateChanged(lastState, currentState)) {
		lastState = currentState;
		switch (currentState) {
		case States::NOT_READY_TO_SWITCH_ON:
			throw nanolib_exception(" Not ready to switch on: Error can't be solved by software.");
			return EXIT_FAILURE;
		case States::SWITCHED_ON_DISABLED:
			Shutdown_2_6_8();
			break;
		case States::READY_TO_SWITCH_ON:
			SwitchOn_3();
			break;
		case States::SWITCHED_ON:
			EnableOperation_4();
			break;
		case States::OPERATION_ENABLED:
			//already enabled
			break;
		case States::QUICK_STOP_ACTIVE:
			EnableOperationAfterQuickStop_16();
			break;
		case States::FAULT:
			//try 2 times and then remain in error
			if (errorCounter >= 2) {
				FaultReset_15();
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

int PowerSM::DisableOperation() {
	uint8_t currentState = 0;
	if (GetCurrentState(currentState))
		return EXIT_FAILURE;
	if (currentState == States::NOT_READY_TO_SWITCH_ON) {
		throw(nanolib_exception("Not ready to switch on: Error can't be solved by software"));
		return EXIT_FAILURE;
	}
	if (currentState == States::FAULT)
		FaultReset_15();
	if (currentState == States::OPERATION_ENABLED)
		DisableOperation_5();
	GetCurrentState(currentState);
	//still operation enabled?
	if (currentState == States::OPERATION_ENABLED) {
		throw(nanolib_exception("Couldn't disable operation"));
		return EXIT_FAILURE;
	}
	else
		return EXIT_SUCCESS;
}

int PowerSM::Shutdown() {
	uint8_t currentState = 0;
	if (GetCurrentState(currentState))
		return EXIT_FAILURE;
	if (currentState == States::NOT_READY_TO_SWITCH_ON) {
		throw(nanolib_exception("Not ready to switch on: Error can't be solved by software"));
		return EXIT_FAILURE;
	}
	if (currentState == States::FAULT)
		FaultReset_15();
	if (currentState == States::QUICK_STOP_ACTIVE)
		DisableVoltage_7_10_9_12();
	if (currentState == States::OPERATION_ENABLED || currentState == States::SWITCHED_ON || currentState == States::SWITCHED_ON_DISABLED)
		Shutdown_2_6_8();
	return EXIT_SUCCESS;
}

int PowerSM::QuickStop() {
	uint8_t currentState = 0;
	if (GetCurrentState(currentState))
		return EXIT_FAILURE;
	if (currentState == States::OPERATION_ENABLED) {
		QuickStop_11();
	}
	GetCurrentState(currentState);
	//still operation enabled?
	if (currentState == States::OPERATION_ENABLED) {
		throw(nanolib_exception("Quickstop failed"));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int PowerSM::GetCurrentState(uint8_t& state) {
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

void PowerSM::Shutdown_2_6_8() {
	uint16_t uWord16 = static_cast<uint16_t>(nanolibHelper->readInteger(connectedDeviceHandle->value(), nlc::OdIndex(0x6040, 0x00)));
	uWord16 &= ~(1U << 0);
	uWord16 |= (1U << 1);
	uWord16 |= (1U << 2);
	uWord16 &= ~(1U << 7);
	nanolibHelper->writeInteger(connectedDeviceHandle->value(), uWord16, nlc::OdIndex(0x6040, 0x00), 16);
}

void PowerSM::SwitchOn_3()
{
		uint16_t uWord16 = static_cast<uint16_t>(nanolibHelper->readInteger(connectedDeviceHandle->value(), nlc::OdIndex(0x6040, 0x00)));
		uWord16 |= (1U << 0);
		uWord16 |= (1U << 1);
		uWord16 |= (1U << 2);
		uWord16 &= ~(1U << 3);
		uWord16 &= ~(1U << 7);
		nanolibHelper->writeInteger(connectedDeviceHandle->value(), uWord16, nlc::OdIndex(0x6040, 0x00), 16);
}

void PowerSM::DisableVoltage_7_10_9_12()
{
		uint16_t uWord16 = static_cast<uint16_t>(nanolibHelper->readInteger(connectedDeviceHandle->value(), nlc::OdIndex(0x6040, 0x00)));
		uWord16 &= ~(1U << 1);
		uWord16 &= ~(1U << 7);
		nanolibHelper->writeInteger(connectedDeviceHandle->value(), uWord16, nlc::OdIndex(0x6040, 0x00), 16);
}
void PowerSM::QuickStop_11(){
		uint16_t uWord16 = static_cast<uint16_t>(nanolibHelper->readInteger(connectedDeviceHandle->value(), nlc::OdIndex(0x6040, 0x00)));
		uWord16 |= (1U << 1);
		uWord16 &= ~(1U << 2);
		uWord16 &= ~(1U << 7);
		nanolibHelper->writeInteger(connectedDeviceHandle->value(), uWord16, nlc::OdIndex(0x6040, 0x00), 16);
}

void PowerSM::DisableOperation_5()
{
	uint16_t uWord16 = static_cast<uint16_t>(nanolibHelper->readInteger(connectedDeviceHandle->value(), nlc::OdIndex(0x6040, 0x00)));
	uWord16 |= (1U << 0);
	uWord16 |= (1U << 1);
	uWord16 |= (1U << 2);
	uWord16 &= ~(1U << 3);
	uWord16 &= ~(1U << 7);
	nanolibHelper->writeInteger(connectedDeviceHandle->value(), uWord16, nlc::OdIndex(0x6040, 0x00), 16);
}

void PowerSM::EnableOperation_4()
{
	uint16_t uWord16 = static_cast<uint16_t>(nanolibHelper->readInteger(connectedDeviceHandle->value(), nlc::OdIndex(0x6040, 0x00)));
	uWord16 |= (1U << 0);
	uWord16 |= (1U << 1);
	uWord16 |= (1U << 2);
	uWord16 |= (1U << 3);
	uWord16 &= ~(1U << 7);
	nanolibHelper->writeInteger(connectedDeviceHandle->value(), uWord16, nlc::OdIndex(0x6040, 0x00), 16);
}

void PowerSM::EnableOperationAfterQuickStop_16() {
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

void PowerSM::FaultReset_15()
{
	uint16_t uWord16 = static_cast<uint16_t>(nanolibHelper->readInteger(connectedDeviceHandle->value(), nlc::OdIndex(0x6040, 0x00)));
	if ((uWord16 >> 7) & 1U) {
		uWord16 &= ~(1U << 7);
		nanolibHelper->writeInteger(connectedDeviceHandle->value(), uWord16, nlc::OdIndex(0x6040, 0x00), 16);
	}
	uWord16 |= 1U << 7;
}
