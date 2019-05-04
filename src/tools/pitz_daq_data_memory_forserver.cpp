
//pitz_daq_memory.cpp
// 2017 Sep 11

#include "pitz/daq/data/memory/forserver.hpp"
#include <stdlib.h>
#include <memory.h>

namespace pitz{ namespace daq{
void swap4Bytes2(void* a_argPtr, int a_nCount);
}}

using namespace pitz::daq;

data::memory::ForServerBase::ForServerBase(void* a_pParent, size_t a_unOffset)
        :
        Base(a_unOffset),
        m_pParent(a_pParent)
{
    // 8 Byte: 4->time, 4->eventNumber
    if(Resize(m_unOffset+8)){throw "Low memory!";}
}


data::memory::ForServerBase::~ForServerBase()
{
    //free(m_pBuffer);
}


void* data::memory::ForServerBase::Parent()
{
    return m_pParent;
}


void data::memory::ForServerBase::SetParent(void* a_newParent)
{
    m_pParent = a_newParent;
}


void* data::memory::ForServerBase::bufferForValue()
{
    return m_rawBuffer+m_unOffset+pitz::daq::data::memory::offset::VALUE;
}


const void* data::memory::ForServerBase::bufferForValue()const
{
    return m_rawBuffer+m_unOffset+pitz::daq::data::memory::offset::VALUE;
}


void* data::memory::ForServerBase::bufferForRoot()
{
    return ( void*)(m_rawBuffer+m_unOffset);
}


const void* data::memory::ForServerBase::buffer()const
{
    return ( const void*)(m_rawBuffer+m_unOffset);
}


void* data::memory::ForServerBase::buffer()
{
    return ( void*)(m_rawBuffer+m_unOffset);
}


void data::memory::ForServerBase::copyFromProtected(const ForServerBase* a_cM)
{
    time() = a_cM->time();
    gen_event() = a_cM->gen_event();
}


/*///////////////////////////////////*/
data::memory::M01::M01(void* a_pParent)
        :
        ForServerBase(a_pParent,0)
{
    if(Resize(12)){throw "Low memory!";}
}


data::memory::M01::~M01()
{
}


Int_t& data::memory::M01::value()
{
    return *((Int_t*)(m_rawBuffer+m_unOffset+pitz::daq::data::memory::offset::VALUE));
}


const Int_t& data::memory::M01::value()const
{
    return *((const Int_t*)(m_rawBuffer+m_unOffset+memory::offset::VALUE));
}


void data::memory::M01::copyFrom(const ForServerBase* a_cM)
{
    M01* cM = (M01*)a_cM;
    this->copyFromProtected(a_cM);
    value() = cM->value();
}


/*///////////////////////////////////*/
data::memory::M02::M02(void* a_pParent)
        :
        ForServerBase(a_pParent,0)
{
    if(Resize(12)){throw "Low memory!";}
}


data::memory::M02::~M02()
{
}


Float_t& data::memory::M02::value()
{
    return *((Float_t*)(m_rawBuffer+m_unOffset+pitz::daq::data::memory::offset::VALUE));
}


const Float_t& data::memory::M02::value()const
{
    return *((const Float_t*)(m_rawBuffer+m_unOffset+memory::offset::VALUE));
}


void data::memory::M02::copyFrom(const ForServerBase* a_cM)
{
    M02* cM = (M02*)a_cM;
    this->copyFromProtected(a_cM);
    value() = cM->value();
}


/*///////////////////////////////////*/
data::memory::M03::M03(void* a_pParent,size_t a_strLen)
        :
        ForServerBase(a_pParent,0),
        m_strLen(a_strLen)
{
    if(Resize(a_strLen+8)){throw "Low memory!";}
}


data::memory::M03::~M03()
{
}


char* data::memory::M03::value()
{
    return (char*)(m_rawBuffer+m_unOffset+pitz::daq::data::memory::offset::VALUE);
}


const char* data::memory::M03::value()const
{
    return (const char*)(m_rawBuffer+m_unOffset+pitz::daq::data::memory::offset::VALUE);
}


const size_t& data::memory::M03::strLen()const
{
    return m_strLen;
}


void data::memory::M03::copyFrom(const ForServerBase* a_cM)
{
    M03* cM = (M03*)a_cM;
    this->copyFromProtected(a_cM);
    if(m_strLen<cM->m_strLen){
        Resize(cM->m_strLen+8);
    }m_strLen=cM->m_strLen;
    memcpy(value(),cM->value(),m_strLen+1);
}



