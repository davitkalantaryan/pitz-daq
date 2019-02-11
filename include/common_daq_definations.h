// common_daq_definations
/*
 *	File: common_daq_definations.h
 *
 *	Created on: 02 Feb 2017
 *	Created by: Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *  This file implements ...
 *
 */
#ifndef COMMON_DAQ_DEFINATIONS_H
#define COMMON_DAQ_DEFINATIONS_H

#if defined(_MSC_VER) & (_MSC_VER>=1913)

#ifndef CINTERFACE
#define CINTERFACE
#endif
#ifndef __msxml_h__
//#define __msxml_h__
#endif
#ifndef __urlmon_h__
//#define __urlmon_h__
#endif
#ifndef COM_NO_WINDOWS_H
#define COM_NO_WINDOWS_H
#endif

#endif  // #if defined(_MSC_VER) & (_MSC_VER>=1913)


#ifndef __SOURCE_FILE__
#define __SOURCE_FILE__ strrchr(__FILE__,'/') ? (strrchr(__FILE__,'/')+1) : \
    (strrchr(__FILE__,'\\') ? (strrchr(__FILE__,'\\')+1) : __FILE__)
#endif // #ifndef __SOURCE_FILE__


#ifdef _MSC_VER

#ifndef STDOUT_FILENO
#define STDOUT_FILENO _fileno(stdout) 
#endif  // #ifndef STDOUT_FILENO
#ifndef STDERR_FILENO
#define STDERR_FILENO _fileno(stderr) 
#endif  // #ifndef STDERR_FILENO

#endif  // #ifdef _MSC_VER


#ifdef _WIN32

#if !defined(pthread_t_defined) && !defined(pthread_t)
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#define pthread_t_defined
typedef HANDLE pthread_t;
#endif  // #if !defined(pthread_t_defined) && !defined(pthread_t)
#ifdef getpid
#undef getpid
#endif
#define getpid	GetCurrentProcessId
#define pthread_self GetCurrentThread
#define SleepMsInt(_timeMs)	SleepEx((_timeMs),TRUE)

#else    // #ifdef _WIN32

#include <unistd.h>
#define SleepMsInt(_timeMs) \
	do{ \
		sleep((_timeMs)/1000);usleep(1000*((_timeMs)%1000)); \
	}while(0)

#endif   // #ifdef _WIN32

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>


#ifdef _MSC_VER

#ifdef __cplusplus
extern "C" {
#endif

	int vdprintf(int fd, const char *format, va_list ap);

#ifdef __cplusplus
}
#endif


#endif


#endif // COMMON_DAQ_DEFINATIONS_H
