//
// file:        pitz/daq/data/indexer.hpp
//

#ifndef __pitz_daq_data_indexer_hpp__
#define __pitz_daq_data_indexer_hpp__

#include <pitz/daq/base.hpp>
#include <pitz/daq/callbackn.hpp>


namespace pitz{ namespace daq { namespace data { namespace indexer{

namespace callbackN{
typedef daq::callbackN::retType::Type (*TypeFileNameReader)(void*,const char*);
}

int DoIndexing(const char* rootFileName, const char* daqEntryName, int startTime, int endTime, int startGenEvent, int endGenEvent);
int GetListOfFilesForTimeInterval(void* clbkData, callbackN::TypeFileNameReader fpReader, const char* daqEntryName, int startTime, int endTime);
int GetListOfFilesForGenEventInterval(void* clbkData, callbackN::TypeFileNameReader fpReader,const char* daqEntryName, int startGenEvent, int endGenEvent);


}}}}  // namespace pitz{ namespace daq { namespace data { namespace indexer{


#endif  // #ifndef __pitz_daq_data_indexer_hpp__


