//
// file:        pitz/daq/data/engine/local.hpp
//

#ifndef PITZ_DAQ_DATA_ENGINE_LOCAL_HPP
#define PITZ_DAQ_DATA_ENGINE_LOCAL_HPP

#include "pitz/daq/data/engine/base.hpp"

class TBranch;

namespace pitz{ namespace daq { namespace data{ namespace engine{

const char* GetDataTypeAndCount(const TBranch* a_pBranch, data::EntryInfo* a_pInfo);

class Local __FINAL__ : public Base
{
public:
    Local();
    ~Local() __OVERRIDE__ ;

    int  Initialize() __OVERRIDE__ __FINAL__;
    void Cleanup() __OVERRIDE__ __FINAL__;    
    int  GetEntriesInfo( const char* rootFileName) __OVERRIDE__ __FINAL__;
    int  GetMultipleEntries( const char* rootFileName, ::common::List<TBranchItemPrivate*>* pBranches) __OVERRIDE__ __FINAL__;

};


}}}}  // namespace pitz{ namespace daq { namespace data{ namespace engine{


#endif  // #ifndef PITZ_DAQ_DATA_ENGINE_LOCAL_HPP
