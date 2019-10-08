/*/////////////////////////////////////////////////////////////////////////////////*/

#include "pitz/daq/data/memory/base.hpp"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

using namespace pitz::daq;

data::memory::Base::Base(size_t a_unOffset)
    :
      m_rawBuffer(static_cast<char*>(malloc(DAQ_HEADER_SIZE))),
      m_memorySize(DAQ_HEADER_SIZE),
      m_maxMemorySize(DAQ_HEADER_SIZE),
      m_unOffset(a_unOffset)
{
}


data::memory::Base::Base(const Base& a_cM)
    :
      m_rawBuffer(static_cast<char*>(malloc(a_cM.m_memorySize))),
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


#ifdef CPP11_DEFINED2
data::memory::Base::Base( Base&& a_cM)
    :
      m_rawBuffer(static_cast<char*>(malloc(a_cM.m_maxMemorySize))),
      m_memorySize(a_cM.m_memorySize),
      m_maxMemorySize(a_cM.m_maxMemorySize),
      m_unOffset(a_cM.m_unOffset)
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
    m_unOffset = a_cM.m_unOffset;

    if(newMemLength>this->m_maxMemorySize){
        char* pcTmpBuffer;
        pcTmpBuffer = static_cast<char*>(realloc(m_rawBuffer,newMemLength));
        if(!pcTmpBuffer){return *this;}
        m_rawBuffer = pcTmpBuffer;
        this->m_maxMemorySize = newMemLength;
    }
    m_memorySize = newMemLength;
    memcpy(m_rawBuffer,a_cM.m_rawBuffer,m_memorySize);
    return *this;
}
#endif  // #ifdef CPP11_DEFINED2


data::memory::Base& data::memory::Base::operator=(const Base& a_cM)
{
    uint32_t newMemLength = a_cM.m_memorySize;
    m_unOffset = a_cM.m_unOffset;

    if(newMemLength>this->m_maxMemorySize){
        char* pcTmpBuffer;
        pcTmpBuffer = static_cast<char*>(realloc(m_rawBuffer,newMemLength));
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
    return *(  reinterpret_cast<int*>(  m_rawBuffer+m_unOffset )  );
}


int& data::memory::Base::time()
{
    return *(  reinterpret_cast<int*>(  m_rawBuffer+m_unOffset )  );
}


const int& data::memory::Base::gen_event()const
{
    return *(  reinterpret_cast<int*>(m_rawBuffer+m_unOffset+4)  );
}

int& data::memory::Base::gen_event()
{
    return *(  reinterpret_cast<int*>(  m_rawBuffer+m_unOffset+4)   );
}


int data::memory::Base::Resize(size_t a_size)
{

    if(a_size>this->m_maxMemorySize){
        char* pcTmpBuffer;
        pcTmpBuffer = static_cast<char*>(realloc(m_rawBuffer,a_size));
        if(!pcTmpBuffer){return -1;}
        m_rawBuffer = pcTmpBuffer;
        this->m_maxMemorySize = static_cast<uint32_t>(a_size);
    }
    m_memorySize = static_cast<uint32_t>(a_size);
    return 0;
}


void data::memory::Base::setBranchInfo(const data::EntryInfo& a_info)
{
    uint32_t newMemLength = a_info.memorySize();
    this->Resize(newMemLength+m_unOffset);
}
