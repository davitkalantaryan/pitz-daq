//pitz_daq_memory.cpp
// 2019 Jun 28

#include <pitz/daq/data/memory/forclient.hpp>

using namespace pitz::daq;

data::memory::ForClient::ForClient( const EntryInfoBase& a_info, void* a_pParent )
    :
      data::memory::Base(a_pParent,0)
{
    SetBranchInfo(a_info);
}


data::memory::ForClient::ForClient(ForClient& a_cM,void* a_pParent)
    :
      data::memory::Base(a_pParent,0)
{
    swap(a_cM);
}


data::memory::ForClient::~ForClient()
{
    //
}
