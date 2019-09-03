//
// file:        pitz/daq/data/memory/base.hpp
// created on:  2018 Nov 08
//
#ifndef PITZ_DAQ_DATA_MEMORY_BASE_HPP
#define PITZ_DAQ_DATA_MEMORY_BASE_HPP

#include <stddef.h>
#include <stdint.h>
#include <pitz/daq/data/entryinfo.hpp>
#include <cpp11+/common_defination.h>


namespace pitz{ namespace daq{ namespace data{ namespace memory{

namespace offset{enum Type{VALUE=8};}

class Base{
public:
        Base(void* a_pParent,size_t unOffset=0);
        Base(const Base& cM);
        virtual ~Base();
#ifdef __CPP11_DEFINED__
        Base( Base&& cM);
        Base& operator=( Base&& aM);
#endif
        Base& operator=(const Base& aM);
        const int&  time()const;
        const int&  gen_event()const;
              int&  time();
              int&  gen_event();
        void*       rawBuffer(){return m_rawBuffer;}
        const void* rawBuffer()const{return m_rawBuffer;}
        uint32_t    memorySize()const {return m_memorySize;}
        void        SetBranchInfo(const EntryInfoBase& info);
        void*       parent();
        void        SetParent(void* newParent);
        template <typename Type>const Type* dataPtr()const {return REINTERPRET_CAST(const Type*,(m_rawBuffer + m_unOffset+DAQ_HEADER_SIZE));}
        template <typename Type>const Type& data(size_t a_unIndex)const {return *(dataPtr<Type>()+a_unIndex);}

protected:
        int Resize(size_t size);
        void swap(Base& other);

protected:
        void*       m_pParent;
        char*       m_rawBuffer;
        uint32_t    m_memorySize;
        uint32_t    m_maxMemorySize;
        size_t      m_unOffset;
};


}}}}  // namespace pitz{ namespace daq{ namespace data{ namespace memory{


#endif // PITZ_DAQ_DATA_MEMORY_BASE_HPP
