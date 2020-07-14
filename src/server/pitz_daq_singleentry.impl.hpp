
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
#include <pitz_daq_data_handling_types.h>

template <typename IntType>
pitz::daq::EntryParams::IntParam<IntType>::IntParam(const char* a_entryParamName)
    :
      Base(a_entryParamName)
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
size_t pitz::daq::EntryParams::IntParam<IntType>::writeDataToLineBuffer(char* a_entryLineBuffer, size_t a_unBufferSize)const
{
    return static_cast<size_t>(snprintf(a_entryLineBuffer,a_unBufferSize,"%lld",static_cast<long long int>(m_value)));
}


template <typename IntType>
pitz::daq::EntryParams::IntParam<IntType>::operator const IntType&()const
{
    return m_value;
}


template <typename IntType>
const IntType& pitz::daq::EntryParams::IntParam<IntType>::operator=(const IntType& a_newValue)
{
    m_value=a_newValue;
	return m_value;
}


template <typename IntType>
const IntType& pitz::daq::EntryParams::IntParam<IntType>::operator++()
{
	return ++m_value;
}

/*//////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

//template <typename PlatformType, typename EntryParamType>
//template <class... Args >
//pitz::daq::EntryParams::AddDataItem<PlatformType,EntryParamType>::AddDataItem(const char* a_entryParamName,Args&&... a_args)
//	:
//	  PlatformType(a_args...),
//	  EntryParamType(a_entryParamName)
//{
//	m_pBranch = nullptr;
//}
//
//
//template <typename PlatformType, typename EntryParamType>
//pitz::daq::EntryParams::AddDataItem<PlatformType,EntryParamType>::~AddDataItem()
//{
//}
//

/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

template <typename Int32Type>
pitz::daq::EntryParams::SomeInts<Int32Type>::SomeInts(const char* a_entryParamName)
	:
	  IntParam<Int32Type>(a_entryParamName)
{
	struct PrepareDaqEntryInputs in;
	struct PrepareDaqEntryOutputs out;

	memset(&in,0,sizeof(in));
	memset(&out,0,sizeof(out));

	m_rootable.header.samples = 1;
	m_rootable.header.branch_num_in_rcv_and_next_samples_in_root = 1;

	IntParam<Int32Type>::m_value = 0;

	in.dataType = DATA_INT;
	m_rootFormatString = PrepareDaqEntryBasedOnType(&in,&out);
}


template <typename Int32Type>
pitz::daq::EntryParams::SomeInts<Int32Type>::~SomeInts()
{
	//
}


template <typename Int32Type>
Int32Type pitz::daq::EntryParams::SomeInts<Int32Type>::value()const
{
	return IntParam<Int32Type>::m_value;
}


template <typename Int32Type>
pitz::daq::EntryParams::Base* pitz::daq::EntryParams::SomeInts<Int32Type>::thisPtr()
{
	return this;
}


template <typename Int32Type>
size_t pitz::daq::EntryParams::SomeInts<Int32Type>::writeDataToLineBuffer(char* a_entryLineBuffer, size_t a_unBufferSize)const
{
	::std::string strAdditionalString = this->additionalString();
	size_t unReturn = IntParam<Int32Type>::writeDataToLineBuffer(a_entryLineBuffer,a_unBufferSize);
	size_t unStrLen = strAdditionalString.length();

	if(unStrLen){
		if((unReturn+unStrLen+2)<=a_unBufferSize){
			a_entryLineBuffer[unReturn++]='(';
			memcpy(a_entryLineBuffer+unReturn,strAdditionalString.c_str(),unStrLen);
			unReturn += unStrLen;
			a_entryLineBuffer[unReturn++]=')';
		}
	}

	return unReturn;

}


template <typename Int32Type>
void pitz::daq::EntryParams::SomeInts<Int32Type>::Fill(DEC_OUT_PD(Header)* a_pHeader)
{
	m_rootable.header.seconds = a_pHeader->seconds;
	m_rootable.header.gen_event = a_pHeader->gen_event;
	m_rootable.value = IntParam<Int32Type>::m_value;
	IntParam<Int32Type>::m_pBranch->SetAddress(&m_rootable);
}

#define str(s) #s
#define DATA_INT_STR	str(DATA_INT)


template <typename Int32Type>
const char* pitz::daq::EntryParams::SomeInts<Int32Type>::rootFormatString()const
{
	//return "data_" DATA_INT_STR "/I";
	//return "data/I";
	return m_rootFormatString;
}


template <typename Int32Type>
int pitz::daq::EntryParams::SomeInts<Int32Type>::dataType()const
{
	return DATA_INT;
}


template <typename Int32Type>
int	pitz::daq::EntryParams::SomeInts<Int32Type>::samples() const
{
	return 1;
}

#endif // #ifndef PITZ_DAQ_SINGLEENTRY_IMPL_HPP
