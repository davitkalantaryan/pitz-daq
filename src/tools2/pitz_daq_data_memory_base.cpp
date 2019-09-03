/*/////////////////////////////////////////////////////////////////////////////////*/

#include "pitz/daq/data/memory/base.hpp"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

using namespace pitz::daq;

data::memory::Base::Base(void* a_pParent,size_t a_unOffset)
    :
      m_pParent(a_pParent),
      m_rawBuffer(STATIC_CAST(char*,malloc(DAQ_HEADER_SIZE))),
      m_memorySize(DAQ_HEADER_SIZE),
      m_maxMemorySize(DAQ_HEADER_SIZE),
      m_unOffset(a_unOffset)
{
}


data::memory::Base::Base(const Base& a_cM)
    :
      m_pParent(a_cM.m_pParent),
      m_rawBuffer(STATIC_CAST(char*,malloc(a_cM.m_memorySize))),
      m_memorySize(a_cM.m_memorySize),
      m_maxMemorySize(a_cM.m_memorySize),
      m_unOffset(a_cM.m_unOffset)
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
      m_pParent(a_cM.m_pParent),
      m_rawBuffer(STATIC_CAST(char*,malloc(a_cM.m_maxMemorySize))),
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
        pcTmpBuffer = STATIC_CAST(char*,realloc(m_rawBuffer,newMemLength));
        if(!pcTmpBuffer){return *this;}
        m_rawBuffer = pcTmpBuffer;
        this->m_maxMemorySize = newMemLength;
    }
    m_memorySize = newMemLength;
    memcpy(m_rawBuffer,a_cM.m_rawBuffer,m_memorySize);
    return *this;
}
#endif  // #ifdef __CPP11_DEFINED__

#if 0
void*       m_pParent;
char*       m_rawBuffer;
uint32_t    m_memorySize;
uint32_t    m_maxMemorySize;
size_t      m_unOffset;
#endif

void data::memory::Base::swap(Base& a_cM)
{
    char* thisRawBufferOld = m_rawBuffer;
    uint32_t thisMemorySizeOld = m_memorySize;
    uint32_t thisMaxMemorySizeOld = m_maxMemorySize;
    size_t   thisOffsetOld = m_unOffset;

    m_rawBuffer = a_cM.m_rawBuffer;
    m_memorySize = a_cM.m_memorySize;
    m_maxMemorySize = a_cM.m_maxMemorySize;
    m_unOffset = a_cM.m_unOffset;

    a_cM.m_rawBuffer = thisRawBufferOld;
    a_cM.m_memorySize = thisMemorySizeOld;
    a_cM.m_maxMemorySize = thisMaxMemorySizeOld;
    a_cM.m_unOffset = thisOffsetOld;
}


data::memory::Base& data::memory::Base::operator=(const Base& a_cM)
{
    uint32_t newMemLength = a_cM.m_memorySize;
    m_unOffset = a_cM.m_unOffset;

    if(newMemLength>this->m_maxMemorySize){
        char* pcTmpBuffer;
        pcTmpBuffer = STATIC_CAST(char*,realloc(m_rawBuffer,newMemLength));
        if(!pcTmpBuffer){return *this;}
        m_rawBuffer = pcTmpBuffer;
        this->m_maxMemorySize = newMemLength;
    }
    m_memorySize = newMemLength;
    memcpy(m_rawBuffer,a_cM.m_rawBuffer,m_memorySize);
    return *this;
}

void* data::memory::Base::parent()
{
    return m_pParent;
}


void data::memory::Base::SetParent(void* a_newParent)
{
    m_pParent = a_newParent;
}


const int& data::memory::Base::time()const
{
    return *(  REINTERPRET_CAST(int*,(  m_rawBuffer+m_unOffset ) )  );
}


int& data::memory::Base::time()
{
    return *(  REINTERPRET_CAST(int*,(  m_rawBuffer+m_unOffset ) )  );
}


const int& data::memory::Base::gen_event()const
{
    return *(  REINTERPRET_CAST(int*,(  (m_rawBuffer+m_unOffset+4) ) )  );
}

int& data::memory::Base::gen_event()
{
    return *(  REINTERPRET_CAST(int*,(  (m_rawBuffer+m_unOffset+4) ) )  );
}


int data::memory::Base::Resize(size_t a_size)
{

    if(a_size>this->m_maxMemorySize){
        char* pcTmpBuffer;
        pcTmpBuffer = STATIC_CAST(char*,realloc(m_rawBuffer,a_size));
        if(!pcTmpBuffer){return -1;}
        m_rawBuffer = pcTmpBuffer;
        this->m_maxMemorySize = STATIC_CAST(uint32_t,a_size);
    }
    m_memorySize = STATIC_CAST(uint32_t,a_size);
    return 0;
}


void data::memory::Base::SetBranchInfo(const data::EntryInfoBase& a_info)
{
    uint32_t newMemLength = a_info.memorySize();
    this->Resize(newMemLength+m_unOffset);
}
