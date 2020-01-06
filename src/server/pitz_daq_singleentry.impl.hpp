
// pitz_daq_singleentry.impl.hpp
// 2017 Sep 15

#ifndef PITZ_DAQ_SINGLEENTRY_IMPL_HPP
#define PITZ_DAQ_SINGLEENTRY_IMPL_HPP

#ifndef PITZ_DAQ_SINGLEENTRY_HPP
//#error Do not include this file directly
#include "pitz_daq_singleentry.hpp"
#endif

#include <stdint.h>
#include <stdlib.h>

template <typename IntType>
pitz::daq::EntryParams::IntParam<IntType>::IntParam(const char* a_entryParamName, ::std::list<EntryParams::Base*>* a_pContainer)
    :
      Base(a_entryParamName,a_pContainer)
{
    m_value = 0;
}


template <typename IntType>
pitz::daq::EntryParams::IntParam<IntType>::~IntParam()
{
}


template <typename IntType>
bool pitz::daq::EntryParams::IntParam<IntType>::GetDataFromLine(const char* a_entryLine)
{
    m_value = static_cast<IntType>(strtoll(a_entryLine,nullptr,10));
    return true;
}


template <typename IntType>
size_t pitz::daq::EntryParams::IntParam<IntType>::WriteDataToLineBuffer(char* a_entryLineBuffer, size_t a_unBufferSize)const
{
    return static_cast<size_t>(snprintf(a_entryLineBuffer,a_unBufferSize,"%lld",static_cast<long long int>(m_value)));
}


template <typename IntType>
pitz::daq::EntryParams::IntParam<IntType>::operator const IntType&()const
{
    return m_value;
}


template <typename IntType>
void pitz::daq::EntryParams::IntParam<IntType>::operator=(const IntType& a_newValue)
{
    m_value=a_newValue;
}


template <typename IntType>
void pitz::daq::EntryParams::IntParam<IntType>::operator++()
{
    ++m_value;
}


#endif // #ifndef PITZ_DAQ_SINGLEENTRY_IMPL_HPP
