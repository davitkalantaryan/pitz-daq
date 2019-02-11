/*
 *	File: pitz_daq_globalfunctionsiniter.hpp
 *
 *	Created on: 02 Feb 2017
 *	Created by: Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *  This file implements ...
 *
 */
#ifndef PITZ_DAQ_GLOBALFUNCTIONSINITER_HPP
#define PITZ_DAQ_GLOBALFUNCTIONSINITER_HPP

#define USE_H_STRUCT

#include	<eq_fct.h>

namespace pitz{ namespace daq{

#ifdef USE_H_STRUCT
typedef struct  H_struct
{
  int   seconds;
  int   microseconds;
  int   gen_event;
  int   rep_rate;
}H_struct;

extern H_struct * g_shareptr;
#endif // #ifdef USE_H_STRUCT

class GlobalFunctionsIniter
{
public:
    virtual ~GlobalFunctionsIniter(){}

    virtual void eq_init_prolog();
    virtual void eq_cancel(){}
    virtual void interrupt_usr1_prolog(){}
    virtual void eq_init_epilog(){}
    virtual void post_init_prolog(){}
    virtual void refresh_prolog(){}
    virtual void refresh_epilog(){}
    virtual void post_init_epilog(){}
    virtual void interrupt_usr1_epilog(){}
    virtual void interrupt_usr1_prolog(int /*val*/){}
    virtual void interrupt_usr1_epilog(int /*val*/){}
};

}}

pitz::daq::GlobalFunctionsIniter*    CreateGlobalFunctionsMember();

#endif // PITZ_DAQ_GLOBALFUNCTIONSINITER_HPP
