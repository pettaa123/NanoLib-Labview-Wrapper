#pragma once

#include <bitset>

#include "motor.h"


// Motor in Profile Position Mode
class ProfilePositionMotor : public Motor402 {

public:
	ProfilePositionMotor(NanoLibHelper *nanolibHelper, std::optional<nlc::DeviceHandle> *connectedDeviceHandle, PowerSM *powerSM) :
		Motor402(nanolibHelper, connectedDeviceHandle,powerSM)
	{
		setModeOfOperation(OperationMode::ProfilePosition);
	}

	// Start the motor movement with specified speed
	int startPositioning() {
		//go
		uint16_t uWord16 = static_cast<uint16_t>(m_nanolibHelper->readInteger(m_connectedDeviceHandle->value(), nlc::OdIndex(0x6040, 0x00)));
		//immediate start of new target position
		uWord16 |= (1U << 5);
		//reset halt bit
		uWord16 &= ~(1U << 8);
		//set start bit
		uWord16 |= (1U << 4);
		m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value(), uWord16, nlc::OdIndex(0x6040, 0x00), 16);
		uWord16 = static_cast<uint16_t>(m_nanolibHelper->readInteger(m_connectedDeviceHandle->value(), nlc::OdIndex(0x6040, 0x00)));
		//set start bit

		//power sm to ready tp switch on
		if (m_powerSM->enableOperation())
			return EXIT_FAILURE;

		return EXIT_SUCCESS;
	}

	int halt() override {
		uint16_t uWord16 = static_cast<uint16_t>(m_nanolibHelper->readInteger(m_connectedDeviceHandle->value(), nlc::OdIndex(0x6040, 0x00)));
		//set halt bit
		uWord16 |= (1U << 8);
		m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value(), uWord16, nlc::OdIndex(0x6040, 0x00), 16);
		return EXIT_SUCCESS;
	}


	void setTargetPosition(int32_t value, uint32_t absRel) {

		uint16_t uWord16 = static_cast<uint16_t>(m_nanolibHelper->readInteger(m_connectedDeviceHandle->value(), nlc::OdIndex(0x6040, 0x00)));
		//interprate target as absolute/relative target position
		if (absRel == 1)//1:relative
			uWord16 |= (1U << 6);
		else
			uWord16 &= ~(1U << 6);

		//reset "new setpoint" bit)
		uWord16 &= ~(1U << 4);
		m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value(), uWord16, nlc::OdIndex(0x6040, 0x00), 16);

		//Limit Switch Error Option Code
		/*
		Abbremsen mit quick stop ramp und anschließendem
		Zustandswechsel in Switch on disabled
		*/
		m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value(), 2, nlc::OdIndex(0x3701, 0x00), 16);
		//607Ah Target Position
		m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value(), value, nlc::OdIndex(0x607A, 0x00), 32);

	}

	void setProfileVelocity(uint32_t speed) {
		//set target position
		m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value(), speed, nlc::OdIndex(0x6081, 0x00), 32);
		return;
	}

	void getPositioningParameters(uint32_t& profileVelocity, int32_t& targetPosition) {
		profileVelocity = static_cast<uint32_t>(m_nanolibHelper->readInteger(m_connectedDeviceHandle->value(), nlc::OdIndex(0x6081, 0x00)));
		targetPosition = static_cast<int32_t>(m_nanolibHelper->readInteger(m_connectedDeviceHandle->value(), nlc::OdIndex(0x607A, 0x00)));
	}

	void setProfileAcceleration(uint32_t acc) {
		//set profile acceleration
		m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value(), acc, nlc::OdIndex(0x6083, 0x00), 32);
		return;
	}

	int setUserUnitsPositioning(uint32_t posUnit, uint32_t posExp) {
		//make sure operation is disabled before changing user defined units
		if (m_powerSM->disableOperation())
			return EXIT_FAILURE;

		uint32_t val = static_cast<uint32_t>(m_nanolibHelper->readInteger(m_connectedDeviceHandle->value(), nlc::OdIndex(0x60A8, 0x00)));

		uint32_t unit = (posUnit << 16);
		uint32_t exp = (posExp << 24);

		//reset
		val &= ~0xFFFF0000;
		//set
		val = ((val |= unit) |= exp);

		m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value(), val, nlc::OdIndex(0x60A8, 0x00), 32);

		return EXIT_SUCCESS;
	}

	void getUserUnitsPositioning(uint32_t &unit, uint32_t &exp) {
		uint32_t val = static_cast<uint32_t>(m_nanolibHelper->readInteger(m_connectedDeviceHandle->value(), nlc::OdIndex(0x60A8, 0x00)));
		exp = (val >> 24) & 0xff;
		unit = (val >> 16) & 0xff;
	}


private:

	uint16_t getState() {
		return EXIT_SUCCESS;
	}

};


