#include <chrono>
#include <thread>
#include "motor.h"


Motor402::Motor402(NanoLibHelper *nanolibHelper, std::optional<nlc::DeviceHandle> *connectedDeviceHandle, PowerSM *powerSM) :
	m_nanolibHelper(nanolibHelper),
	m_connectedDeviceHandle(connectedDeviceHandle),
	m_powerSM(powerSM)
{
}


int Motor402::setModeOfOperation(int8_t mode) {
	// get current mode of operat
	if (getModeOfOperation() == mode)
		return EXIT_SUCCESS;
	//necessary? what happens when chaning operation mode to power sm
	if (m_powerSM->disableOperation())
		return EXIT_FAILURE;

	m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value(), mode, nlc::OdIndex(0x6060, 0x00), 8);

	return EXIT_SUCCESS;
}

void Motor402::getMotorParameters(uint32_t& polePairCount, uint32_t& ratedCurrent, uint32_t& maxCurrent, uint32_t& maxCurrentDuration, uint32_t& idleCurrent, DriveMode& driveMode) {
	//get polpaarzahl
	polePairCount = static_cast<uint32_t>(m_nanolibHelper->readInteger(m_connectedDeviceHandle->value(), nlc::OdIndex(0x2030, 0x00)));
	// get motorstrom  maximal zulässigen Motorstrom (Motorschutz) in mA
	maxCurrent = static_cast<uint32_t>(m_nanolibHelper->readInteger(m_connectedDeviceHandle->value(), nlc::OdIndex(0x2031, 0x00)));
	//  Nennstrom des Motors in mA (siehe Motordatenblatt) e
	ratedCurrent = static_cast<uint32_t>(m_nanolibHelper->readInteger(m_connectedDeviceHandle->value(), nlc::OdIndex(0x203B, 0x01)));
	//max duration of max current in ms
	maxCurrentDuration = static_cast<uint32_t>(m_nanolibHelper->readInteger(m_connectedDeviceHandle->value(), nlc::OdIndex(0x203B, 0x02)));
	//idle current in mA
	idleCurrent = static_cast<uint32_t>(m_nanolibHelper->readInteger(m_connectedDeviceHandle->value(), nlc::OdIndex(0x2037, 0x00)));

	uint32_t uWord32 = static_cast<uint32_t>(m_nanolibHelper->readInteger(m_connectedDeviceHandle->value(), nlc::OdIndex(0x3202, 0x00)));
	if (!(uWord32 & (1U << 0))) {//open loop
		if (uWord32 & (1U << 3))
			driveMode = DriveMode::STEPPER_OPEN_LOOP_W_CURR_REDUCTION;
		else
			driveMode = DriveMode::STEPPER_OPEN_LOOP_WO_CURR_REDUCTION;
	}
	else if (uWord32 & (1U << 6))
		driveMode = DriveMode::BLDC;
	else
		driveMode = DriveMode::STEPPER_CLOSED_LOOP;
	return;
}

int8_t Motor402::getModeOfOperation() {
	// get current mode of operation
	return static_cast<int8_t>(m_nanolibHelper->readInteger(m_connectedDeviceHandle->value(), nlc::OdIndex(0x6061, 0x00)));
}

