//
// file:        mex_simple_root_reader.cpp
//

#include "pitz/daq/data/engine/frompipe.hpp"
#include <string.h>
#include "bin_for_mexdaq_browser_common.h"
#include <stdio.h>
#ifdef _WIN32
#else
#include <sys/wait.h>
//#include <unistd.h>
#endif

#define HELPER_BINARY_NAME  "bindaq_browser_actual"
//#define HELPER_BINARY_NAME  "bindaq_browser.3.2.0"
//#define HELPER_BINARY_NAME  "bindaq_browser.3.2.3"
//#define HELPER_BINARY_NAME  "bindaq_browser.4.1.0"

static void SignalHandler(int){}

using namespace pitz::daq;


data::engine::FromPipe::FromPipe(  )
    :
      FromPipeBase(::read)
{
    m_pidCurrent = 0;
    m_watcherRun = 0;
}


data::engine::FromPipe::~FromPipe()
{
}


int data::engine::FromPipe::StartPipes(const ::std::string & a_branchNamesAll, const char* a_rootFileName)
{
    int vnPipesData[2], vnPipesCtrl[2], vnPipesInfo[2];
    int nPid;

    if(m_nPipesDataRd>=0){
        MAKE_WARNING_THIS("Pipe is not null!");
        return -1;
    }

    pipe(vnPipesData);
    pipe(vnPipesCtrl);
    pipe(vnPipesInfo);

    m_nPipesDataRd = vnPipesData[0];
    m_nPipesCtrlRd = vnPipesCtrl[0];
    m_nPipesInfoRd = vnPipesInfo[0];

    nPid = fork();

    if(nPid){
        close(vnPipesInfo[1]);close(vnPipesCtrl[1]);close(vnPipesData[1]);
        m_fifoOfPids.AddElement(nPid);
        m_semaForWait.post();
        return nPid;
    }
    else{
        char* argv[16];
        int nNumOfArgs;
        char vectPipes[128];
        char vcLogLevel[32];

        close(vnPipesInfo[0]);close(vnPipesCtrl[0]);close(vnPipesData[0]);
        snprintf(vectPipes,127,"%d,%d,%d",vnPipesData[1],vnPipesCtrl[1],vnPipesInfo[1]);
        snprintf(vcLogLevel,127,"%d",(int)log::g_nLogLevel);

        argv[0] = const_cast<char*>(HELPER_BINARY_NAME);
        argv[1] = const_cast<char*>(OPTION_NAME_USE_PRIV_PIPE);
        argv[2]=vectPipes;

        if(this->m_clbkType == callbackN::Type::MultiEntries){
            char vcStartTime[32], vcEndTime[32];

            argv[3] = const_cast<char*>(OPTION_NAME_ALL_DAQ_NAMES);
            argv[4] = const_cast<char*>(a_branchNamesAll.c_str());
            if( this->m_pFilter && (this->m_pFilter->start>0) && (this->m_pFilter->end>=this->m_pFilter->start) ){
                snprintf(vcStartTime,31,"%d",this->m_pFilter->start);
                snprintf(vcEndTime,31,"%d",this->m_pFilter->end);
                argv[5] = const_cast<char*>(TO_DO_GET_DATA_FOR_TIME_INTERVAL);
                argv[6] = const_cast<char*>(OPTION_NAME_START_TIME);
                argv[7] = vcStartTime;
                argv[8] = const_cast<char*>(OPTION_NAME_END_TIME);
                argv[9] = vcEndTime;
                nNumOfArgs = 9;
            }
            else{
                argv[5] = const_cast<char*>(TO_DO_GET_DATA_FOR_ENTRIES);
                argv[6] = const_cast<char*>(OPTION_NAME_ROOT_FILE);
                argv[7] = const_cast<char*>(a_rootFileName);
                nNumOfArgs = 7;
            }
        }
        else if(this->m_clbkType == callbackN::Type::Info){
            argv[3] = const_cast<char*>(TO_DO_GET_FILE_ENTRIES);
            argv[4] = const_cast<char*>(OPTION_NAME_ROOT_FILE);
            argv[5] = const_cast<char*>(a_rootFileName);
            nNumOfArgs = 5;
        }
        else{
            nPid = -1;
            //MAKE_ERROR_THIS("wrong option!");
            exit(1);
        }

        argv[++nNumOfArgs] = const_cast<char*>(OPTION_NAME_LOG_LEVEL);
        argv[++nNumOfArgs] =vcLogLevel;
        argv[++nNumOfArgs] = NULL;
        if(m_isDebug){argv[nNumOfArgs] = const_cast<char*>(OPTION_NAME_WAIT_FOR_DEBUG);}
        argv[++nNumOfArgs] = NULL;

        MAKE_REPORT_GLOBAL(2,"fl:%s, line:%d, nNumOfArgs=%d",__FILE__,__LINE__,nNumOfArgs);
        for(int i(0);i<nNumOfArgs+1;++i){
            MAKE_REPORT_GLOBAL(3,"argv[%d]=%s",i,argv[i]?argv[i]:"null");
        }

        execvp (argv[0], argv);

    }

    return 0;
}


void data::engine::FromPipe::StopPipes()
{
    if(m_nPipesInfoRd>=0){close(m_nPipesInfoRd);}
    if(m_nPipesCtrlRd>=0){close(m_nPipesCtrlRd);}
    if(m_nPipesDataRd>=0){close(m_nPipesDataRd);}
    m_nPipesDataRd = m_nPipesCtrlRd = m_nPipesInfoRd = -1;
}


int data::engine::FromPipe::Initialize()
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
    m_threadWatcher = ::STDN::thread(&FromPipe::PidWaitThread,this);
    m_isInited = 1;

    return 0;
}


void data::engine::FromPipe::Cleanup()
{
    if(!m_isInited){return;}
    m_watcherRun = 0;
    //pthread_kill(m_threadWatcher.native_handle(),SIGUSR1);//m_watcherThreadHandle
    m_semaForWait.post();
    if(m_pidCurrent){usleep(100000);if(m_pidCurrent){kill(m_pidCurrent,SIGKILL);}}
    m_threadWatcher.join();
    sigaction(SIGUSR1,&m_sigusr1Initial,NULL);
    m_isInited = 0;
}


void data::engine::FromPipe::PidWaitThread()
{
    int nPid;
    int nStatus;
    int nWaitResult;

    MAKE_REPORT_THIS(1,"PidWaitThread: ");

    while(m_watcherRun){
        m_semaForWait.wait();
        while(m_fifoOfPids.Extract(&nPid) && m_watcherRun){

            m_pidCurrent = nPid;
            nWaitResult = 0;
            while(nWaitResult>=0){
                nWaitResult=waitpid( nPid, &nStatus, WUNTRACED | WCONTINUED );

                if(WIFEXITED(nStatus) && (nWaitResult>=0)){
                    m_nWork2=0;
                    usleep(1000000);
                    if(m_isRunning){pthread_kill(m_mainThreadHandle,SIGUSR1);m_isRunning=0;}
                    break;
                }
            }  // while(nWaitResult>=0){
            m_nWork2=0;
            m_pidCurrent = 0;

        }  // write(s_vnPipesCtrl[1],&nReturn,4);
    }  // while(s_nThreadMustRun){

    m_pidCurrent = 0;

    while(m_fifoOfPids.Extract(&nPid)){
        kill(nPid,SIGKILL);
        nWaitResult = 0;
        while(nWaitResult>=0){nWaitResult=waitpid( nPid, &nStatus, WUNTRACED | WCONTINUED );}
    }  // write(s_vnPipesCtrl[1],&nReturn,4);
}
