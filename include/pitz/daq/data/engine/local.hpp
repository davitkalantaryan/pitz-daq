//
// file:        pitz/daq/data/engine/local.hpp
//

#ifndef __pitz_daq_data_engine_local_hpp__
#define __pitz_daq_data_engine_local_hpp__

#include "pitz/daq/data/engine/base.hpp"

class TBranch;

namespace pitz{ namespace daq { namespace data{ namespace engine{

const char* GetDataTypeAndCount(const TBranch* a_pBranch, data::EntryInfo* a_pInfo);

class Local : public Base  __FINAL__
{
public:
    Local();
    ~Local();

    int  Initialize() __OVERRIDE__ __FINAL__;
    void Cleanup() __OVERRIDE__ __FINAL__;    
    int  GetEntriesInfo( const char* rootFileName) __OVERRIDE__ __FINAL__;
    int  GetMultipleEntries( const char* rootFileName, ::common::List<TBranchItemPrivate*>* pBranches) __OVERRIDE__ __FINAL__;

};


}}}}  // namespace pitz{ namespace daq { namespace data{ namespace engine{


#endif  // #ifndef __pitz_daq_data_engine_local_hpp__
