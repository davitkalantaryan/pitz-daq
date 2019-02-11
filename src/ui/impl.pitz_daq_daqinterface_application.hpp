/*
 *	File      : impl.pitz_daq_daqinterface_application.hpp
 *
 *	Created on: 22 Mar, 2017
 *	Author    : Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *
 */

#ifndef IMPL_PITZ_DAQ_DAQINTERFACE_APPLICATION_HPP
#define IMPL_PITZ_DAQ_DAQINTERFACE_APPLICATION_HPP

#ifndef PITZ_DAQ_DAQINTERFACE_APPLICATION_HPP
#include "pitz_daq_daqinterface_application.hpp"
#error do not include this file directly
#endif

template <typename ClsType>
void pitz::daq::daqinterface::Application::SetNewTask(void(ClsType::*fpClb)(SCallArgsAll,int64_t err),ClsType* owner,SCallArgsAll a_args)
{
    SetNewTask((TypeCallback1)FUNCTION_POINTER_TO_VOID_POINTER(fpClb),(void*)owner,a_args);
}


template <typename ClsType>
void pitz::daq::daqinterface::Application::IterateOverEntries( ClsType* a_pOwner, void(ClsType::*a_fpClb)(SCollemtorItem*& ) )
{
    ::common::NewSharedLockGuard< ::STDN::shared_mutex > aSharedGuard;

    aSharedGuard.SetAndLockMutex(&m_mutexForEntries);

    m_listAndHash.IterateOverEntries(a_pOwner, a_fpClb);

    aSharedGuard.UnsetAndUnlockMutex();
}

#endif // IMPL_PITZ_DAQ_DAQINTERFACE_APPLICATION_HPP
