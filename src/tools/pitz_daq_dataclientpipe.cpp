//
// file:        mex_simple_root_reader.cpp
//

#include "pitz/daq/dataclientpipe.hpp"
#include "bin_for_mexdaq_browser_common.h"
#include <stdio.h>

#ifdef _WIN32
#else
#include <sys/wait.h>
#endif

#define HELPER_BINARY_NAME  "bindaq_browser_actual"
//#define HELPER_BINARY_NAME  "bindaq_browser.3.2.0"
//#define HELPER_BINARY_NAME  "bindaq_browser.3.2.3"

static void SignalHandler(int){}


/***********************************************************************************************************************/
#if 0
int         m_nPipesData;
int         m_nPipesCtrl;
int         m_nPipesInfo;
#endif
pitz::daq::dataClient::PipeClient::PipeClient()
    :
      PipeClientBase( &::read )
{
    m_nPipesDataWr = m_nPipesCtrlWr = m_nPipesInfoWr = -1;
    m_watcherRun = 0;
}


pitz::daq::dataClient::PipeClient::~PipeClient()
{
    this->ClosePipes();
    this->Cleanup();
}


int pitz::daq::dataClient::PipeClient::OpenPipesForTime(const ::std::string & a_branchNamesAll, int a_startTime, int a_endTime)
{
    char* argv[16];
    int vnPipesData[2], vnPipesCtrl[2], vnPipesInfo[2];
    int nPid;

    if(m_nPipesDataWr>=0){
        MAKE_WARNING_THIS("Pipe is not null!");
        return -1;
    }

    pipe(vnPipesData);
    pipe(vnPipesCtrl);
    pipe(vnPipesInfo);

    m_nPipesData = vnPipesData[0];
    m_nPipesCtrl = vnPipesCtrl[0];
    m_nPipesInfo = vnPipesInfo[0];

    m_nPipesDataWr = vnPipesData[1];
    m_nPipesCtrlWr = vnPipesCtrl[1];
    m_nPipesInfoWr = vnPipesInfo[1];

    nPid = fork();

    if(nPid){
        int nStatus;
        read(vnPipesCtrl[0],&nStatus,4);
        m_fifoOfPids.AddElement(nPid);
        m_semaForWait.post();
        return nPid;
    }
    else{
        char vectPipes[128];
        char vcStartTime[32], vcEndTime[32];

        snprintf(vcStartTime,31,"%d",a_startTime);
        snprintf(vcEndTime,31,"%d",a_endTime);

        snprintf(vectPipes,127,"%d,%d,%d,%d,%d,%d",vnPipesData[1],vnPipesCtrl[1],vnPipesData[0],vnPipesCtrl[0],vnPipesInfo[1],vnPipesInfo[0]);

        argv[0] = const_cast<char*>(HELPER_BINARY_NAME);
        argv[1] = const_cast<char*>(OPTION_NAME_USE_PRIV_PIPE);
        argv[2]=vectPipes;

        argv[3] = const_cast<char*>(TO_DO_GET_DATA_FOR_TIME_INTERVAL);
        argv[4] = const_cast<char*>(OPTION_NAME_START_TIME);
        argv[5] = vcStartTime;

        argv[6] = const_cast<char*>(OPTION_NAME_ALL_DAQ_NAMES);
        argv[7] = const_cast<char*>(a_branchNamesAll.c_str());
        argv[8] = const_cast<char*>(OPTION_NAME_END_TIME);
        argv[9] = vcEndTime;
        argv[10] = NULL;
        if(m_isDebug){argv[10] = const_cast<char*>(OPTION_NAME_WAIT_FOR_DEBUG);}
        argv[11] = NULL;

        execvp (argv[0], argv);
        return -1; // should not return
    }

    return 0;

}


void pitz::daq::dataClient::PipeClient::ClosePipes()
{
    // todo:
    if(m_nPipesDataWr>=0){
        close(m_nPipesData);
        close(m_nPipesCtrl);
        close(m_nPipesInfo);
        close(m_nPipesDataWr);
        close(m_nPipesCtrlWr);
        close(m_nPipesInfoWr);

        m_nPipesData = m_nPipesCtrl = m_nPipesInfo = -1;
        m_nPipesDataWr = m_nPipesCtrlWr = m_nPipesInfoWr = -1;
    }

}


int pitz::daq::dataClient::PipeClient::Initialize()
{
    struct sigaction  sigusr1Action;

    MAKE_REPORT_THIS(0,"version 3.2.0.");
    sigusr1Action.sa_flags = 0;
    sigemptyset(&sigusr1Action.sa_mask);
    sigusr1Action.sa_restorer = NULL;
    sigusr1Action.sa_handler = &SignalHandler;
    sigaction(SIGUSR1,&sigusr1Action,&m_sigusr1Initial);
    m_mainThreadHandle = pthread_self();
    m_watcherRun = 1;
    m_threadWatcher = ::STDN::thread(&PipeClient::PidWaitThread,this);
    m_isInited = 1;

    return 0;
}


void pitz::daq::dataClient::PipeClient::Cleanup()
{
    if(!m_isInited){return;}
    m_watcherRun = 0;
    m_semaForWait.post();
    m_threadWatcher.join();
    sigaction(SIGUSR1,&m_sigusr1Initial,NULL);
    m_isInited = 0;
}


void pitz::daq::dataClient::PipeClient::PidWaitThread()
{
    int nPid;
    int nStatus;
    int nWaitResult;

    MAKE_REPORT_THIS(1,"PidWaitThread: ");

    while(m_watcherRun){
        m_semaForWait.wait();
        while(m_fifoOfPids.Extract(&nPid) && m_watcherRun){

            nWaitResult = 0;
            while(nWaitResult>=0){
                nWaitResult=waitpid( nPid, &nStatus, WUNTRACED | WCONTINUED );

                if(WIFEXITED(nStatus) && (nWaitResult>=0)){
                    m_nWork=0;
                    usleep(300000);
                    pthread_kill(m_mainThreadHandle,SIGUSR1);
                    break;
                }
            }  // while(nWaitResult>=0){

        }  // write(s_vnPipesCtrl[1],&nReturn,4);
    }  // while(s_nThreadMustRun){
}
