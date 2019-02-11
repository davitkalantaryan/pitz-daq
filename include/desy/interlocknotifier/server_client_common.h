//
// file:		server_client_common.h
// created on:	2018 Jun 21
//

#ifndef __desy_interlocknotifier_server_client_common_h__
#define __desy_interlocknotifier_server_client_common_h__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "driver/i4/hal/daq.h"

#ifdef LOG_LEVEL_DECLARED
extern int g_nLogLevel;
#endif

#ifdef _WIN32
#define exit _exit
#else
#include <unistd.h>
#define Sleep(_x)	usleep(1000*(_x))
#endif

#define INTERLOCK_NOTIFIER_SERVER_PORT	10030

#define HANDLE_MEMORY_DEF(_memPointer)	do{if(!(_memPointer)){exit(1);}}while(0)

#ifndef _PATH_DELIM
#ifdef _WIN32
#define _PATH_DELIM	'\\'
#else
#define _PATH_DELIM	'/'
#endif
#endif


#define _FILE_FROM_PATH(__file) ( strrchr((__file),_PATH_DELIM) ? (strrchr((__file),_PATH_DELIM)+1) : (__file)   )
#define DEBUG_APP_PRINT_ROW(__logLevel,...)  do{if((__logLevel)<=g_nLogLevel){printf(__VA_ARGS__);}}while(0)
#define DEBUG_APP_PRINT_PLACE(__logLevel)  DEBUG_APP_PRINT_ROW((__logLevel),"fl:%s,ln:%d: ",_FILE_FROM_PATH(__FILE__),__LINE__)
#define DEBUG_APP_PRINT_LINE_WITH_PLACE(__logLevel,...)  \
	do{ \
		DEBUG_APP_PRINT_PLACE((__logLevel)); \
		DEBUG_APP_PRINT_ROW((__logLevel),__VA_ARGS__); \
	}while(0)
#define DEBUG_APP(__logLevel,...)  \
	do{ \
		DEBUG_APP_PRINT_PLACE((__logLevel)); \
		DEBUG_APP_PRINT_ROW((__logLevel),__VA_ARGS__); \
		DEBUG_APP_PRINT_ROW((__logLevel),"\n"); \
	}while(0)


#endif // #ifndef __desy_interlocknotifier_server_client_common_h__
