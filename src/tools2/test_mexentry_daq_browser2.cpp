/*
 *  Copyright (C) DESY For details refers to LICENSE.md
 *
 *  Written by Davit Kalantaryan <davit.kalantaryan@desy.de>
 *
 */

/**
 *
 * @file       mexentry_daq_browser2.c
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
#include <string.h>
#include <stdarg.h>
#include <dlfcn.h>

//void mexFunction (int a_nNumOuts, mxArray *a_Outputs[],int a_nNumInps, const mxArray*a_Inputs[]) __attribute__ ((unused));
extern "C" int mexPrintf(const char	*/* printf style format */,.../* any additional arguments */)  __attribute__((weak));

extern "C"  void mexFunction(int a_nNumOuts, mxArray *a_Outputs[],int a_nNumInps, const mxArray*a_Inputs[])
{
    mexPrintf("version: 9. ni=%d, inps=%p, no=%d, outs=%p\n",a_nNumInps,a_Inputs,a_nNumOuts,a_Outputs);

}


extern "C"{

typedef int (*TypemexPrintf)(const char *a_fmt,...);

int mexPrintf(const char *a_fmt,...)
{
    int nReturn;
    va_list argList;
    TypemexPrintf fpFunct1 = reinterpret_cast<TypemexPrintf>(dlsym(RTLD_NEXT,"mexPrintf"));

    va_start(argList,a_fmt);
    if(fpFunct1){
        char vcBuffer[4096];
        vsnprintf(vcBuffer,4095,a_fmt,argList);
        nReturn = (*fpFunct1)(a_fmt);
    }
    else{
        nReturn = vprintf(a_fmt,argList);
    }
    va_end(argList);


    return nReturn;
}

}  // extern "C"{
