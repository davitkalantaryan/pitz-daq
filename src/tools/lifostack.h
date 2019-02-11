#ifndef LIFOSTACK_H
#define LIFOSTACK_H

#include "smallmutex.h"

template <typename Type, int STACK_SIZE_>
class LifoStack
{    
public:
    LifoStack():m_nSize(0){}

    bool AddNew(const Type& a_tNew)
    {
        bool bRet(true);
        m_Mutex.Lock();
        if(m_nSize<STACK_SIZE_) {m_vtElements[m_nSize++] = a_tNew;}
        else                    {bRet = false;}
        m_Mutex.UnLock();
        return bRet;
    }


    bool Extract(Type*const& a_ptBuf)
    {
        bool bRet(true);
        m_Mutex.Lock();
        if(m_nSize) {*a_ptBuf = m_vtElements[--m_nSize];}
        else        {bRet = false;}
        m_Mutex.UnLock();
        return bRet;
    }
private:
    int         m_nSize;
    Type        m_vtElements[STACK_SIZE_];
    SmallMutex  m_Mutex;
};




////////////////////////////////////////////////////////
template <typename Type>
class LifoStackDyn
{
public:
    LifoStackDyn(int a_nStackSize)
            :   m_nStackSize(a_nStackSize>0 ? a_nStackSize:1),
                m_nSize(0),
                m_ptElements(NULL),
                m_Mutex()
    {
        m_ptElements = (Type*)malloc(sizeof(Type)*m_nStackSize);
        if(!m_ptElements)throw "No mem to create m_ptElements";
    }


    ~LifoStackDyn()
    {
        free(m_ptElements);
    }

    bool AddNew(const Type& a_tNew)
    {
        bool bRet(true);
        m_Mutex.Lock();
        if(m_nSize<m_nStackSize) {m_ptElements[m_nSize++] = a_tNew;}
        else                     {bRet = false;}
        m_Mutex.UnLock();
        return bRet;
    }


    bool Extract(Type*const& a_ptBuf)
    {
        bool bRet(true);
        m_Mutex.Lock();
        if(m_nSize) {*a_ptBuf = m_ptElements[--m_nSize];}
        else        {bRet = false;}
        m_Mutex.UnLock();
        return bRet;
    }
private:
    const int   m_nStackSize;
    int         m_nSize;
    Type*       m_ptElements;
    SmallMutex  m_Mutex;
};

#endif // LIFOSTACK_H
