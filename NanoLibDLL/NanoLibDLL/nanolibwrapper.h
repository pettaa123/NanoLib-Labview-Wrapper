#pragma once
#include<vector>
#include<string>

#include "fundtypes.h"
#include "extcode.h"

#ifdef NANOLIBDLL_EXPORTS
#define NANOLIBDLL_API __declspec(dllexport)
#else
#define NANOLIBDLL_API __declspec(dllimport)
#endif


// This prolog and epilog should be used around any typedef that is passed from LabVIEW
// to a DLL for parameters configured as LabVIEW type or Adapt to Type
#include "lv_prolog.h"
typedef struct
{
    int32 dimSize;
    LStrHandle elt[1];
} LStrArrayRec, *LStrArrayPtr, **LStrArrayHdl;
#include "lv_epilog.h"

#if IsOpSystem64Bit
#define uPtr uQ //unsigned Quad aka 64-bit
#else
#define uPtr uL //unsigned Long aka 32-bit
#endif


extern "C" NANOLIBDLL_API int getPorts(char **list, int rows, int cols);

extern "C" NANOLIBDLL_API MgErr getPortsLV(void **controllerPtr,LStrArrayHdl *LVAllocatedStrArray);

extern "C" NANOLIBDLL_API MgErr openPort(void **controllerPtr, unsigned int portToOpen);

extern "C" NANOLIBDLL_API MgErr scanBusLV(void** controllerPtr, LStrArrayHdl * LVAllocatedStrArray);

extern "C" NANOLIBDLL_API MgErr connectDevice(void** controllerPtr, unsigned int deviceToOpen);

extern "C" NANOLIBDLL_API MgErr autoSetupMotPams(void** controllerPtr);

extern "C" NANOLIBDLL_API MgErr home(void** controllerPtr);

extern "C" NANOLIBDLL_API MgErr moveToDeciMM(void** controllerPtr, int deciMM);

extern "C" NANOLIBDLL_API MgErr closePortLV(void **controllerPtr);

MgErr vecStrToLVStrArr(const std::vector<std::string> &s, LStrArrayHdl* arr);


