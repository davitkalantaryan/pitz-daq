//
// file:        pitz/daq/data/getter/frompipe.hpp
//

#ifndef __pitz_daq_data_getter_frompipe_hpp__
#define __pitz_daq_data_getter_frompipe_hpp__

#include <pitz/daq/data/getter/noindexing.hpp>
#include <pitz/daq/data/engine/bypipe.hpp>
#include <common/common_unnamedsemaphorelite.hpp>
#include <cpp11+/thread_cpp11.hpp>
#include <cpp11+/shared_mutex_cpp14.hpp>
#include <signal.h>


namespace pitz{ namespace daq { namespace data { namespace getter{


class FromPipe : public getter::BaseTmp<getter::NoIndexing, engine::ByPipe>
{
public:
    FromPipe(  );
    virtual ~FromPipe();

    int connect(const char*, int = -1){return 0;}
    void disconnet();

	//int  GetMultipleEntriesTI(const ::std::vector< ::std::string >& branchNames, int startTime, int endTime) __OVERRIDE__ __FINAL__;
	virtual int  GetEntriesInfo(const char* rootFileName) __OVERRIDE__;
	virtual int  GetMultipleEntries(const char* rootFileName, const ::std::vector< ::std::string >& branchNames) __OVERRIDE__;
	virtual int  GetMultipleEntriesTI(const ::std::vector< ::std::string >& branchNames, int startTime, int endTime) __OVERRIDE__;

protected:
    int    CreateGetterAndPipes( const char* rootFileName, const ::std::vector< ::std::string > *  branchNames=NULL);
    void   PidWaitThread();

protected:
    ::common::List<int>             m_listOfPids;
    ::STDN::thread                  m_threadWaiter;
    ::STDN::shared_mutex            m_mutexForPids;
    struct sigaction                m_sigusr1Initial;
    volatile uint64_t               m_isWork : 1;
    volatile uint64_t               m_isWatcherRuns : 1;
    volatile uint64_t               m_isWaitForProcesses : 1;
    pthread_t                       m_mainThreadHandle;
    pthread_t                       m_waiterThreadHandle;

};


}}}}  // namespace pitz{ namespace daq { namespace data { namespace engine{


#endif  // #ifndef __pitz_daq_data_getter_frompipe_hpp__


