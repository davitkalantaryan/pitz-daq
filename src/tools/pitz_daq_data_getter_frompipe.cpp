//
// file:        mex_simple_root_reader.cpp
//

#include "pitz/daq/data/getter/frompipe.hpp"
#include <string.h>
#include "bin_for_mexdaq_browser_common.h"
#include <stdio.h>
#include <common/newlockguards.hpp>
#ifdef _WIN32
#else
#include <sys/wait.h>
//#include <unistd.h>
#endif

static void SignalHandler(int){}

using namespace pitz::daq;


data::getter::FromPipe::FromPipe(  )
    :
      getter::BaseTmp<getter::NoIndexing, engine::ByPipe>(::read)
{
    struct sigaction  sigusr1Action;

    MAKE_REPORT_THIS(0,"version 3.2.0.");
    sigusr1Action.sa_flags = 0;
    sigemptyset(&sigusr1Action.sa_mask);
    sigusr1Action.sa_restorer = NULL;
    sigusr1Action.sa_handler = &SignalHandler;
    sigaction(SIGUSR1,&sigusr1Action,&m_sigusr1Initial);
    m_mainThreadHandle = pthread_self();
    m_waiterThreadHandle = (pthread_t)0;
    m_isWork = 1;
    m_isWatcherRuns = 0;
    m_isWaitForProcesses = 1;
    m_threadWaiter = ::STDN::thread(&FromPipe::PidWaitThread,this);
    m_isInited = 1;

}


data::getter::FromPipe::~FromPipe()
{
    ::common::listN::ListItem<int>* pPidItem;
    ::common::NewSharedLockGuard< ::STDN::shared_mutex > aSharedGuard;

    if(!m_isInited){return;}
    m_isWork = 0;
    //pthread_kill(m_threadWatcher.native_handle(),SIGUSR1);//m_watcherThreadHandle//m_waiterThreadHandle
    if(m_waiterThreadHandle){pthread_kill(m_waiterThreadHandle,SIGUSR1);}

    aSharedGuard.SetAndLockMutex(&m_mutexForPids);
    pPidItem = m_listOfPids.first();
    while(pPidItem){
        kill(pPidItem->data,SIGKILL);
        pPidItem = pPidItem->next;
    }
    aSharedGuard.UnsetAndUnlockMutex();

    m_threadWaiter.join();
    sigaction(SIGUSR1,&m_sigusr1Initial,NULL);
    m_isInited = 0;
}


