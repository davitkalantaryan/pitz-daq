
#ifndef PITZ_DAQ_EQFCTCOLLECTOR_IMPL_HPP
#define PITZ_DAQ_EQFCTCOLLECTOR_IMPL_HPP

#ifndef PITZ_DAQ_EQFCTCOLLECTOR_HPP
//#error Do not include this file directly
#include "pitz_daq_eqfctcollector.hpp"
#endif

template< typename TypeMutex >
pitz::daq::NewLockGuard<TypeMutex>::NewLockGuard(TypeMutex* a_pMutex)
{
    m_pMutex = nullptr;
    Lock(a_pMutex);
}


template< typename TypeMutex >
pitz::daq::NewLockGuard<TypeMutex>::~NewLockGuard()
{
    Unlock();
}


template< typename TypeMutex >
void pitz::daq::NewLockGuard<TypeMutex>::Lock(TypeMutex*   a_mutex)
{
    if(a_mutex && (!m_pMutex)){
        a_mutex->lock();
        m_pMutex = a_mutex;
    }
}


template< typename TypeMutex >
void pitz::daq::NewLockGuard<TypeMutex>::Unlock()
{
    if(m_pMutex){
        m_pMutex->unlock();
        m_pMutex = nullptr;
    }
}


/*/////////////////////////////////////////////////////////////////////////////////////////*/

//template< typename TypeSharedMutex >
//class NewSharedLockGuard

template< typename TypeSharedMutex >
pitz::daq::NewSharedLockGuard<TypeSharedMutex>::NewSharedLockGuard(TypeSharedMutex* a_pMutex)
{
    m_pMutex = nullptr;
    LockShared(a_pMutex);
}


template< typename TypeSharedMutex >
pitz::daq::NewSharedLockGuard<TypeSharedMutex>::~NewSharedLockGuard()
{
    UnlockShared();
}


template< typename TypeSharedMutex >
void pitz::daq::NewSharedLockGuard<TypeSharedMutex>::LockShared(TypeSharedMutex*  a_mutex)
{
    if(a_mutex && (!m_pMutex)){
        a_mutex->lock_shared();
        m_pMutex = a_mutex;
    }
}


template< typename TypeSharedMutex >
void pitz::daq::NewSharedLockGuard<TypeSharedMutex>::UnlockShared()
{
    if(m_pMutex){
        m_pMutex->unlock_shared();
        m_pMutex = nullptr;
    }
}

/*/////////////////////////////////////////////////////////////////////////////////////////*/

template <typename QueueType>
void pitz::daq::ProtectedQueue<QueueType>::pushBack(const QueueType& a_newElem)
{
    ::std::queue<QueueType>::push(a_newElem);
}


template <typename QueueType>
bool pitz::daq::ProtectedQueue<QueueType>::frontAndPop( QueueType* a_pBuffer )
{
    ::std::lock_guard< ::std::mutex > aGuard(m_mutexForQueue);
    if( ::std::queue<QueueType>::size() ){
        *a_pBuffer = ::std::queue<QueueType>::front();
        ::std::queue<QueueType>::pop();
        return true;
    }
    return false;
}

#endif  // #ifndef PITZ_DAQ_EQFCTCOLLECTOR_IMPL_HPP
