/*
 *  Copyright (C) DESY For details refers to LICENSE.md
 *
 *  Written by Davit Kalantaryan <davit.kalantaryan@desy.de>
 *
 */

/**
 *
 * @file       mex_daq_browser2_common.h
 * @copyright  Copyright (C) DESY For details refers to LICENSE.md
 * @brief      Header for providing API to comunicate with extensions
 * @author     Davit Kalantaryan <davit.kalantaryan@desy.de>
 * @date       2019 Sep 3
 * @details
 *   Include file declars symbols necessary for extending MATLAB emulator (method 1) \n
 *   No library is needed in order to use functionalities described in this header
 *
 */

#ifndef PITZ_DAQ_MEX_DATA_BROWSER_COMMON_H
#define PITZ_DAQ_MEX_DATA_BROWSER_COMMON_H


#include <matrix.h>
#ifdef printf
#undef printf
#endif
#ifdef vprintf
#undef vprintf
#endif
#ifdef vfprintf
#undef vfprintf
#endif
#ifdef fprintf
#undef fprintf
#endif
#include <stdio.h>


#endif   // #ifndef PITZ_DAQ_MEX_DATA_BROWSER_COMMON_H
