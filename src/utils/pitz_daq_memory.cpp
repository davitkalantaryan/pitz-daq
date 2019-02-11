
//pitz_daq_memory.cpp
// 2017 Sep 11

#include "pitz_daq_memory.hpp"
#include <stdlib.h>
#include <memory.h>

pitz::daq::MemoryBase::MemoryBase(void* a_pParent, size_t a_unOffset)
        :
        m_pParent(a_pParent),
        m_pBuffer(NULL),
        m_unMaxSize(0),
        m_unOffset(a_unOffset)
{
    // 8 Byte: 4->time, 4->eventNumber
    if(Resize(m_unOffset+8)){throw "Low memory!";}
}


pitz::daq::MemoryBase::~MemoryBase()
{
    free(m_pBuffer);
}


void* pitz::daq::MemoryBase::Parent()
{
    return m_pParent;
}


void pitz::daq::MemoryBase::SetParent(void* a_newParent)
{
    m_pParent = a_newParent;
}


int pitz::daq::MemoryBase::Resize(size_t a_size)
{
    if(a_size>m_unMaxSize){
        m_pBuffer = (uint8_t*)realloc(m_pBuffer,a_size);
        if(!m_pBuffer){
            m_unMaxSize = 0;
            return -1;
        } // if(!m_pBuffer){
        m_unMaxSize = a_size;
    } // if(a_size>m_unMaxSize){
    return 0;
}


Int_t& pitz::daq::MemoryBase::time()
{
    return *((Int_t*)(m_pBuffer+m_unOffset+memory::offset::TIME));
}


const Int_t& pitz::daq::MemoryBase::time()const
{
    return *((const Int_t*)(m_pBuffer+m_unOffset+memory::offset::TIME));
}


Int_t& pitz::daq::MemoryBase::eventNumber()
{
    return *((Int_t*)(m_pBuffer+m_unOffset+pitz::daq::memory::offset::EVENT_NUM));
}


const Int_t& pitz::daq::MemoryBase::eventNumber()const
{
    return *((const Int_t*)(m_pBuffer+m_unOffset+memory::offset::EVENT_NUM));
}


void* pitz::daq::MemoryBase::bufferForRoot()
{
    return ( void*)(m_pBuffer+m_unOffset);
}


const void* pitz::daq::MemoryBase::buffer()const
{
    return ( const void*)(m_pBuffer+m_unOffset);
}


void pitz::daq::MemoryBase::copyFromProtected(const MemoryBase* a_cM)
{
    time() = a_cM->time();
    eventNumber() = a_cM->eventNumber();
}


/*///////////////////////////////////*/
pitz::daq::Memory01::Memory01(void* a_pParent)
        :
        MemoryBase(a_pParent,0)
{
    if(Resize(12)){throw "Low memory!";}
}


pitz::daq::Memory01::~Memory01()
{
}


Int_t& pitz::daq::Memory01::value()
{
    return *((Int_t*)(m_pBuffer+m_unOffset+pitz::daq::memory::offset::VALUE));
}


const Int_t& pitz::daq::Memory01::value()const
{
    return *((const Int_t*)(m_pBuffer+m_unOffset+memory::offset::VALUE));
}


void pitz::daq::Memory01::copyFrom(const MemoryBase* a_cM)
{
    Memory01* cM = (Memory01*)a_cM;
    this->copyFromProtected(a_cM);
    value() = cM->value();
}


/*///////////////////////////////////*/
pitz::daq::Memory02::Memory02(void* a_pParent)
        :
        MemoryBase(a_pParent,0)
{
    if(Resize(12)){throw "Low memory!";}
}


pitz::daq::Memory02::~Memory02()
{
}


Float_t& pitz::daq::Memory02::value()
{
    return *((Float_t*)(m_pBuffer+m_unOffset+pitz::daq::memory::offset::VALUE));
}


const Float_t& pitz::daq::Memory02::value()const
{
    return *((const Float_t*)(m_pBuffer+m_unOffset+memory::offset::VALUE));
}


void pitz::daq::Memory02::copyFrom(const MemoryBase* a_cM)
{
    Memory02* cM = (Memory02*)a_cM;
    this->copyFromProtected(a_cM);
    value() = cM->value();
}


