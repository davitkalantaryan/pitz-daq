/*
 *	File: pitz_daq_collectorinfo.hpp
 *
 *	Created on: 30 Jan 2017
 *	Created by: Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *  This file implements ...
 *
 */
#ifndef PITZ_DAQ_COLLECTORINFO_HPP
#define PITZ_DAQ_COLLECTORINFO_HPP

#define     CODEDACOLLECTORINFO  298	// eq_fct_type number for the .conf file
#define     INFO_LOCATION_NAME  "INFO._SVR"

#include	<eq_fct.h>

#include <string.h>

namespace pitz{namespace daq{

class D_int_for_debug_level : public D_int
{
public:
    D_int_for_debug_level(const char* propName);
    virtual ~D_int_for_debug_level();
protected:
    virtual void set (EqAdr* addr, EqData* fromUser, EqData* toUser, EqFct* fct);
    virtual void get (EqAdr* addr, EqData* fromUser, EqData* toUser, EqFct* fct);
};

class CollectorInfo : public EqFct
{
protected:
    CollectorInfo(std::string& a_loc_name);
    virtual ~CollectorInfo();

public:
    static CollectorInfo* CreateNew();
    static void DeleteInstance();

protected:
    virtual int	fct_code()	{ return CODEDACOLLECTORINFO; }
    void    init();

protected:
    static CollectorInfo*   m_spSingleInstance;
    static D_int            m_sDebugLevelProp;
    D_int                   m_rpc_number;
    D_string                m_host_name;

}; // class CollectorInfo

}// namespace daq
}  // namespace pitz


#if __cplusplus > 199711L
#include <thread>
#define thread_namespace std
#else  // #if __cplusplus > 199711L
  //#error This library needs at least a C++11 compliant compiler
#include <pthread.h>
#include <stddef.h>
namespace thread_namespace{
class thread{
    pthread_t m_pthread;
public:
    thread(void (*a_fpThreadFnc)(void));
    void detach();
};
}
#endif  // #if __cplusplus > 199711L


#endif // PITZ_DAQ_COLLECTORINFO_HPP
