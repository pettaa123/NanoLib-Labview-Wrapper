#pragma once

#ifdef NANOLIBDLL_EXPORTS
#define NANOLIBDLL_API __declspec(dllexport)
#else
#define NANOLIBDLL_API __declspec(dllimport)
#endif

extern "C" NANOLIBDLL_API int getPorts();


