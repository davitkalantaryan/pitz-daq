//
// file:        pitz/daq/data/memory/base.hpp
// created on:  2018 Nov 08
//
#ifndef PITZ_DAQ_DATA_MEMORY_FORSERVER_HPP
#define PITZ_DAQ_DATA_MEMORY_FORSERVER_HPP

#include <pitz/daq/data/memory/base.hpp>

#ifdef ROOT_APP
#include "Rtypes.h"
#else   // #ifdef ROOT_APP
typedef int Int_t;
typedef float Float_t;
#endif  // #ifdef ROOT_APP


namespace pitz{ namespace daq{

bool copyString(char** a_dst, const char* a_str);

namespace data{ namespace memory{

class ForServerBase : public Base
{
public:
        ForServerBase(void* parent, size_t offset);
        //ForServerBase(const Base& cM);
        virtual ~ForServerBase();
#ifdef CPP11_DEFINED2
        //ForServerBase( ForServerBase&& cM);
        //ForServerBase& operator=( ForServerBase&& aM);
#endif

        void* Parent();
        void SetParent(void* newParent);
        virtual void* buffer();
        virtual const void* buffer()const;
        void* bufferForValue();
        const void* bufferForValue()const;
        void* bufferForRoot();
        virtual void copyFrom(const ForServerBase* cM)=0;
        virtual size_t streamSize()const{return 12;}

protected:
        void copyFromProtected(const ForServerBase* cM);

protected:
        void*    m_pParent;
};


class M01 : public ForServerBase
{
public:
    M01(void* parent);
    virtual ~M01();

    Int_t& value();
    const Int_t& value()const;
    virtual void copyFrom(const ForServerBase* cM);

};


class M02 : public ForServerBase
{
public:
    M02(void* parent);
    virtual ~M02();

    Float_t& value();
    const Float_t& value()const;
    virtual void copyFrom(const ForServerBase* cM);

};


class M03 : public ForServerBase
{
public:
    M03(void* parent, size_t strLen);
    virtual ~M03();

    char* value();
    const char* value()const;
    const size_t& strLen()const;
    virtual void copyFrom(const ForServerBase* cM);
    virtual size_t streamSize()const{return m_strLen+8;}

protected:
    size_t  m_strLen;
};


class M15 : public ForServerBase
{
public:
    M15(void* parent);
    virtual ~M15();

    char* value();
    const char* value()const;
    virtual void copyFrom(const ForServerBase* cM);
    virtual size_t streamSize()const;

};


class M19 : public ForServerBase
{
public:
    M19(void* parent, size_t numberOfElements, size_t offset);
    virtual ~M19();

    int PrepareUdpReceivedData(int* secondsPtr, int* eventNumberPtr);
    int SwapDataIfNecessary();

    Float_t* value();
    const Float_t* value()const;
    virtual void copyFrom(const ForServerBase* cM);
    size_t SetElements(const float* fpValues,size_t numOfElements);
    virtual size_t streamSize()const{return 8200;}

protected:
    size_t       m_numberOfElements;
};


}}}}  // namespace pitz{ namespace daq{ namespace data{ namespace memory{


#endif // PITZ_DAQ_DATA_MEMORY_FORSERVER_HPP
