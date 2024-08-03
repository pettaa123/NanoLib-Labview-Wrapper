#pragma once
#include<vector>
#include<string>

#ifdef NANOLIBDLL_EXPORTS
#define NANOLIBDLL_API __declspec(dllexport)
#else
#define NANOLIBDLL_API __declspec(dllimport)
#endif


#include "fundtypes.h"
#include "extcode.h"

// This prolog and epilog should be used around any typedef that is passed from LabVIEW
// to a DLL for parameters configured as LabVIEW type or Adapt to Type
#include "lv_prolog.h"
#include "platdefines.h"
typedef struct
{
    int32_t dimSize;
    LStrHandle elt[1];
} LStrArrayRec, *LStrArrayPtr, **LStrArrayHdl;

typedef struct {
	int32_t dimSize;
	uint32_t elt[1];
} LVuint32Array;
typedef LVuint32Array** LVuint32ArrayHdl;

#include "lv_epilog.h"

#if IsOpSystem64Bit
#define uPtr uQ //unsigned Quad aka 64-bit
#else
#define uPtr uL //unsigned Long aka 32-bit
#endif

extern "C" NANOLIBDLL_API int32_t getPortsLV(LStrArrayHdl * LVAllocatedStrArray);

extern "C" NANOLIBDLL_API int32_t scanBusLV(LStrArrayHdl * LVAllocatedStrArray);

extern "C" NANOLIBDLL_API int32_t getExceptions(std::vector<std::string> &exceptions);

extern "C" NANOLIBDLL_API int32_t getExceptionsLV(LStrArrayHdl * LVAllocatedStrArray);

extern "C" NANOLIBDLL_API int32_t getErrorStackLV(LStrArrayHdl * LVAllocatedStrArray);

extern "C" NANOLIBDLL_API int32_t getModeOfOperationLV(LStrHandle * LVAllocatedStr);

extern "C" NANOLIBDLL_API int32_t getCiA402StateLV(LStrHandle *LVAllocatedStr, LVBoolean *fault, LVBoolean *voltageEnabled, LVBoolean *quickStop, LVBoolean *warning, LVBoolean *targetReached, LVBoolean *limitReached, LVBoolean *bit12, LVBoolean *bit13);

int32_t stdStrToLVStr(const std::string& s, LStrHandle * str);

int32_t vecStrToLVStrArr(const std::vector<std::string>& s, LStrArrayHdl* arr);

int32_t vecUint32ToLVuint32Arr(const std::vector<std::string>& s, LVuint32ArrayHdl* arr);

//***GENERAL***


extern "C" NANOLIBDLL_API int32_t rebootDevice();

extern "C" NANOLIBDLL_API int32_t getPorts(std::vector<std::string> &ports);

extern "C" NANOLIBDLL_API int32_t getUserUnits(uint32_t & feed, uint32_t &shaftRevs, uint32_t &posUnit, uint32_t &posExp, uint32_t &velUnit, uint32_t &velExp, uint32_t &velTime, uint32_t &gearRatioMotorRevs, uint32_t &gearRatioShaftRevs);

extern "C" NANOLIBDLL_API int32_t setUserUnits(uint32_t feed, uint32_t shaftRevs, uint32_t posUnit, uint32_t posExp, uint32_t velUnit, uint32_t velExp, uint32_t velTime, uint32_t gearRatioMotorRevs, uint32_t gearRatioShaftRevs);

extern "C" NANOLIBDLL_API int32_t openPort(uint32_t portToOpen);

extern "C" NANOLIBDLL_API int32_t scanBus(std::vector<std::string> &ports);

extern "C" NANOLIBDLL_API int32_t connectDevice(uint32_t deviceToOpen);

extern "C" NANOLIBDLL_API int32_t disconnectDevice();

extern "C" NANOLIBDLL_API int32_t closePort();

extern "C" NANOLIBDLL_API int32_t home(uint32_t speedZero, uint32_t speedSwitch);

extern "C" NANOLIBDLL_API int32_t getFirmwareVersion(std::string & ver);

extern "C" NANOLIBDLL_API int32_t getFirmwareVersionLV(LStrHandle * ver);

extern "C" NANOLIBDLL_API int32_t halt();

extern "C" NANOLIBDLL_API int32_t quickStop();

extern "C" NANOLIBDLL_API int32_t autoSetupMotPams();

extern "C" NANOLIBDLL_API int32_t setMotorParameters(uint32_t polePairCount,uint32_t ratedCurrent, uint32_t maxCurrent, uint32_t maxCurrentDuration, uint32_t idleCurrent,uint32_t driveMode);

extern "C" NANOLIBDLL_API int32_t getMotorParameters(uint32_t & polePairCount, uint32_t & ratedCurrent, uint32_t & maxCurrent, uint32_t & maxCurrentTime, uint32_t &idleCurrent, uint32_t & driveMode);

extern "C" NANOLIBDLL_API int32_t getPositioningParameters(uint32_t &profileVelocity, int32_t &setTarget);

extern "C" NANOLIBDLL_API int32_t saveMotorParameters();

extern "C" NANOLIBDLL_API int32_t saveUserUnits();

extern "C" NANOLIBDLL_API int32_t getPositionActual(int32_t &pos);


//***HOMING***


extern "C" NANOLIBDLL_API int32_t home(uint32_t speedZero, uint32_t speedSwitch);

extern "C" NANOLIBDLL_API int32_t setHomingAcceleration(uint32_t acc);


//***VELOCITY

extern "C" NANOLIBDLL_API int32_t startVelocity();

extern "C" NANOLIBDLL_API int32_t setVelocityPams(int16_t vel, uint32_t deltaSpeedAcc, uint16_t deltaTimeAcc, uint32_t deltaSpeedDec, uint16_t deltaTimeDec);

extern "C" NANOLIBDLL_API int32_t getVelocityPams(int16_t &vel, uint32_t &deltaSpeedAcc, uint16_t &deltaTimeAcc, uint32_t &deltaSpeedDec, uint16_t &deltaTimeDec);

extern "C" NANOLIBDLL_API int32_t getVelocityStatus(int16_t &demandedSpeed, int16_t & speedActual);


//***POSITIONING***

extern "C" NANOLIBDLL_API int32_t setTargetPosition(int32_t position, uint32_t absRel);

extern "C" NANOLIBDLL_API int32_t setProfileVelocity(uint32_t speed);

extern "C" NANOLIBDLL_API int32_t startPositioning();

extern "C" NANOLIBDLL_API int32_t setProfileAcceleration(uint32_t acc);

