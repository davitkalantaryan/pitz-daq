//
// file:        pitz/daq/data/getter/pipe.hpp
//

#ifndef __pitz_daq_data_getter_base_hpp__
#define __pitz_daq_data_getter_base_hpp__

#include <pitz/daq/base.hpp>
#include <pitz/daq/data/engine/base.hpp>

namespace pitz{ namespace daq { namespace data { namespace getter{

void GetBranchNames(const char* a_branchNamesRaw, ::std::vector< ::std::string >* a_pBranchNames);


class Base : public ::pitz::daq::Base
{
public:
    Base();
    virtual ~Base();

    engine::Base* operator->();
    const filter::Data& filter()const{return m_filter;}
    virtual void SetFilter(data::filter::Type::Type type, ...);

    virtual int  GetEntriesInfo( const char* rootFileName);
    virtual int  GetMultipleEntries( const char* rootFileName, const ::std::vector< ::std::string >& branchNames);
    virtual int  GetMultipleEntriesTI( const ::std::vector< ::std::string >& branchNames, int startTime, int endTime)=0;

protected:
    virtual daq::callbackN::retType::Type NumberOfEntries(int number)=0;
    virtual daq::callbackN::retType::Type ReadEntry(int index, const memory::Base& mem)=0;
    virtual daq::callbackN::retType::Type InfoGetter(int index, const data::EntryInfo& info)=0;
    virtual daq::callbackN::retType::Type AdvInfoGetter(int index, const engine::EntryInfoAdv& info)=0;

    void SetMultipleEntriesCallback();

protected:
    engine::Base*       m_pEngine;
    filter::Data        m_filter;

};

template <typename TypeGetter, typename TypeEngine>
class BaseTmp : public TypeGetter
{
public:
	BaseTmp() {m_engine.Initialize(); TypeGetter::m_pEngine=&m_engine;}
	template <typename TypeConstrArg>BaseTmp(TypeConstrArg a_arg) :m_engine(a_arg){m_engine.Initialize(); TypeGetter::m_pEngine=&m_engine;}
	virtual ~BaseTmp() { m_engine.Cleanup(); TypeGetter::m_pEngine = NULL; }

protected:
	TypeEngine	m_engine;
};


}}}} // namespace pitz{ namespace daq { namespace data { namespace getter{


#endif  // #ifndef __pitz_daq_data_getter_base_hpp__


