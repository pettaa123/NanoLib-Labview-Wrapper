#pragma once

#include "fundtypes.h"

#ifdef NANOLIBDLL_EXPORTS
#define NANOLIBDLL_API __declspec(dllexport)
#else
#define NANOLIBDLL_API __declspec(dllimport)
#endif

/// @brief Long Pascal-style string types.
typedef struct
{
    int32 cnt; // number of bytes that follow
    uChar str[1]; // cnt bytes
} LStr, * LStrPtr, ** LStrHandle;
typedef LStr const* ConstLStrP;
typedef LStr const* const* ConstLStrH;

typedef struct
{
    int32 dimSize;
    LStrHandle String[1];
} LStrHandleArrayBase;
typedef LStrHandleArrayBase** LStrHandleArray;

extern "C" NANOLIBDLL_API int getPorts(char **list, int rows, int cols);

extern "C" NANOLIBDLL_API int getPortsLV(LStrHandleArray list, int rows, int cols);