/*///////////////////////////////////*/
data::memory::M15::M15(void* a_pParent)
        :
        ForServerBase(a_pParent,0)
{
    if(Resize(128)){throw "Low memory!";}
}


data::memory::M15::~M15()
{
}


char* data::memory::M15::value()
{
    return (char*)(m_rawBuffer+m_unOffset+pitz::daq::data::memory::offset::VALUE);
}


const char* data::memory::M15::value()const
{
    return (const char*)(m_rawBuffer+m_unOffset+pitz::daq::data::memory::offset::VALUE);
}

void data::memory::M15::copyFrom(const ForServerBase* a_cM)
{
    memcpy(m_rawBuffer,a_cM->buffer(),128);
}


size_t data::memory::M15::streamSize()const
{
    return 128;
}


/*///////////////////////////////////*/
data::memory::M19::M19(void* a_pParent, size_t a_numberOfElements,size_t a_unOffset)
        :
        ForServerBase(a_pParent,a_unOffset),
        m_numberOfElements(a_numberOfElements)
{
    if(Resize(a_unOffset+8+m_numberOfElements*4)){throw "Low memory!";}
}


data::memory::M19::~M19()
{
}


Float_t* data::memory::M19::value()
{
    return (Float_t*)(m_rawBuffer+m_unOffset+pitz::daq::data::memory::offset::VALUE);
}


const Float_t* data::memory::M19::value()const
{
    return (const Float_t*)(m_rawBuffer+m_unOffset+pitz::daq::data::memory::offset::VALUE);
}


void data::memory::M19::copyFrom(const ForServerBase* a_cM)
{
    M19* cM = (M19*)a_cM;
    size_t unNewSize = cM->m_numberOfElements*4+8+m_unOffset;
    this->copyFromProtected(a_cM);
    if(m_numberOfElements<cM->m_numberOfElements){
        Resize(unNewSize);
    }m_numberOfElements=cM->m_numberOfElements;
    memcpy(m_rawBuffer,cM->m_rawBuffer,unNewSize);
}


size_t data::memory::M19::SetElements(const float* a_fpValues,size_t a_numOfElements)
{
    if(a_numOfElements > m_numberOfElements){a_numOfElements=m_numberOfElements;}
    memcpy(value(),a_fpValues,a_numOfElements);
    return a_numOfElements;
}


int data::memory::M19::SwapDataIfNecessary()
{
    int & nEndian(*((int*)m_rawBuffer));
    int & nBranchNum =  *((int*)(m_rawBuffer+4));
    int & nSecondsIn = *((int*)(m_rawBuffer+8));
    int & nSecondsFnlGenIn = *((int*)(m_rawBuffer+12));
    int & nGenEventFnl = *((int*)(m_rawBuffer+16));

    int nGenEvent = nSecondsFnlGenIn;

    nSecondsFnlGenIn = nSecondsIn;
    nGenEventFnl = nGenEvent;

    if(nEndian !=1 ){
        swap4Bytes2(&nBranchNum,1);
        swap4Bytes2(&nSecondsFnlGenIn,1);
        swap4Bytes2(&nGenEventFnl,1);
        swap4Bytes2(m_rawBuffer+m_unOffset+pitz::daq::data::memory::offset::VALUE,m_numberOfElements);
    }

    //*a_pEventNumber = nGenEventFnl =nSecondsFnlGenIn;
    //*a_pSeconds = nSecondsFnlGenIn = nSecondsIn;

    return nBranchNum;
}




/*////////////////////////////////*/
namespace pitz{ namespace daq{

void swap4Bytes2(void* a_argPtr, int a_nCount)
{
    //unsigned int& arg = *((unsigned int*)a_argPtr);
    unsigned int* argPtr = (unsigned int*)a_argPtr;
    union
    {
        unsigned int argc;
        char ch[4];
    } integ;

    char ch;

    for(int i(0);i<a_nCount;++i){
        integ.argc = argPtr[i];

        ch = integ.ch[0];
        integ.ch[0] = integ.ch[3];
        integ.ch[3] = ch;

        ch = integ.ch[1];
        integ.ch[1] = integ.ch[2];
        integ.ch[2] = ch;

        argPtr[i] = integ.argc;
    }

}


bool copyString(char** a_dst, const char* a_str)
{
    if(*a_dst != NULL){return true;}
    size_t unStrLenPlus1(a_str?(strlen(a_str)+1):1);
    *a_dst = (char*)malloc(unStrLenPlus1);
    if(!(*a_dst)){throw "Low memory!";}
    if(a_str){memcpy((*a_dst),a_str,unStrLenPlus1);}
    else{*(*a_dst)=0;}

    return true;
}


}}