/*///////////////////////////////////*/
pitz::daq::Memory03::Memory03(void* a_pParent,size_t a_strLen)
        :
        MemoryBase(a_pParent,0),
        m_strLen(a_strLen)
{
    if(Resize(a_strLen+8)){throw "Low memory!";}
}


pitz::daq::Memory03::~Memory03()
{
}


char* pitz::daq::Memory03::value()
{
    return (char*)(m_pBuffer+m_unOffset+pitz::daq::memory::offset::VALUE);
}


const char* pitz::daq::Memory03::value()const
{
    return (const char*)(m_pBuffer+m_unOffset+pitz::daq::memory::offset::VALUE);
}


const size_t& pitz::daq::Memory03::strLen()const
{
    return m_strLen;
}


void pitz::daq::Memory03::copyFrom(const MemoryBase* a_cM)
{
    Memory03* cM = (Memory03*)a_cM;
    this->copyFromProtected(a_cM);
    if(m_strLen<cM->m_strLen){
        Resize(cM->m_strLen+8);
    }m_strLen=cM->m_strLen;
    memcpy(value(),cM->value(),m_strLen+1);
}



/*///////////////////////////////////*/
pitz::daq::Memory15::Memory15(void* a_pParent)
        :
        MemoryBase(a_pParent,0)
{
    if(Resize(128)){throw "Low memory!";}
}


pitz::daq::Memory15::~Memory15()
{
}


char* pitz::daq::Memory15::value()
{
    return (char*)(m_pBuffer+m_unOffset+pitz::daq::memory::offset::VALUE);
}


const char* pitz::daq::Memory15::value()const
{
    return (const char*)(m_pBuffer+m_unOffset+pitz::daq::memory::offset::VALUE);
}

void pitz::daq::Memory15::copyFrom(const MemoryBase* a_cM)
{
    memcpy(m_pBuffer,a_cM->buffer(),128);
}



/*///////////////////////////////////*/
pitz::daq::Memory19::Memory19(void* a_pParent, size_t a_numberOfElements,size_t a_unOffset)
        :
        MemoryBase(a_pParent,a_unOffset),
        m_numberOfElements(a_numberOfElements)
{
    if(Resize(a_unOffset+8+m_numberOfElements*4)){throw "Low memory!";}
}


pitz::daq::Memory19::~Memory19()
{
}


Float_t* pitz::daq::Memory19::value()
{
    return (Float_t*)(m_pBuffer+m_unOffset+pitz::daq::memory::offset::VALUE);
}


const Float_t* pitz::daq::Memory19::value()const
{
    return (const Float_t*)(m_pBuffer+m_unOffset+pitz::daq::memory::offset::VALUE);
}


void pitz::daq::Memory19::copyFrom(const MemoryBase* a_cM)
{
    Memory19* cM = (Memory19*)a_cM;
    size_t unNewSize = cM->m_numberOfElements*4+8+m_unOffset;
    this->copyFromProtected(a_cM);
    if(m_numberOfElements<cM->m_numberOfElements){
        Resize(unNewSize);
    }m_numberOfElements=cM->m_numberOfElements;
    memcpy(m_pBuffer,cM->m_pBuffer,unNewSize);
}


size_t pitz::daq::Memory19::SetElements(const float* a_fpValues,size_t a_numOfElements)
{
    if(a_numOfElements > m_numberOfElements){a_numOfElements=m_numberOfElements;}
    memcpy(value(),a_fpValues,a_numOfElements);
    return a_numOfElements;
}


int pitz::daq::Memory19::SwapDataIfNecessary()
{
    int & nEndian(*((int*)m_pBuffer));
    int & nBranchNum =  *((int*)(m_pBuffer+4));
    int & nSecondsIn = *((int*)(m_pBuffer+8));
    int & nSecondsFnlGenIn = *((int*)(m_pBuffer+12));
    int & nGenEventFnl = *((int*)(m_pBuffer+16));

    int nGenEvent = nSecondsFnlGenIn;

    nSecondsFnlGenIn = nSecondsIn;
    nGenEventFnl = nGenEvent;

    if(nEndian !=1 ){
        swap4Bytes2(&nBranchNum,1);
        swap4Bytes2(&nSecondsFnlGenIn,1);
        swap4Bytes2(&nGenEventFnl,1);
        swap4Bytes2(m_pBuffer+m_unOffset+pitz::daq::memory::offset::VALUE,m_numberOfElements);
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

