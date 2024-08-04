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

namespace NanoLibWrapper {

	extern "C" NANOLIBDLL_API int32_t GetPortsLV(LStrArrayHdl * LVAllocatedStrArray);

	extern "C" NANOLIBDLL_API int32_t ScanBusLV(LStrArrayHdl * LVAllocatedStrArray);

	extern "C" NANOLIBDLL_API int32_t GetExceptions(std::vector<std::string> &exceptions);

	extern "C" NANOLIBDLL_API int32_t GetExceptionsLV(LStrArrayHdl * LVAllocatedStrArray);

	extern "C" NANOLIBDLL_API int32_t GetErrorStackLV(LStrArrayHdl * LVAllocatedStrArray);

	extern "C" NANOLIBDLL_API int32_t GetModeOfOperationLV(LStrHandle * LVAllocatedStr);

	extern "C" NANOLIBDLL_API int32_t GetCiA402StateLV(LStrHandle * LVAllocatedStr, LVBoolean * fault, LVBoolean * voltageEnabled, LVBoolean * quickStop, LVBoolean * warning, LVBoolean * targetReached, LVBoolean * limitReached, LVBoolean * bit12, LVBoolean * bit13);

	int32_t StdStrToLVStr(const std::string& s, LStrHandle* str);

	int32_t VecStrToLVStrArr(const std::vector<std::string>& s, LStrArrayHdl* arr);

	int32_t VecUint32ToLVuint32Arr(const std::vector<std::string>& s, LVuint32ArrayHdl* arr);

	//***GENERAL***


	extern "C" NANOLIBDLL_API int32_t RebootDevice();

	extern "C" NANOLIBDLL_API int32_t GetPorts(std::vector<std::string> &ports);

	extern "C" NANOLIBDLL_API int32_t GetUserUnits(uint32_t & feed, uint32_t & shaftRevs, uint32_t & posUnit, uint32_t & posExp, uint32_t & velUnit, uint32_t & velExp, uint32_t & velTime, uint32_t & gearRatioMotorRevs, uint32_t & gearRatioShaftRevs);

	extern "C" NANOLIBDLL_API int32_t SetUserUnits(uint32_t feed, uint32_t shaftRevs, uint32_t posUnit, uint32_t posExp, uint32_t velUnit, uint32_t velExp, uint32_t velTime, uint32_t gearRatioMotorRevs, uint32_t gearRatioShaftRevs);

	extern "C" NANOLIBDLL_API int32_t OpenPort(uint32_t portToOpen);

	extern "C" NANOLIBDLL_API int32_t ScanBus(std::vector<std::string> &ports);

	extern "C" NANOLIBDLL_API int32_t ConnectDevice(uint32_t deviceToOpen);

	extern "C" NANOLIBDLL_API int32_t DisconnectDevice();

	extern "C" NANOLIBDLL_API int32_t ClosePort();

	extern "C" NANOLIBDLL_API int32_t Home(uint32_t speedZero, uint32_t speedSwitch);

	extern "C" NANOLIBDLL_API int32_t GetFirmwareVersion(std::string & ver);

	extern "C" NANOLIBDLL_API int32_t GetFirmwareVersionLV(LStrHandle * ver);

	extern "C" NANOLIBDLL_API int32_t Halt();

	extern "C" NANOLIBDLL_API int32_t QuickStop();

	extern "C" NANOLIBDLL_API int32_t AutoSetupMotPams();

	extern "C" NANOLIBDLL_API int32_t SetMotorParameters(uint32_t polePairCount, uint32_t ratedCurrent, uint32_t maxCurrent, uint32_t maxCurrentDuration, uint32_t idleCurrent, uint32_t driveMode);

	extern "C" NANOLIBDLL_API int32_t GetMotorParameters(uint32_t & polePairCount, uint32_t & ratedCurrent, uint32_t & maxCurrent, uint32_t & maxCurrentTime, uint32_t & idleCurrent, uint32_t & driveMode);

	extern "C" NANOLIBDLL_API int32_t GetPositioningParameters(uint32_t & profileVelocity, int32_t & setTarget);

	extern "C" NANOLIBDLL_API int32_t SaveMotorParameters();

	extern "C" NANOLIBDLL_API int32_t SaveUserUnits();

	extern "C" NANOLIBDLL_API int32_t GetPositionActual(int32_t & pos);


	//***HOMING***


	extern "C" NANOLIBDLL_API int32_t Home(uint32_t speedZero, uint32_t speedSwitch);

	extern "C" NANOLIBDLL_API int32_t SetHomingAcceleration(uint32_t acc);


	//***VELOCITY

	extern "C" NANOLIBDLL_API int32_t StartVelocity();

	extern "C" NANOLIBDLL_API int32_t SetVelocityPams(int16_t vel, uint32_t deltaSpeedAcc, uint16_t deltaTimeAcc, uint32_t deltaSpeedDec, uint16_t deltaTimeDec);

	extern "C" NANOLIBDLL_API int32_t GetVelocityPams(int16_t & vel, uint32_t & deltaSpeedAcc, uint16_t & deltaTimeAcc, uint32_t & deltaSpeedDec, uint16_t & deltaTimeDec);

	extern "C" NANOLIBDLL_API int32_t GetVelocityStatus(int16_t & demandedSpeed, int16_t & speedActual);


	//***POSITIONING***

	extern "C" NANOLIBDLL_API int32_t SetTargetPosition(int32_t position, uint32_t absRel);

	extern "C" NANOLIBDLL_API int32_t SetProfileVelocity(uint32_t speed);

	extern "C" NANOLIBDLL_API int32_t StartPositioning();

	extern "C" NANOLIBDLL_API int32_t SetProfileAcceleration(uint32_t acc);

}