//
// file:        mex_simple_root_reader.cpp
//

#ifndef __pitz_daq_data_getter_impl_tobuffer_hpp__
#define __pitz_daq_data_getter_impl_tobuffer_hpp__

#ifndef __pitz_daq_data_getter_tobuffer_hpp__
#error do not include directly
#include "tobuffer.hpp"
#endif

#include "../src/tools/pitz_daq_data_engine_branchitemprivate.hpp"


template <typename TypeGetter>
//template <typename TypeEngine>
::pitz::daq::data::getter::ToBuffer<TypeGetter>::ToBuffer(  )
{
}


template <typename TypeGetter>
::pitz::daq::data::getter::ToBuffer<TypeGetter>::~ToBuffer(  )
{
	TypeGetter::m_pEngine = NULL;
}


template <typename TypeGetter>
::pitz::daq::callbackN::retType::Type  pitz::daq::data::getter::ToBuffer<TypeGetter>::NumberOfEntries(int a_number)
{
    if(TypeGetter::m_pEngine->callbackType() == engine::callbackN::Type::MultiEntries){m_vectData.resize(a_number);}
    else if(TypeGetter::m_pEngine->callbackType() == engine::callbackN::Type::Info){ m_vectInfo.resize(a_number);}
    else{ return daq::callbackN::retType::Stop;}
    return daq::callbackN::retType::Continue;
}


template <typename TypeGetter>
::pitz::daq::callbackN::retType::Type pitz::daq::data::getter::ToBuffer<TypeGetter>::ReadEntry(int a_index, const memory::Base& a_mem)
{
    if( a_index>=((int)m_vectData.size())  ){return daq::callbackN::retType::Stop;}
    m_vectData[a_index].entryData.push_back(a_mem);
    return daq::callbackN::retType::Continue;
}


template <typename TypeGetter>
::pitz::daq::callbackN::retType::Type pitz::daq::data::getter::ToBuffer<TypeGetter>::InfoGetter(int a_index, const data::EntryInfo& a_info)
{
    if( a_index>=((int)m_vectData.size())  ){return daq::callbackN::retType::Stop;}
    m_vectData[a_index].entryInfo.dataType = a_info.dataType;
    m_vectData[a_index].entryInfo.itemsCount = a_info.itemsCount;
    return daq::callbackN::retType::Continue;
}


template <typename TypeGetter>
::pitz::daq::callbackN::retType::Type pitz::daq::data::getter::ToBuffer<TypeGetter>::AdvInfoGetter(int a_index, const engine::EntryInfoAdv& a_info)
{
    if( a_index>=((int)m_vectInfo.size())  ){return daq::callbackN::retType::Stop;}
    m_vectInfo[a_index] = a_info;
    return daq::callbackN::retType::Continue;
}


template <typename TypeGetter>
int pitz::daq::data::getter::ToBuffer<TypeGetter>::GetMultipleEntries( const char* a_rootFileName,const ::std::vector< ::std::string >& a_branchNames)
{
    this->m_vectData.resize(a_branchNames.size());
    return TypeGetter::GetMultipleEntries(a_rootFileName,a_branchNames);
}


template <typename TypeGetter>
int pitz::daq::data::getter::ToBuffer<TypeGetter>::GetMultipleEntriesTI( const ::std::vector< ::std::string >& a_branchNames, int a_startTime, int a_endTime)
{
    this->m_vectData.resize(a_branchNames.size());
    return TypeGetter::GetMultipleEntriesTI(a_branchNames, a_startTime, a_endTime);
}

#endif  // #ifndef __pitz_daq_data_getter_impl_tobuffer_hpp__