int data::getter::FromPipe::CreateGetterAndPipes( const char* a_rootFileName, const ::std::vector< ::std::string > * a_pBranchNames)
{
    int vnPipes[2];
    int nIndex,nPid;
    const int cnNumberOfPipes((int)data::byPipe::pipePurpose::Count);
    int vnPipesRd[cnNumberOfPipes], vnPipesWr[cnNumberOfPipes];


    if(m_engine.IsPipesSet()){
        MAKE_WARNING_THIS("Pipe is not null!");
        return -1;
    }

    for(nIndex=0;nIndex< cnNumberOfPipes;++nIndex){
        pipe(vnPipes);
        vnPipesRd[nIndex]=vnPipes[0];
        vnPipesWr[nIndex]=vnPipes[1];
    }

    nPid = fork();

    if(nPid){
        ::common::NewLockGuard< ::STDN::shared_mutex > aGuard;

        aGuard.SetAndLockMutex(&m_mutexForPids);
        m_listOfPids.AddData(nPid);
        aGuard.UnsetAndUnlockMutex();

        for(nIndex=0;nIndex<byPipe::pipePurpose::Count;++nIndex){close(vnPipesWr[nIndex]);}
        m_engine.SetPipes(vnPipesRd);
        return nPid;
    }
    else{
        char* argv[32];
        int nOffSet(0);
        int nNumOfArgs=0;
        char vectPipes[128];
        char vcLogLevel[32];

        m_isWaitForProcesses = 0;
        m_isWork = 0;
        SleepMsInt(1);
        m_listOfPids.clear();

        //close(vnPipesInfo[0]);close(vnPipesCtrl[0]);close(vnPipesData[0]);
        //snprintf(vectPipes,127,"%d,%d,%d",vnPipesData[1],vnPipesCtrl[1],vnPipesInfo[1]);
        for(nIndex=0;nIndex<byPipe::pipePurpose::Count;++nIndex){
            close(vnPipesRd[nIndex]);
            nOffSet += (snprintf(vectPipes+nOffSet,128-nOffSet,"%d, ",vnPipesWr[nIndex])-1);
        }


        snprintf(vcLogLevel,127,"%d",(int)log::g_nLogLevel);

        argv[0] = const_cast<char*>(HELPER_BINARY_NAME);

        if(m_isDebug){argv[1] = const_cast<char*>(OPTION_NAME_WAIT_FOR_DEBUG);}
        else{argv[1] = const_cast<char*>(OPTION_NAME_DUMMY);}

        argv[2] = const_cast<char*>(OPTION_NAME_USE_PRIV_PIPE);
        argv[3]=vectPipes;

        argv[4] = const_cast<char*>(OPTION_NAME_LOG_LEVEL);
        argv[5] =vcLogLevel;

        switch(m_filter.type){
        case filter::Type::FileEntriesInfo:
            argv[6] = const_cast<char*>(TO_DO_GET_FILE_ENTRIES);
            argv[7] = const_cast<char*>(OPTION_NAME_ROOT_FILE);
            argv[8] = const_cast<char*>(a_rootFileName);
            argv[9]=NULL;
            nNumOfArgs = 9;
            break;
        case filter::Type::MultyBranchFromFile:
        {

            if(a_pBranchNames){
                const ::std::vector< ::std::string > & a_branchNames = *a_pBranchNames;
                ::std::string strAllBranches;
                const int cnNumberOfBranchesMin1((int)a_branchNames.size()-1);
                int i;

                if (cnNumberOfBranchesMin1 < 0) { return -1; }
                for (i = 0; i < cnNumberOfBranchesMin1; ++i) {
                    strAllBranches += a_branchNames[i].c_str();
                    strAllBranches += ";";
                }strAllBranches += a_branchNames[cnNumberOfBranchesMin1].c_str();

                argv[6] = const_cast<char*>(TO_DO_GET_DATA_FOR_ENTRIES);

                argv[7] = const_cast<char*>(OPTION_NAME_ALL_DAQ_NAMES);
                argv[8] = const_cast<char*>(strAllBranches.c_str());

                argv[9] = const_cast<char*>(OPTION_NAME_ROOT_FILE);
                argv[10] = const_cast<char*>(a_rootFileName);

                argv[11] = NULL;
                nNumOfArgs = 10;
            }
            else{
                MAKE_ERROR_THIS(" ");
                return -1;
            }

        }
        break;
        case filter::Type::ByTime2:
        {
            if(a_pBranchNames){
                const ::std::vector< ::std::string > & a_branchNames = *a_pBranchNames;
                char vcStartTime[32], vcEndTime[32];
                ::std::string strAllBranches;
                const int cnNumberOfBranchesMin1((int)a_branchNames.size()-1);
                int i;

                if (cnNumberOfBranchesMin1 < 0) { return -1; }
                for (i = 0; i < cnNumberOfBranchesMin1; ++i) {
                    strAllBranches += a_branchNames[i].c_str();
                    strAllBranches += ";";
                }strAllBranches += a_branchNames[cnNumberOfBranchesMin1].c_str();

                snprintf(vcStartTime,31,"%d",this->m_filter.start);
                snprintf(vcEndTime,31,"%d",this->m_filter.end);

                argv[6] = const_cast<char*>(TO_DO_GET_DATA_FOR_TIME_INTERVAL);

                argv[7] = const_cast<char*>(OPTION_NAME_ALL_DAQ_NAMES);
                argv[8] = const_cast<char*>(strAllBranches.c_str());

                argv[9] = const_cast<char*>(OPTION_NAME_START_TIME);
                argv[10] = vcStartTime;
                argv[11] = const_cast<char*>(OPTION_NAME_END_TIME);
                argv[12] = vcEndTime;

                argv[13] = NULL;
                nNumOfArgs = 12;
            }
            else{
                MAKE_ERROR_THIS(" ");
                return -1;
            }

        }
        break;
        case filter::Type::ByEvent2:
            MAKE_WARNING_THIS("ByTime2 is not implemented yet!");
            return 0;
        default:
            MAKE_ERROR_THIS("Wrong option for query!");
            return -1;
        }


        MAKE_REPORT_GLOBAL(2,"fl:%s, line:%d, nNumOfArgs=%d",__FILE__,__LINE__,nNumOfArgs);
        for(int i(0);i<nNumOfArgs+1;++i){
            MAKE_REPORT_GLOBAL(3,"argv[%d]=%s",i,argv[i]?argv[i]:"null");
        }

        execvp (argv[0], argv);
        return -1;

    }

    return 0;
}