int Motor402::setMotorParameters(uint32_t polePairCount, uint32_t ratedCurrent, uint32_t maxCurrent, uint32_t maxCurrentDuration, uint32_t idleCurrent, DriveMode driveMode) {

	if (m_powerSM->disableOperation())
		return EXIT_FAILURE;

	//write polpaarzahl
	m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value(), polePairCount, nlc::OdIndex(0x2030, 0x00), 32);
	// write motorstrom  maximal zulässigen Motorstrom (Motorschutz) in mA
	m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value(), maxCurrent, nlc::OdIndex(0x2031, 0x00), 32);
	//  Nennstrom des Motors in mA (siehe Motordatenblatt) e
	m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value(), ratedCurrent, nlc::OdIndex(0x203B, 0x01), 32);
	//max duration of max current in ms
	m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value(), maxCurrentDuration, nlc::OdIndex(0x203B, 0x02), 32);
	//open loop idle current
	m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value(), idleCurrent, nlc::OdIndex(0x2037, 0x00), 32);
	//motor type
	uint32_t uWord32 = static_cast<uint32_t>(m_nanolibHelper->readInteger(m_connectedDeviceHandle->value(), nlc::OdIndex(0x3202, 0x00)));

	switch (driveMode) {
	case DriveMode::BLDC:
		uWord32 |= driveMode;
		break;
	case  DriveMode::STEPPER_CLOSED_LOOP:
		uWord32 &= ~(1UL << 6); //reset bldc
		uWord32 |= (1UL << 0); //set closed loop
		break;
	case DriveMode::STEPPER_OPEN_LOOP_WO_CURR_REDUCTION:
		uWord32 &= ~(1UL << 6); //reset bldc
		uWord32 &= ~(1UL << 0); //reset closed loop
		uWord32 &= ~(1UL << 3); //reset current reduction
		break;
	case DriveMode::STEPPER_OPEN_LOOP_W_CURR_REDUCTION:
		uWord32 &= ~(1UL << 6); //reset bldc
		uWord32 &= ~(1UL << 0); //reset closed loop
		uWord32 |= (1UL << 3); //set current reduction
		break;
	default:
		throw(nanolib_exception("unknown drive mode"));
		break;
	}
	m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value(), uWord32, nlc::OdIndex(0x3202, 0x00), 32);

	return EXIT_SUCCESS;

}

int Motor402::saveGroup(uint8_t group) {
	if (m_powerSM->disableOperation())
		return EXIT_FAILURE;

	m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value(), 1702257011, nlc::OdIndex(0x1010, group), 32);

	uint16_t iterationsDone = 0;
	uint16_t maxIterations = 300;

	uint32_t uWord32;

	using namespace std::chrono_literals; // ns, us, ms, s, h, etc.

	do {
		uWord32 = static_cast<uint32_t>(
			m_nanolibHelper->readInteger(m_connectedDeviceHandle->value(), nlc::OdIndex(0x1010, group)));
		DBOUT("Save running\n");
		std::this_thread::sleep_for(100ms);
		iterationsDone += 1;
		if (iterationsDone == maxIterations) {
			throw(nanolib_exception("Saving timeout"));
			return EXIT_FAILURE;
		}
	} while (uWord32 != 1);
	DBOUT("Saving done\n");

	return EXIT_SUCCESS;
}


int Motor402::setUserUnitsFeed(uint32_t feedPer, uint32_t shaftRevolutions) {
	//make sure operation is disabled before changing user defined units
	if (m_powerSM->disableOperation()) {
		throw(nanolib_exception("Couldn't disable operation"));
		return EXIT_FAILURE;
	}

	m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value(), feedPer, nlc::OdIndex(0x6092, 0x01), 32);
	m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value(), shaftRevolutions, nlc::OdIndex(0x6092, 0x02), 32);
	return EXIT_SUCCESS;
}

int32_t Motor402::getSpeedActual() {
	return static_cast<int32_t>(m_nanolibHelper->readInteger(m_connectedDeviceHandle->value(), nlc::OdIndex(0x606C, 0x00)));
}

int32_t Motor402::getPositionActual() {
	return static_cast<int32_t>(m_nanolibHelper->readInteger(m_connectedDeviceHandle->value(), nlc::OdIndex(0x6064, 0x00)));
}

void Motor402::setMaxMotorSpeed(uint32_t maxSpeed) {
	m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value(), maxSpeed, nlc::OdIndex(0x6080, 0x00), 32);
}

int Motor402::halt() {
	auto mode=getModeOfOperation();
	if (
		(mode != OperationMode::ProfilePosition) && (mode != OperationMode::Velocity) && (mode != OperationMode::ProfileVelocity) && (mode != OperationMode::ProfileTorque) && (mode != OperationMode::InterpolatedPosition)) {
		throw(nanolib_exception("Can't halt this mode"));
		return EXIT_FAILURE;
	}
	//halt option: slow down ramp
	m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value(), 1, nlc::OdIndex(0x605D, 0x00), 16);

	int16_t word16 = static_cast<int16_t>(m_nanolibHelper->readInteger(m_connectedDeviceHandle->value(), nlc::OdIndex(0x605D, 0x00)));
	word16 |= (1 << 8);
	m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value(), word16, nlc::OdIndex(0x6040, 0x00), 16);
	return EXIT_SUCCESS;
}



