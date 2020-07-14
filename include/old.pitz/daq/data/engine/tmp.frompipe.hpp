//
// file:        pitz/daq/data/engine/frompipebase.hpp
//

#ifndef __pitz_daq_data_engine_pipe_hpp__
#define __pitz_daq_data_engine_pipe_hpp__

#include <pitz/daq/data/engine/frompipebase.hpp>
#include <common/common_unnamedsemaphorelite.hpp>
#include <cpp11+/thread_cpp11.hpp>
#include <signal.h>


namespace pitz{ namespace daq { namespace data { namespace engine{


class FromPipe : public engine::FromPipeBase
{
public:
    FromPipe( );
    ~FromPipe();

    int  Initialize() __OVERRIDE__ __FINAL__;
    void Cleanup() __OVERRIDE__ __FINAL__;

protected:
    int  StartPipes(const ::std::string & branchNamesAll, const char* rootFileName) __OVERRIDE__ __FINAL__;
    void StopPipes() __OVERRIDE__ __FINAL__;

    void PidWaitThread();

protected:
    ::common::listN::Fifo<int>      m_fifoOfPids;
    ::common::UnnamedSemaphoreLite  m_semaForWait;
    ::STDN::thread                  m_threadWatcher;
    struct sigaction                m_sigusr1Initial;
    volatile int                    m_watcherRun;
    pthread_t                       m_mainThreadHandle;
    int                             m_pidCurrent;  // stupid way
};


}}}}  // namespace pitz{ namespace daq { namespace data { namespace engine{


#endif  // #ifndef __pitz_daq_data_engine_pipe_hpp__


