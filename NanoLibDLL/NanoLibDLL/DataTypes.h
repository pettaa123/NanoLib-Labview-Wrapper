/*MIT License

Copyright(c) 2022 David Lafreniere

Permission is hereby granted, free of charge, to any person obtaining a copy
of this softwareand associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright noticeand this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#ifndef _DATA_TYPES_H
#define _DATA_TYPES_H

#include "windows.h"

	typedef signed char INT8;
	typedef unsigned char UINT8;
	typedef signed short INT16;
	typedef unsigned short UINT16;
	typedef unsigned int UINT32;
	typedef int INT32;
	typedef char CHAR;
	typedef short SHORT;
	typedef long LONG;
	typedef int INT;
	typedef unsigned int UINT;
	typedef unsigned long DWORD;
	typedef unsigned char BYTE;
	typedef unsigned short WORD;
	typedef float FLOAT;
	typedef double DOUBLE;
	typedef int BOOL;

	#ifndef NULL
	#ifdef __cplusplus
	#define NULL    0
	#else
	#define NULL    ((void *)0)
	#endif
	#endif

	#ifndef FALSE
	#define FALSE               0
	#endif

	#ifndef TRUE
	#define TRUE                1
	#endif
#endif


