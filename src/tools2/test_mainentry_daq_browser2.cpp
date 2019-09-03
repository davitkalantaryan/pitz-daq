/*
 *  Copyright (C) DESY For details refers to LICENSE.md
 *
 *  Written by Davit Kalantaryan <davit.kalantaryan@desy.de>
 *
 */

/**
 *
 * @file       mainentry_daq_browser2.c
 * @copyright  Copyright (C) DESY For details refers to LICENSE.md
 * @brief      Header for providing API to comunicate with extensions
 * @author     Davit Kalantaryan <davit.kalantaryan@desy.de>
 * @date       2019 Sep 3
 * @details
 *   Include file declars symbols necessary for extending MATLAB emulator (method 1) \n
 *   No library is needed in order to use functionalities described in this header
 *
 */

#include <mex_daq_browser2_common.h>
#include "daq_root_reader.hpp"
#include <string.h>
#include <utility>

#ifndef HANDLE_MEM_DEF
#define HANDLE_MEM_DEF(...)
#endif


int main(int a_argc, char* a_argv[])
{
    printf("argc=%d, argv=%p\n",a_argc,static_cast<void*>(a_argv));

    //if(a_argc>1){
    //    mxArray* pArray = nullptr;
    //    pitz::daq::RootInitialize();
    //    pArray = MatlabGetMultipleBranchesForTimeInterval2(a_argc,a_argv);
    //    printf("pArray=%p\n",static_cast<void*>(pArray));
    //    pitz::daq::RootCleanup();
    //}

    return 0;
}
