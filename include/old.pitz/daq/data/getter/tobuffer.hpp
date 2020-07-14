//
// file:        pitz/daq/data/getter/tobuffer.hpp
//

#ifndef __pitz_daq_data_getter_tobuffer_hpp__
#define __pitz_daq_data_getter_tobuffer_hpp__

#include <pitz/daq/data/getter/base.hpp>
#include <pitz/daq/callbackn.hpp>
#include <vector>


namespace pitz{ namespace daq { namespace data{ namespace getter{

struct EntryInfoAdvTI
{
    data::EntryInfoBase                 entryInfo;
    ::std::vector< data::memory::Base > entryData;
};


template <typename TypeGetter>
class ToBuffer : public TypeGetter
{
public:
	//template <typename TypeEngine>ToBuffer(  );
	ToBuffer();
    virtual ~ToBuffer();

    const ::std::vector< engine::EntryInfoAdv >&   Info()const{return m_vectInfo;}
    const ::std::vector< EntryInfoAdvTI >&   Data()const{return m_vectData;}

    virtual int  GetMultipleEntries( const char* rootFileName, const ::std::vector< ::std::string >& branchNames) __OVERRIDE__ ;
    virtual int  GetMultipleEntriesTI( const ::std::vector< ::std::string >& branchNames, int startTime, int endTime) __OVERRIDE__;

protected:
    virtual daq::callbackN::retType::Type NumberOfEntries(int number);
    virtual daq::callbackN::retType::Type ReadEntry(int index, const memory::Base& mem);
    virtual daq::callbackN::retType::Type InfoGetter(int index, const data::EntryInfo& info);
    virtual daq::callbackN::retType::Type AdvInfoGetter(int index, const engine::EntryInfoAdv& info);

protected:
    ::std::vector< engine::EntryInfoAdv >   m_vectInfo;
    ::std::vector< EntryInfoAdvTI >         m_vectData;
};


}}}}  // namespace pitz{ namespace daq { namespace data{ namespace getter{

#include "impl.tobuffer.hpp"


#endif  // #ifndef __pitz_daq_data_getter_tobuffer_hpp__


