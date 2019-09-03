
#ifndef PITZ_DAQ_DAQ_ROOT_READER_HPP
#define PITZ_DAQ_DAQ_ROOT_READER_HPP

#include <list>
#include <vector>
#include <string>
#include <stdint.h>
#include <pitz/daq/data/memory/forclient.hpp>

#define INPUT_PD
#define OUTPUT_PD



namespace pitz{ namespace daq{

struct BranchUserInputInfo{
    BranchUserInputInfo(const ::std::string& a_branchName):branchName(a_branchName){}
    BranchUserInputInfo(::std::string&& a_branchName):branchName(a_branchName){}
    ::std::string   branchName      INPUT_PD ;
};

struct BranchOutputForUserInfo{
    const BranchUserInputInfo*                  userClbk;
    data::EntryInfoBase                         info;
    ::std::vector< data::memory::ForClient* >   data;
};

int  RootInitialize();
void RootCleanup();
int  GetMultipleBranchesFromFile( const char* a_rootFileName, const ::std::list< BranchUserInputInfo >& a_pInput, ::std::list< BranchOutputForUserInfo* >* a_pOutput);
void GetMultipleBranchesForTime( time_t a_startTime, time_t a_endTime, const ::std::list< BranchUserInputInfo >& a_Input, ::std::list< BranchOutputForUserInfo* >* a_pOutput);

}}



#endif  // #ifndef PITZ_DAQ_DAQ_ROOT_READER_HPP
