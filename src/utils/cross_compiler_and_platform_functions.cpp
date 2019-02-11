//
// file:		cross_compiler_and_platform_functions.cpp
// created on:	2018 Nov 26
// created by:	D. Kalantaryan (davit.kalantaryan@desy.de)
//

#include <stdarg.h>
#include <stdio.h>


#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#ifndef __SOCET_LIB_LOADED
#pragma comment(lib, "Ws2_32.lib")
#define __SOCET_LIB_LOADED
#endif //__SOCET_LIB_LOADED

#else
#endif


int vdprintf(int a_fd, const char * a_format, va_list a_ap)
{
	int nReturn;
	FILE* fpStream = _fdopen(a_fd, "a");

	if(!fpStream){return 0;}
	nReturn = vfprintf(fpStream, a_format, a_ap);
	fclose(fpStream);
	return nReturn;
}


#ifdef __cplusplus
}
#endif
