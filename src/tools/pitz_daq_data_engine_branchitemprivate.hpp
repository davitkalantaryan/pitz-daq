//
// file:    pitz_daq_data_engine_branchitemprivate.hpp
//
#ifndef __PITZ_DAQ_DATA_ENGINE_BRANCHITEMPRIVATE_HPP__
#define __PITZ_DAQ_DATA_ENGINE_BRANCHITEMPRIVATE_HPP__

#include <string>
#include <common/lists.hpp>

namespace pitz{ namespace daq{ namespace data{ namespace engine{

struct BranchItemPrivate
{
    std::string  branchName;
    ::common::listN::ListItem<BranchItemPrivate*> *item;
    int index;
    BranchItemPrivate(const ::std::string& a_branchName, int a_index):branchName(a_branchName),index(a_index){}
};

}}}}  // namespace pitz{ namespace daq{ namespace data{ namespace engine{


#endif // __PITZ_DAQ_DATA_ENGINE_BRANCHITEMPRIVATE_HPP__
