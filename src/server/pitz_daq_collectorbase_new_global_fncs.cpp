/*
 *	File: pitz_daq_collectorbase.cpp
 *
 *	Created on: 30 Jan 2017
 *	Created by: Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *  This file implements ...
 *
 */
#include "pitz_daq_collectorbase.hpp"
#include "pitz_daq_collectorinfo.hpp"
#include <string.h>
#include "pitz_daq_globalfunctionsiniter.hpp"
#include "common_daq_definations.h"

#ifdef WIN32
#include <windows.h>
#else
#ifndef Sleep
#include <unistd.h>
#define Sleep(__x__) usleep(1000*(__x__))
#endif
#endif

pitz::daq::GlobalFunctionsIniter*    s_pGlobalFunctionsClass;


/*////////////////////////////////////////////////////////////////////////*/

void eq_init_prolog() // called once before init of all EqFct's
{
    s_pGlobalFunctionsClass = CreateGlobalFunctionsMember();
    Sleep(61000);
    __DEBUG_APP__(1,"s_pGlobalFunctionsClass=0x%p ",s_pGlobalFunctionsClass);
    if(!s_pGlobalFunctionsClass){throw "low memory\n" __FILE__ ;}
    s_pGlobalFunctionsClass->eq_init_prolog();
}

void eq_cancel()
{
    s_pGlobalFunctionsClass->eq_cancel();
    __DEBUG_APP__(2," ");
}


void interrupt_usr1_prolog()
{
    __DEBUG_APP__(2," ");
    s_pGlobalFunctionsClass->interrupt_usr1_prolog();
}


void eq_init_epilog()
{
    __DEBUG_APP__(2," ");
    s_pGlobalFunctionsClass->eq_init_epilog();
}


void post_init_prolog()
{
    __DEBUG_APP__(2," ");
    s_pGlobalFunctionsClass->post_init_prolog();
}


void refresh_prolog()
{
    __DEBUG_APP__(2," ");
    s_pGlobalFunctionsClass->refresh_prolog();
}

void refresh_epilog()
{
    __DEBUG_APP__(2," ");
    s_pGlobalFunctionsClass->refresh_epilog();
}


void post_init_epilog()
{
    __DEBUG_APP__(2," ");
    s_pGlobalFunctionsClass->post_init_epilog();
}

void interrupt_usr1_epilog()
{
    __DEBUG_APP__(2," ");
    s_pGlobalFunctionsClass->interrupt_usr1_epilog();
}


void interrupt_usr1_prolog(int a_val)
{
    __DEBUG_APP__(2," ");
    s_pGlobalFunctionsClass->interrupt_usr1_prolog(a_val);
}

void interrupt_usr1_epilog(int a_val)
{
    __DEBUG_APP__(2," ");
    s_pGlobalFunctionsClass->interrupt_usr1_epilog(a_val);
}
