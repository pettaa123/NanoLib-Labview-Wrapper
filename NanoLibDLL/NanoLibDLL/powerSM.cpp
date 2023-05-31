#include "pch.h"
#include "powerSM.h"

PowerSM::PowerSM(NanoLibHelper& nanolibHelper, nlc::BusHardwareId& openedBusHardware, nlc::DeviceHandle& connectedDeviceHandle) :
	nanolibHelper(nanolibHelper),
	openedBusHardware(openedBusHardware),
	connectedDeviceHandle(connectedDeviceHandle)
{};

PowerSM::~PowerSM() {};

bool PowerSM::stateChanged(uint8_t &lastState, uint8_t &currentState)
{
	if (getCurrentState(currentState)) {
		return false;
	}
	return currentState != lastState;
}

int PowerSM::enableOperation() {
	uint8_t currentState = 0;
	uint8_t lastState = 255;
	if (getCurrentState(currentState))
		return EXIT_FAILURE;
	while (stateChanged(lastState,currentState)) {
		lastState = currentState;
		switch (currentState) {
		case ST_NOT_READY_TO_SWITCH_ON:
			throw nanolib_exception(" Not ready to switch on: Error can't be solved by software.");
			return EXIT_FAILURE;
		case ST_SWITCHED_ON_DISABLED:
			shutdown_1_5_8();
			break;
		case ST_READY_TO_SWITCH_ON:
			switchOn_2();
			break;
		case ST_SWITCHED_ON:
			enableOperation_3();
			break;
		case ST_OPERATION_ENABLED:
			//already enabled
			break;
		case ST_QUICK_STOP_ACTIVE:
			disableVoltage_6_7_9_12();
			break;
		case ST_FAULT:
			faultReset_13();
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
	if (currentState == ST_OPERATION_ENABLED)
		if (disableOperation_4())
			return EXIT_FAILURE;
	getCurrentState(currentState);
	//still operation enabled?
	if (currentState == ST_OPERATION_ENABLED)
		return EXIT_FAILURE;
	else
		return EXIT_SUCCESS;
}

int PowerSM::getCurrentState(uint8_t& state) {
	try {
		uint16_t uWord16 = static_cast<uint16_t>(nanolibHelper.readInteger(connectedDeviceHandle, nlc::OdIndex(0x6041, 0x00)));
		//figure out state
		if (
			((uWord16 >> 0) & 1U) == 0 &&
			((uWord16 >> 1) & 1U) == 0 &&
			((uWord16 >> 2) & 1U) == 0 &&
			((uWord16 >> 3) & 1U) == 0 &&
			((uWord16 >> 6) & 1U) == 0
			)
		{
			state = States::ST_NOT_READY_TO_SWITCH_ON;
		}
		else if (
			((uWord16 >> 0) & 1U) == 0 &&
			((uWord16 >> 1) & 1U) == 0 &&
			((uWord16 >> 2) & 1U) == 0 &&
			((uWord16 >> 3) & 1U) == 0 &&
			((uWord16 >> 6) & 1U) == 1
			)
		{
			state = States::ST_SWITCHED_ON_DISABLED;
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
			state = States::ST_READY_TO_SWITCH_ON;
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
			state = States::ST_SWITCHED_ON;
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
			state = States::ST_OPERATION_ENABLED;
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
			state = States::ST_QUICK_STOP_ACTIVE;
		}
		else if (
			((uWord16 >> 0) & 1U) == 1 &&
			((uWord16 >> 1) & 1U) == 1 &&
			((uWord16 >> 2) & 1U) == 1 &&
			((uWord16 >> 3) & 1U) == 1 &&
			((uWord16 >> 6) & 1U) == 0
			)
		{
			state = States::ST_FAULT_REACTION_ACTIVE;
		}
		else if (
			((uWord16 >> 0) & 1U) == 0 &&
			((uWord16 >> 1) & 1U) == 0 &&
			((uWord16 >> 2) & 1U) == 0 &&
			((uWord16 >> 3) & 1U) == 1 &&
			((uWord16 >> 6) & 1U) == 0
			)
		{
			state = States::ST_FAULT;
		}
		else {
			DBOUT("WTF")
			return EXIT_FAILURE;
		}

	}
	catch (const nanolib_exception& e) {
		//"Error occurred e.what();
		DBOUT("exception: " << e.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int PowerSM::shutdown_1_5_8() {
	try
	{
		uint16_t uWord16 = static_cast<uint16_t>(nanolibHelper.readInteger(connectedDeviceHandle, nlc::OdIndex(0x6040, 0x00)));
		uWord16 &= ~(1U << 0);
		uWord16 |= 1U << 1;
		uWord16 |= 1U << 2;
		uWord16 &= ~(1U << 7);
		nanolibHelper.writeInteger(connectedDeviceHandle, uWord16, nlc::OdIndex(0x6040, 0x00), 16);
	}
	catch (const nanolib_exception& e) {
		//"Error occurred e.what();
		DBOUT("exception: " << e.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;

}

int PowerSM::switchOn_2()
{
	try
	{
		uint16_t uWord16 = static_cast<uint16_t>(nanolibHelper.readInteger(connectedDeviceHandle, nlc::OdIndex(0x6040, 0x00)));
		uWord16 |= 1U << 0;
		uWord16 |= 1U << 1;
		uWord16 |= 1U << 2;
		uWord16 &= ~(1U << 3);
		uWord16 &= ~(1U << 7);
		nanolibHelper.writeInteger(connectedDeviceHandle, uWord16, nlc::OdIndex(0x6040, 0x00), 16);
	}
	catch (const nanolib_exception& e) {
		//"Error occurred e.what();
		DBOUT("exception: " << e.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int PowerSM::disableVoltage_6_7_9_12() {
	try
	{
		uint16_t uWord16 = static_cast<uint16_t>(nanolibHelper.readInteger(connectedDeviceHandle, nlc::OdIndex(0x6040, 0x00)));
		uWord16 &= ~(1U << 1);
		uWord16 &= ~(1U << 7);
		nanolibHelper.writeInteger(connectedDeviceHandle, uWord16, nlc::OdIndex(0x6040, 0x00), 16);
	}
	catch (const nanolib_exception& e) {
		//"Error occurred e.what();
		DBOUT("exception: " << e.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
int PowerSM::quickStop_10()
{
	try
	{
		uint16_t uWord16 = static_cast<uint16_t>(nanolibHelper.readInteger(connectedDeviceHandle, nlc::OdIndex(0x6040, 0x00)));
		uWord16 |= 1U << 1;
		uWord16 &= ~(1U << 2);
		uWord16 &= ~(1U << 7);
		nanolibHelper.writeInteger(connectedDeviceHandle, uWord16, nlc::OdIndex(0x6040, 0x00), 16);
	}
	catch (const nanolib_exception& e) {
		//"Error occurred e.what();
		DBOUT("exception: " << e.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int PowerSM::disableOperation_4()
{
	try
	{
		uint16_t uWord16 = static_cast<uint16_t>(nanolibHelper.readInteger(connectedDeviceHandle, nlc::OdIndex(0x6040, 0x00)));
		uWord16 |= 1U << 0;
		uWord16 |= 1U << 1;
		uWord16 |= 1U << 2;
		uWord16 &= ~(1U << 3);
		uWord16 &= ~(1U << 7);
		nanolibHelper.writeInteger(connectedDeviceHandle, uWord16, nlc::OdIndex(0x6040, 0x00), 16);
	}
	catch (const nanolib_exception& e) {
		//"Error occurred e.what();
		DBOUT("exception: " << e.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int PowerSM::enableOperation_3() {
	try
	{
		uint16_t uWord16 = static_cast<uint16_t>(nanolibHelper.readInteger(connectedDeviceHandle, nlc::OdIndex(0x6040, 0x00)));
		uWord16 |= 1U << 0;
		uWord16 |= 1U << 1;
		uWord16 |= 1U << 2;
		uWord16 |= 1U << 3;
		uWord16 &= ~(1U << 7);
		nanolibHelper.writeInteger(connectedDeviceHandle, uWord16, nlc::OdIndex(0x6040, 0x00), 16);
	}
	catch (const nanolib_exception& e) {
		//"Error occurred e.what();
		DBOUT("exception: " << e.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int PowerSM::faultReset_13() {
	try
	{
		uint16_t uWord16 = static_cast<uint16_t>(nanolibHelper.readInteger(connectedDeviceHandle, nlc::OdIndex(0x6040, 0x00)));
		if ((uWord16 >> 7) & 1U) {
			uWord16 &= ~(1U << 7);
			nanolibHelper.writeInteger(connectedDeviceHandle, uWord16, nlc::OdIndex(0x6040, 0x00), 16);
		}
		uWord16 |= 1U << 7;
		nanolibHelper.writeInteger(connectedDeviceHandle, uWord16, nlc::OdIndex(0x6040, 0x00), 16);
	}
	catch (const nanolib_exception& e) {
		//"Error occurred e.what();
		DBOUT("exception: " << e.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
