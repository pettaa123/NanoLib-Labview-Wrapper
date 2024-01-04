#pragma once

#include "motor.h"

/* Motor in Velocity Mode
 special function: Bit 8 in 6040h halt
 
*/
class VelocityMotor : public Motor402 {

public:
    VelocityMotor(NanoLibHelper *nanolibHelper, std::optional<nlc::DeviceHandle> *connectedDeviceHandle, PowerSM *powerSM) : 
        Motor402(nanolibHelper, connectedDeviceHandle,powerSM)
    {
        setModeOfOperation(OperationMode::Velocity);
    }

    // Start the motor movement with specified speed
    int startVelocity()  {
        
        //Limit Switch Error Option Code
        /*
        Abbremsen mit quick stop ramp und anschließendem
        Zustandswechsel in Switch on disabled
        */
        m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value(), 2, nlc::OdIndex(0x3701, 0x00), 16);

        uint16_t uWord16 = static_cast<uint16_t>(m_nanolibHelper->readInteger(m_connectedDeviceHandle->value(), nlc::OdIndex(0x6040, 0x00)));
        //reset halt bit
        uWord16 &= ~(1U << 8);
        m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value(), uWord16, nlc::OdIndex(0x6040, 0x00), 16);

        //power sm to ready tp switch on
        if (m_powerSM->enableOperation())
            return EXIT_FAILURE;

        return EXIT_SUCCESS;
    }

    //velocity in user defined units
    void setTargetVelocity(int16_t vel) {
        // target velocity in user units
        m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value(), vel, nlc::OdIndex(0x6042, 0x00), 16);
    }

    void getTargetVelocity(int16_t &vel) {
        // target velocity in user units
        vel = static_cast<int16_t>(m_nanolibHelper->readInteger(m_connectedDeviceHandle->value(), nlc::OdIndex(0x6042, 0x00)));
    }

    void setVelocityAcceleration(uint32_t deltaSpeed, uint16_t deltaTime) {
        // target velocity in user units
        m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value(), deltaSpeed, nlc::OdIndex(0x6048, 0x01), 16);
        m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value(), deltaTime, nlc::OdIndex(0x6048, 0x02), 16);
    }

    void getVelocityAcceleration(uint32_t &deltaSpeed, uint16_t &deltaTime) {
        // target velocity in user units
        deltaSpeed = static_cast<int16_t>(m_nanolibHelper->readInteger(m_connectedDeviceHandle->value(), nlc::OdIndex(0x6048, 0x01)));
        deltaTime = static_cast<int16_t>(m_nanolibHelper->readInteger(m_connectedDeviceHandle->value(), nlc::OdIndex(0x6048, 0x02)));
    }

    void setVelocityDeceleration(uint32_t deltaSpeed, uint16_t deltaTime) {
        // target velocity in user units
        m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value(), deltaSpeed, nlc::OdIndex(0x6049, 0x01), 16);
        m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value(), deltaTime, nlc::OdIndex(0x6049, 0x02), 16);
    }

    void getVelocityDeceleration(uint32_t& deltaSpeed, uint16_t& deltaTime) {
        // target velocity in user units
        deltaSpeed = static_cast<int16_t>(m_nanolibHelper->readInteger(m_connectedDeviceHandle->value(), nlc::OdIndex(0x6049, 0x01)));
        deltaTime = static_cast<int16_t>(m_nanolibHelper->readInteger(m_connectedDeviceHandle->value(), nlc::OdIndex(0x6049, 0x02)));
    }

    void getUserUnitsVelocity(uint32_t& unit, uint32_t& exp, uint32_t& time) {

        uint32_t val = static_cast<uint32_t>(m_nanolibHelper->readInteger(m_connectedDeviceHandle->value(), nlc::OdIndex(0x60A9, 0x00)));

        exp = (val >> 24) & 0xff;
        unit = (val >> 16) & 0xff;
        time = (val >> 8) & 0xff;
    }

    int setUserUnitsVelocity(uint32_t velUnit, uint32_t velExp, uint32_t velTime) {

        //make sure operation is disabled before changing user defined units
        if (m_powerSM->disableOperation())
            return EXIT_FAILURE;

        uint32_t val = static_cast<uint32_t>(m_nanolibHelper->readInteger(m_connectedDeviceHandle->value(), nlc::OdIndex(0x60A9, 0x00)));


        uint32_t time = (velTime << 8);
        uint32_t unit = (velUnit << 16);
        uint32_t exp = (velExp << 24);

        //reset
        val = val & ~0xFFFFFF00;
        //set
        val = (((val |= exp) |= unit) |= time);

        m_nanolibHelper->writeInteger(m_connectedDeviceHandle->value(), val, nlc::OdIndex(0x60A9, 0x00), 32);

        return EXIT_SUCCESS;
    }

private:

    uint16_t getState() {
        return EXIT_SUCCESS;
    }

};