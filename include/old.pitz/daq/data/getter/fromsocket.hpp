//
// file:        pitz/daq/data/getter/fromsocket.hpp
//

#ifndef __pitz_daq_data_getter_fromsocket_hpp__
#define __pitz_daq_data_getter_fromsocket_hpp__

#include <pitz/daq/data/browser_server_client_common.h>
#include <pitz/daq/data/getter/noindexing.hpp>
#include <pitz/daq/data/engine/bypipe.hpp>
#include <common/common_unnamedsemaphorelite.hpp>
#include <cpp11+/thread_cpp11.hpp>
#include <signal.h>


namespace pitz{ namespace daq { namespace data { namespace getter{


class FromSocket : public getter::BaseTmp<getter::NoIndexing, engine::ByPipe>
{
public:
	FromSocket(const char* serverName=NULL, int serverPort=-1 );
    virtual ~FromSocket();

	int connect(const char* serverName, int serverPort= PITZ_DAQ_BROWSER_SERVER_DEFAULT_PORT);
	void disconnet();

	//int  GetMultipleEntriesTI(const ::std::vector< ::std::string >& branchNames, int startTime, int endTime) __OVERRIDE__ __FINAL__;
	virtual int  GetEntriesInfo(const char* rootFileName) __OVERRIDE__;
	virtual int  GetMultipleEntries(const char* rootFileName, const ::std::vector< ::std::string >& branchNames) __OVERRIDE__;
	virtual int  GetMultipleEntriesTI(const ::std::vector< ::std::string >& branchNames, int startTime, int endTime) __OVERRIDE__;

protected:
	uint16_t		m_isEndianDiffer;

};


}}}}  // namespace pitz{ namespace daq { namespace data { namespace engine{


#endif  // #ifndef __pitz_daq_data_getter_fromsocket_hpp__