void data::getter::FromPipe::disconnet()
{
    if(m_engine.IsPipesSet()){
        const int* pcPipes = m_engine.pipes();
        const int cnNumberOfPipes((int)data::byPipe::pipePurpose::Count);
        int vcNewPipes[cnNumberOfPipes];

        for(int i(0);i< cnNumberOfPipes;++i){vcNewPipes[i]=-1;if (pcPipes[i] >= 0){close(pcPipes[i]);}}
        m_engine.SetPipes(vcNewPipes);
    }
}


int data::getter::FromPipe::GetEntriesInfo(const char* a_rootFileName)
{
    int nReturn(0);

    SetFilter(filter::Type::FileEntriesInfo);
    nReturn=CreateGetterAndPipes(a_rootFileName); if(nReturn<0){goto returnPoint;}

    nReturn = getter::BaseTmp<getter::NoIndexing, engine::ByPipe>::GetEntriesInfo(a_rootFileName);

returnPoint:
    SetFilter(filter::Type::NoFilter2);
    return nReturn;
}


int data::getter::FromPipe::GetMultipleEntries(const char* a_rootFileName, const ::std::vector< ::std::string >& a_branchNames)
{
    int nReturn(0);

    SetFilter(filter::Type::MultyBranchFromFile);
    nReturn=CreateGetterAndPipes(a_rootFileName,&a_branchNames); if(nReturn<0){goto returnPoint;}

    nReturn = getter::BaseTmp<getter::NoIndexing, engine::ByPipe>::GetMultipleEntries(a_rootFileName,a_branchNames);

returnPoint:
    SetFilter(filter::Type::NoFilter2);
    return nReturn;
}


int data::getter::FromPipe::GetMultipleEntriesTI(const ::std::vector< ::std::string >& a_branchNames, int a_startTime, int a_endTime)
{
    int nReturn(0);

    SetFilter(filter::Type::ByTime2,a_startTime,a_endTime);
    nReturn=CreateGetterAndPipes("",&a_branchNames); if(nReturn<0){goto returnPoint;}

    nReturn = getter::BaseTmp<getter::NoIndexing, engine::ByPipe>::GetMultipleEntriesTI(a_branchNames,a_startTime,a_endTime);

returnPoint:
    SetFilter(filter::Type::NoFilter2);
    return nReturn;
}


void data::getter::FromPipe::PidWaitThread()
{
    ::common::NewLockGuard< ::STDN::shared_mutex > aGuard;
    ::common::NewLockGuard< ::STDN::shared_mutex > aSharedGuard;
    ::common::listN::ListItem<int>* pItem;
    int nStatLoc;
    int nPid;
    bool bWaited;

    m_waiterThreadHandle = pthread_self();
    m_isWatcherRuns = 1;
    MAKE_REPORT_THIS(1,"PidWaitThread: ");

enterTryPoint:
    try{
        while(m_isWork && m_isWaitForProcesses){

            SleepMsInt(5000);

            bWaited = false;
            aGuard.SetAndLockMutex(&m_mutexForPids);
            if(m_listOfPids.first()){
                nPid=waitpid( m_listOfPids.first()->data, &nStatLoc, WUNTRACED | WCONTINUED | WNOHANG );
                if(nPid<0){continue;}
                if ( WIFEXITED(nStatLoc)||WIFSIGNALED(nStatLoc) ){
                    m_listOfPids.RemoveData(m_listOfPids.first());
                    bWaited = true;
                }
            }
            aGuard.UnsetAndUnlockMutex();

            if(bWaited){
                m_engine.Stop();
                SleepMsInt(1000);
                if(m_engine.isRunning()){pthread_kill(m_mainThreadHandle,SIGUSR1);}
            }

        }  // while(m_isWork && m_isWaitForProcesses){


        aSharedGuard.SetAndLockMutex(&m_mutexForPids);
        pItem=m_listOfPids.first();
        while(m_isWaitForProcesses && pItem){

            do {
                nPid= waitpid(pItem->data, &nStatLoc, WUNTRACED | WCONTINUED);
                if(nPid==-1){goto nextItemPoint;}
            } while (!WIFEXITED(nStatLoc) && !WIFSIGNALED(nStatLoc));

nextItemPoint:
            pItem = pItem->next;

        }  // write(s_vnPipesCtrl[1],&nReturn,4);
        aSharedGuard.UnsetAndUnlockMutex();
    }
    catch(...){
        goto enterTryPoint;
    }

    m_isWatcherRuns = 0;
}
