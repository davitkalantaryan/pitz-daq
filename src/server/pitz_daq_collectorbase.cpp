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

int g_nDebugDaqApplication = 1;
extern int     build_phase; // defined in (eq_server.cc, ln:~225)

extern EqFct* eq_create_derived(int eq_code, void* arg);


namespace pitz{namespace daq{

static volatile int s_nRun = 0;
static thread_namespace::thread* s_pThread = NULL;

static void thread_to_create_info_location(void)
{  
    __DEBUG_APP__(2,"!!!!!!!!!!!!!!!!!!!Thread started!");

    while(s_nRun>1)
    {
        if(build_phase)
        {
            add_location(CODEDACOLLECTORINFO,INFO_LOCATION_NAME);
            s_nRun = 0;
            return;
        }
        Sleep(100);
    }
    s_nRun = 0;
    __DEBUG_APP__(1,"!!!!!!!!!!!!!!!!!!!Thread done!");
}


static int InitDaqCollectorLib(void)
{
    int nRun(s_nRun++);
    __DEBUG_APP__(2,"nRun=%d!",nRun);

    if(nRun==0)
    {
        s_nRun = 2;
        s_pThread = new thread_namespace::thread(thread_to_create_info_location);
        if(!s_pThread){return -1;}
        s_pThread->detach();
        delete s_pThread;
    }

    return 0;

}


void DestroyDaqCollectorLib(void)
{
    int nRun(s_nRun++);
    __DEBUG_APP__(2,"nRun=%d!",nRun);
    if(nRun != 3){return;}
    s_nRun = 1;
    while(s_nRun != 0){Sleep(10);}
}

}}


pitz::daq::CollectorBase::CollectorBase()
        :
            EqFct("NAME = location" )
{
    InitDaqCollectorLib();
}


pitz::daq::CollectorBase::~CollectorBase()
{
    DestroyDaqCollectorLib();
}



EqFct* eq_create(int a_eq_code, void* a_pArg)
{
    __DEBUG_APP__(1,"a_eq_code=%d, a_pArg=%p\n",a_eq_code, a_pArg);
    EqFct* eqn = NULL;

    switch (a_eq_code)
    {
    case CODEDACOLLECTORINFO:
        eqn = pitz::daq::CollectorInfo::CreateNew();
        break;

    default:
        eqn = eq_create_derived(a_eq_code,a_pArg);
        //if(eqn==NULL){eqn = s_pGlobalFunctionsClass->eq_create(a_eq_code,a_pArg);}
        break;
    }
    __DEBUG_APP__(1,"a_eq_code=%d, build_phase=%d, eqn=%p  !!!!!!!!!!!!!!!!!!!!!",a_eq_code,build_phase,eqn);
    return eqn;
}




/*///////////////////////////////////////////////////////////////////////////*/
#if __cplusplus > 199711L
#else
  //#error This library needs at least a C++11 compliant compiler
#include <stdarg.h>
namespace thread_namespace{

static void* thread_func_static(void* a_arg){
    void (*fpThreadFnc)(void) = (void (*)(void))a_arg;
    (*fpThreadFnc)();
    return NULL;
}

typedef void* TypeVoidPtr;

static void* FncPtr(int a_a,...){
    void* pFunc;
    va_list fpFunc;
    va_start( fpFunc,a_a );
    pFunc = va_arg(fpFunc, TypeVoidPtr);
    va_end(fpFunc);
    return pFunc;
}

thread::thread(void (*a_fpThreadFnc)(void)){
    pthread_create(&m_pthread,NULL,thread_func_static,FncPtr(1,a_fpThreadFnc));
}

void thread::detach(){pthread_detach(m_pthread);}

} // namespace thread_namespace
#endif  // #if __cplusplus > 199711L
