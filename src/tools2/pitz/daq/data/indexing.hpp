//
// file:        pitz/daq/data/indexing.hpp
// created on:  2019 Jul 07
//
#ifndef PITZ_DAQ_DATA_INDEXING_HPP
#define PITZ_DAQ_DATA_INDEXING_HPP

#include <stdint.h>
#include <time.h>
#include <vector>
#include <string>

namespace pitz{ namespace daq{ namespace data{ namespace indexing{

//bool IndexingByEpoch( time_t from, time_t to, ::std::vector< ::std::string >* pFiles );
//bool IndexingByEventNumber( int64_t from, int64_t to, ::std::vector< ::std::string >* pFiles );

bool DoIndexing(const char* a_rootFileName,const char* a_daqEntryName,int a_startTime, int a_endTime,int a_startGenEvent,int a_endGenEvent);
bool GetListOfFilesForTimeInterval(const char* a_daqEntryName, time_t a_startTime, time_t a_endTime, ::std::vector< ::std::string >* a_pFiles );
bool GetListOfFilesForGenEventInterval(const char* a_daqEntryName, int64_t a_startGenEvent, int64_t a_endGenEvent, ::std::vector< ::std::string >* a_pFiles );

}}}}  // namespace pitz{ namespace daq{ namespace data{

#endif // PITZ_DAQ_DATA_INDEXING_HPP
