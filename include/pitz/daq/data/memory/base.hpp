//
// file:        pitz/daq/data/memory/base.hpp
// created on:  2018 Nov 08
//
#ifndef __PITZ_DAQ_DATA_MEMORY_BASE_HPP__
#define __PITZ_DAQ_DATA_MEMORY_BASE_HPP__

#include <stddef.h>
#include <stdint.h>
#include <pitz/daq/data/entryinfo.hpp>
#include <cpp11+/common_defination.h>


namespace pitz{ namespace daq{ namespace data{ namespace memory{

class Base{
public:
        Base();
        Base(const Base& cM);
        virtual ~Base();
#ifdef __CPP11_DEFINED__
        Base( Base&& cM);
        Base& operator=( Base&& aM);
#endif
        Base& operator=(const Base& aM);
        const int&  time()const;
        const int&  gen_event()const;
        void* rawBuffer(){return m_rawBuffer;}
        const void* rawBuffer()const{return m_rawBuffer;}
        uint32_t   memorySize()const {return m_memorySize;}
        void       setBranchInfo(const EntryInfo& info);
        template <typename Type>const Type* data()const {return (const Type*)(m_rawBuffer + DAQ_HEADER_SIZE);}

private:
        char*       m_rawBuffer;
        uint32_t    m_memorySize;
        uint32_t    m_maxMemorySize;
};


}}}}  // namespace pitz{ namespace daq{ namespace data{ namespace memory{


#endif // __PITZ_DAQ_DATA_MEMORY_BASE_HPP__
