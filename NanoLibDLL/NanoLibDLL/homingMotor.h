#pragma once

#include <bitset>

#include "motor.h"
/*
Aufgabe der Referenzfahrt(Homing Method) ist es, den Positionsnullpunkt der Steuerung auf einen
Encoder - Index bzw.Positionsschalter auszurichten.
*/
class HomingMotor : public Motor402 {

public:
	HomingMotor(NanoLibHelper *nanolibHelper, std::optional<nlc::DeviceHandle> *connectedDeviceHandle, PowerSM *powerSM) :
		Motor402(nanolibHelper, connectedDeviceHandle, powerSM)
	{
		setModeOfOperation(OperationMode::Homing);
	}


	enum Status : uint16_t {
		H_IN_PROGRESS,
		H_INCOMPLETE,
		H_UNACHIEVED,
		H_COMPLETED,
		H_ERROR_STILL_MOVING,
		H_ERROR_HALT
	};

	// Start the motor movement with specified speed
	int startHoming() {

		//ensure homing is not already running
		uint16_t cs = getState();

		if (cs == Status::H_IN_PROGRESS) {
			uint16_t uWord16 = static_cast<uint16_t>(m_nanolibHelper->readInteger(m_connectedDeviceHandle->value(), nlc::OdIndex(0x6040, 0x00)));
			uWord16 &= ~(1U << 4);
			m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value(), uWord16, nlc::OdIndex(0x6040, 0x00), 16);
		}

		//Limit Switch Error Option Code
		
		/*keine Reaktion(um z.B.eine Referenzfahrt durchzuführen), außer
		Vermerken der Endschalterposition*/
		m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value(), -1, nlc::OdIndex(0x3701, 0x00), 16);

		uint16_t uWord16 = static_cast<uint16_t>(m_nanolibHelper->readInteger(m_connectedDeviceHandle->value(), nlc::OdIndex(0x6040, 0x00)));
		//go set start bit
		uWord16 |= (1U << 4);
		m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value(), uWord16, nlc::OdIndex(0x6040, 0x00), 16);		

		if (m_powerSM->enableOperation())
			return EXIT_FAILURE;

		return EXIT_SUCCESS;
	}

	int halt() override {
		uint16_t uWord16 = static_cast<uint16_t>(m_nanolibHelper->readInteger(m_connectedDeviceHandle->value(), nlc::OdIndex(0x6040, 0x00)));
		//reset start bit
		uWord16 &= ~(1U << 4);
		m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value(), uWord16, nlc::OdIndex(0x6040, 0x00), 16);
		return EXIT_SUCCESS;
	}



	int home(uint32_t speedZero=10, uint32_t speedSwitch=50) {
		if (setHomingSpeed(speedZero, speedSwitch))
			return EXIT_FAILURE;
		//reference is positive end switch method 17
		setHomingMode(17);
		if (startHoming())
			return EXIT_FAILURE;
		return EXIT_SUCCESS;
	}

	void setHomingAcceleration(uint32_t acc){
		m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value(), acc, nlc::OdIndex(0x609A, 0x00), 32);
	}

	void setHomingMode(uint8_t method)
	{
		m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value(), method, nlc::OdIndex(0x6098, 0x00), 8);
	}

	int setHomingSpeed(uint32_t speedZero, uint32_t speedSwitch) {
		//SPEED OF INDEX 1 MAY BE LARGER THAN INDEX 2
		if (speedZero >= speedSwitch) {
			throw(nanolib_exception{ "zeroing speed may be less than for switching" });
			return EXIT_FAILURE;
		}
		//set speed during search for zero
		m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value(), speedZero, nlc::OdIndex(0x6099, 0x02), 32);
		//set speed during search for switch
		m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value() , speedSwitch, nlc::OdIndex(0x6099, 0x01), 32);
		return EXIT_SUCCESS;
	}

	/*todo
	home offset
	homing method
	minimum current for block detection
	period of blocking
	*/

	uint16_t getState() {
		uint16_t uWord16 = static_cast<uint16_t>(m_nanolibHelper->readInteger(m_connectedDeviceHandle->value(), nlc::OdIndex(0x6041, 0x00)));
		//Referenzfahrt wird ausgeführt?
		if (
			!(uWord16 & (1U << 13)) &&
			!(uWord16 & (1U << 12)) &&
			!(uWord16 & (1U << 10))
			)
		{
			return Status::H_IN_PROGRESS;
		}
		//Referenzfahrt ist unterbrochen oder nicht gestartet?
		else if (
			!(uWord16 & (1U << 13)) &&
			!(uWord16 & (1U << 12)) &&
			(uWord16 & (1U << 10))
			) 
		{
			return Status::H_INCOMPLETE;
		}
		//Referenzfahrt ist seit dem letzten Neustart bereits durchgeführt
		//worden, aber Ziel ist aktuell nicht erreicht
		else if (
			!(uWord16 & (1U << 13)) &&
			(uWord16 & (1U << 12)) &&
			!(uWord16 & (1U << 10))
			)
		{
			return Status::H_UNACHIEVED;
		}
		//Referenzfahrt vollständig abgeschlossen
		else if (
			!(uWord16 & (1U << 13)) &&
			(uWord16 & (1U << 12)) &&
			(uWord16 & (1U << 10))
			)
		{
			return Status::H_COMPLETED;
		}
		//Fehler während der Referenzfahrt, Motor dreht sich noch
		else if (
			(uWord16 & (1U << 13)) &&
			!(uWord16 & (1U << 12)) &&
			!(uWord16 & (1U << 10))
			)
		{
			return Status::H_ERROR_STILL_MOVING;
		}
		//Fehler während der Referenzfahrt, Motor im Stillstand
		else if (
			(uWord16 & (1U << 13)) &&
			!(uWord16 & (1U << 12)) &&
			(uWord16 & (1U << 10))
			)
		{
			return Status::H_ERROR_HALT;
		}
		throw(nanolib_exception("Unknown Homing State"));
		return 0xFF;
	}

};


