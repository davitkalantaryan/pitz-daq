/*/////////////////////////////////////////////////////////////////////////////////*/

#include "pitz/daq/data/memory/base.hpp"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

using namespace pitz::daq;

data::memory::Base::Base()
    :
      m_rawBuffer((char*)malloc(DAQ_HEADER_SIZE)),
      m_memorySize(DAQ_HEADER_SIZE),
      m_maxMemorySize(DAQ_HEADER_SIZE)
{
}


data::memory::Base::Base(const Base& a_cM)
    :
      m_rawBuffer((char*)malloc(a_cM.m_memorySize)),
      m_memorySize(a_cM.m_memorySize),
      m_maxMemorySize(a_cM.m_memorySize)
{
    // todo: calculate backtrace
    //printf("!!!!!!!!!!!!!!!!!!!!!!!!fl:%s,ln:%d\n",__FILE__,__LINE__);
    memcpy(m_rawBuffer,a_cM.m_rawBuffer,m_memorySize);
}


data::memory::Base::~Base()
{
    free(m_rawBuffer);
}


#ifdef __CPP11_DEFINED__
data::memory::Base::Base( Base&& a_cM)
    :
      m_rawBuffer((char*)malloc(a_cM.m_maxMemorySize)),
      m_memorySize(a_cM.m_memorySize),
      m_maxMemorySize(a_cM.m_maxMemorySize)
{
    //printf("fl:%s,ln:%d\n",__FILE__,__LINE__);
    //memcpy(m_rawBuffer,a_cM.m_rawBuffer,m_memorySize);
    char* pcTmp = a_cM.m_rawBuffer;
    a_cM.m_rawBuffer = this->m_rawBuffer;
    this->m_rawBuffer = pcTmp;
}

data::memory::Base& data::memory::Base::operator=( Base&& a_cM)
{
    uint32_t newMemLength = a_cM.m_memorySize;

    if(newMemLength>this->m_maxMemorySize){
        char* pcTmpBuffer;
        pcTmpBuffer = (char*)realloc(m_rawBuffer,newMemLength);
        if(!pcTmpBuffer){return *this;}
        m_rawBuffer = pcTmpBuffer;
        this->m_maxMemorySize = newMemLength;
    }
    m_memorySize = newMemLength;
    memcpy(m_rawBuffer,a_cM.m_rawBuffer,m_memorySize);
    return *this;
}
#endif  // #ifdef __CPP11_DEFINED__


data::memory::Base& data::memory::Base::operator=(const Base& a_cM)
{
    uint32_t newMemLength = a_cM.m_memorySize;

    if(newMemLength>this->m_maxMemorySize){
        char* pcTmpBuffer;
        pcTmpBuffer = (char*)realloc(m_rawBuffer,newMemLength);
        if(!pcTmpBuffer){return *this;}
        m_rawBuffer = pcTmpBuffer;
        this->m_maxMemorySize = newMemLength;
    }
    m_memorySize = newMemLength;
    memcpy(m_rawBuffer,a_cM.m_rawBuffer,m_memorySize);
    return *this;
}


const int& data::memory::Base::time()const
{
    return *(  (int*)(  (void*)m_rawBuffer )  );
}


const int& data::memory::Base::gen_event()const
{
    return *(  (int*)(  (void*)(m_rawBuffer+4) )  );
}


void data::memory::Base::setBranchInfo(const data::EntryInfo& a_info)
{
    uint32_t newMemLength = a_info.memorySize();

    if(newMemLength>this->m_maxMemorySize){
        char* pcTmpBuffer;
        pcTmpBuffer = (char*)realloc(m_rawBuffer,newMemLength);
        if(!pcTmpBuffer){return;}
        m_rawBuffer = pcTmpBuffer;
        this->m_maxMemorySize = newMemLength;
    }
    m_memorySize = newMemLength;
}
