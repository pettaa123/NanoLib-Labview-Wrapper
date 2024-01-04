#include <chrono>
#include "autoSetupMotor.h"


int AutoSetupMotor::autoSetupMotPams() {

	if (m_powerSM->shutdown())
		return EXIT_FAILURE;

	//lese 6041h:00h(Bit 9(remote), 5(quick_stop) und 0(ready to switch on) = 1 ? )
	uint32_t uWord32 = static_cast<uint32_t>(m_nanolibHelper->readInteger(m_connectedDeviceHandle->value(), nlc::OdIndex(0x6041, 0x00)));
	if (uWord32 << 0 == 0 || uWord32 << 5 == 0 || uWord32 << 9 == 1) {
		throw(nanolib_exception("state either in remote,quickstop or not ready to switch on"));
		return EXIT_FAILURE;
	}

	//power sm to operation enabled
	if (m_powerSM->enableOperation())
		return EXIT_FAILURE;
	//start auto-setup
	uint16_t uWord16 = static_cast<uint16_t>(m_nanolibHelper->readInteger(m_connectedDeviceHandle->value(), nlc::OdIndex(0x6040, 0x00)));
	uWord16 |= 1UL << 4;
	m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value(), uWord16, nlc::OdIndex(0x6040, 0x00), 16);
	//wait till its done
	using namespace std::chrono_literals; // ns, us, ms, s, h, etc.

	uint16_t iterationsDone = 0;
	uint16_t maxIterations = 3000;

	do {
		uWord16 = static_cast<uint16_t>(
			m_nanolibHelper->readInteger(m_connectedDeviceHandle->value(), nlc::OdIndex(0x6041, 0x00)));
		DBOUT("Auto Setup running\n");
		std::this_thread::sleep_for(10ms);
		iterationsDone += 1;
		if (iterationsDone == maxIterations) {
			return EXIT_FAILURE;
		}
	} while (
		((uWord16 >> 12) & 1U) == 0);
	DBOUT("Auto Setup done\n");

	uWord16 = 0;
	m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value(), uWord16, nlc::OdIndex(0x6040, 0x00), 16);

	return EXIT_SUCCESS;
}