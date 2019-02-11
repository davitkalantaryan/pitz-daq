/*
 *	File: pitz_daq_collectorinfo.cpp
 *
 *	Created on: 30 Jan 2017
 *	Created by: Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *  This file implements ...
 *
 */
#include "pitz_daq_collectorinfo.hpp"
#include "common_daq_definations.h"

pitz::daq::D_int_for_debug_level::D_int_for_debug_level(const char* a_cpcPropName)
    :
      D_int(a_cpcPropName,NULL)
{
    //
}


pitz::daq::D_int_for_debug_level::~D_int_for_debug_level()
{
    //
}

void pitz::daq::D_int_for_debug_level::set (EqAdr* /*addr*/, EqData* a_fromUser, EqData* /*toUser*/, EqFct* /*fct*/)
{
    g_nDebugDaqApplication = a_fromUser->get_int();
    set_value(g_nDebugDaqApplication);
}

void pitz::daq::D_int_for_debug_level::get (EqAdr* /*addr*/, EqData* /*fromUser*/, EqData* a_toUser, EqFct* /*fct*/)
{
    a_toUser->set(g_nDebugDaqApplication);
}

/*//////////////////////////////////////////////////////////////////////////////////////*/
D_int pitz::daq::CollectorInfo::m_sDebugLevelProp("DEBUG.LEVEL",NULL);


pitz::daq::CollectorInfo* pitz::daq::CollectorInfo::m_spSingleInstance = NULL;


pitz::daq::CollectorInfo::CollectorInfo(std::string& a_loc_name)
        :
            EqFct("NAME = location",&a_loc_name ),
            m_rpc_number("RPC.NUMBER",this),
            m_host_name("HOSTNAME",this)
{
    prop_reg(&m_sDebugLevelProp);
}


pitz::daq::CollectorInfo::~CollectorInfo()
{
    m_spSingleInstance = NULL;
}


void  pitz::daq::CollectorInfo::init()
{
    g_nDebugDaqApplication = m_sDebugLevelProp.value();
}


pitz::daq::CollectorInfo* pitz::daq::CollectorInfo::CreateNew()
{
    // Should be some syncronization during check
    // Should be implemented later
    CollectorInfo* pInstance = NULL;
    if(m_spSingleInstance==NULL)
    {
        std::string aLocNme(INFO_LOCATION_NAME);
        m_spSingleInstance = (pitz::daq::CollectorInfo*)1;
        pInstance = new CollectorInfo(aLocNme);
        if(!pInstance)
        {
            m_spSingleInstance = NULL;
            throw "Low memory!";
        }
        m_spSingleInstance = pInstance;
    }

    return pInstance;
}


void pitz::daq::CollectorInfo::DeleteInstance()
{
    CollectorInfo* pInstance = m_spSingleInstance;
    if(m_spSingleInstance)
    {
        m_spSingleInstance = NULL;
        delete pInstance;
    }
}
