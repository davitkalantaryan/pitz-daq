//
// file:        mex_simple_root_reader.cpp
//

#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif


#include <mex.h>
//#include "tool/formexdaq_browser_dynuser.h"
#include "tool/simple_root_reader.h"
#include <stdint.h>
#include <signal.h>
#include <cpp11+/thread_cpp11.hpp>
#include <common/common_unnamedsemaphorelite.hpp>
#include <setjmp.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "bin_for_mexdaq_browser_common.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <common/common_unnamedsemaphorelite.hpp>
#include <common/lists.hpp>
#include <signal.h>
#include <vector>
#include <string.h>
#include <string>


#define FNAME_BUF_LEN_MIN1  127
#define MATLAB_DEBUG(...) do{mexPrintf(__VA_ARGS__);mexPrintf("\n");mexEvalString("drawnow");}while(0)

//#define HELPER_BINARY_NAME  "bindaq_browser_actual"
//#define HELPER_BINARY_NAME  "bindaq_browser.3.2.0"
#define HELPER_BINARY_NAME  "bindaq_browser.3.2.3"

#define HANDLE_MEM_DEF(_pointer,...)  do{if(!(_pointer)){mexErrMsgTxt(" ");}}while(0)

typedef struct SAddInfoForSearch{
    int32_t   numberOfEvents;
    uint32_t  memorySize;
    uint16_t  isBeginFound: 1;
    uint16_t  isComplete: 1;
    structBranchDataInfo info;
    SAddInfoForSearch():numberOfEvents(0),memorySize(0){this->isBeginFound=this->isComplete=0;}
}SAddInfoForSearch;

typedef struct DataForMultiRootMex
{
    structCommonNew forReading;
    ::std::vector< ::std::vector<structCommonNew> >& vectorForData;
    ::std::vector<SAddInfoForSearch>& addInfo;
    int maxEventsCount;
    int numberOfBranches;
    // todo:
    // int firstEventNumber, lastEventNumber; // instead of int maxEventsCount;

    /*--------------------------------*/
    DataForMultiRootMex( ::std::vector< ::std::vector<structCommonNew> >& a_vectorForData, std::vector<SAddInfoForSearch>& a_addInfo)
        :vectorForData(a_vectorForData),addInfo(a_addInfo),numberOfBranches(0)
    {
        this->maxEventsCount = 0;

    }

    void setBranchsCount(int a_nNumberOfBranches)
    {
        this->numberOfBranches = a_nNumberOfBranches;
        this->vectorForData.resize(a_nNumberOfBranches);
        this->addInfo.resize(a_nNumberOfBranches);
    }

}DataForMultiRootMex;


static void PrintHelp(void);
static double GetNumericData(const mxArray* a_numeric);
static int StartChildAndWaitReady(char* a_argv[]);
static int  WaitForDataOrControl();
static bool MultiRootReaderHandlerMex(DataForMultiRootMex* a_pClbkData, int pipe);
static int DataToMatlab(const structCommonNew& a_data, mxArray* a_dataMaytlab, int nIndex,
                        int type_field,int time_field,int event_field,int isAvailable_field,int data_field);
static void WaitThreadFunction(void);
static void MexCleanupFunction(void);
static void SignalHandler(int){}

static int s_vnPipesData2[2];
static int s_vnPipesCtrl[2];
static int s_vnPipesInfo[2];

#define PID_TABLE_SIZE      4

::common::UnnamedSemaphoreLite  s_semaForWaitThread;
::common::listN::Fifo<int>  s_fifoPidsToWait;
static int s_vnPids[PID_TABLE_SIZE];

static pthread_t s_nMatlabMainThreadHandle = (pthread_t)0;
static int s_nLibraryStarted = 0;
static volatile int s_nThreadMustRun = 0;
::STDN::thread*  s_pThreadForWait = NULL;
static struct sigaction  s_sigusr1Initial;
static int s_nLogLevel=0;

#ifndef DEBUG_ROOT_APP
#define DEBUG_ROOT_APP(_logLevel,...)   do{if((_logLevel)<=s_nLogLevel){mexPrintf(__VA_ARGS__);}}while(0)
#endif

char* argv[16];


void mexFunction(int a_nNumOuts, mxArray *a_Outputs[],
    int a_nNumInps, const mxArray*a_Inputs[])
{
    static const char* svcpcFieldNames[] = {"type","time","event","isAvailable","data"};
    static const int scnNumberOfFields = sizeof(svcpcFieldNames)/sizeof(const char*);
    int nStrLen;
    int nPid2=0, nPidIndex=0;
    char *pcOption;
    //char* argv[16];

    if(!s_nLibraryStarted){
        struct sigaction  sigusr1Action;

        mexPrintf("version 3.2.0. numberOfInputs=%d, inp=%p, numberOfOutputs=%d, outp=%p\n",
                  a_nNumInps,a_Inputs,a_nNumOuts,a_Outputs);
        sigusr1Action.sa_flags = 0;
        sigemptyset(&sigusr1Action.sa_mask);
        sigusr1Action.sa_restorer = NULL;
        sigusr1Action.sa_handler = &SignalHandler;
        sigaction(SIGUSR1,&sigusr1Action,&s_sigusr1Initial);
        s_nMatlabMainThreadHandle = pthread_self();
        mexAtExit(&MexCleanupFunction);
        s_nThreadMustRun = 1;
        s_pThreadForWait = new ::STDN::thread(&WaitThreadFunction);
        s_nLibraryStarted = 1;
    }

    if( (a_nNumInps<1)||(!mxIsChar(a_Inputs[0])) ){
        mexPrintf("Provide the option!\n");
        PrintHelp();
        goto returnPoint;
    }

    s_vnPipesData2[0]=s_vnPipesData2[1]=s_vnPipesCtrl[0]=s_vnPipesCtrl[1]=s_vnPipesInfo[0]=s_vnPipesInfo[1]=-1;

    nStrLen=(int)(mxGetM(a_Inputs[0])*mxGetN(a_Inputs[0]));
    pcOption=(char*)alloca(nStrLen+4);
    mxGetString(a_Inputs[0],pcOption,nStrLen+1);

    if(strcmp("--help",pcOption)==0){PrintHelp();goto returnPoint;}
    else if(strcmp("--get-log-level",pcOption)==0){
        mexPrintf("log-level=%d\n",s_nLogLevel);
        goto returnPoint;
    }
    else if(strcmp("--set-log-level",pcOption)==0){
        if((a_nNumInps<2)||(!mxIsNumeric(a_Inputs[1]))){
            mexPrintf("logLevel is not provided!\n");
            goto returnPoint;
        }
        s_nLogLevel= (int)GetNumericData(a_Inputs[1]);
        goto returnPoint;
    }

    argv[0] = const_cast<char*>(HELPER_BINARY_NAME);
    //argv[1] = const_cast<char*>(OPTION_NAME_USE_PIPE);
    argv[1] = const_cast<char*>(OPTION_NAME_USE_PRIV_PIPE);
    //argv[2]="pipe1/pipe2";

    if(strcmp("--get-file-entries",pcOption)==0){
        char *cpcRootFileName;
        std::string daqEntryName;
        std::vector<std::string> vectorNames;
        size_t i, unVectorSize;
        //int nOptionStrLenPlus1;
        int nEntryNameLen;

        if( (a_nNumInps<2)||(!mxIsChar(a_Inputs[1]))  ){
            mexPrintf("Provide the name of the file!\n");
            goto returnPoint;
        }

        nStrLen=(int)(mxGetM(a_Inputs[1])*mxGetN(a_Inputs[1]));
        cpcRootFileName=(char*)alloca(nStrLen+4);
        mxGetString(a_Inputs[1],cpcRootFileName,nStrLen+1);

        argv[3] = const_cast<char*>(TO_DO_GET_FILE_ENTRIES);

        argv[4] = const_cast<char*>(OPTION_NAME_ROOT_FILE);
        argv[5] = cpcRootFileName;
        argv[6] = NULL;

        nPid2=StartChildAndWaitReady(argv);
        if(nPid2<=0){goto returnPoint;}

        nPidIndex = nPid2%PID_TABLE_SIZE;
        s_vnPids[nPidIndex]=nPid2;
        s_fifoPidsToWait.AddElement(nPid2);
        s_semaForWaitThread.post();

        while((s_vnPids[nPidIndex]==nPid2)&&WaitForDataOrControl()){
            read(s_vnPipesData2[0],&nEntryNameLen,4);
            if(nEntryNameLen<1){continue;}
            daqEntryName.resize(nEntryNameLen+1,'\0');
            read(s_vnPipesData2[0],const_cast<char*>(daqEntryName.data()),nEntryNameLen);
            vectorNames.push_back(daqEntryName);
        }

        unVectorSize = vectorNames.size();

        if(a_nNumOuts){
            mxArray *pDaqName;
            a_Outputs[0] = mxCreateCellMatrix(1,unVectorSize);
            HANDLE_MEM_DEF(a_Outputs[0]);
            for(i=0;i<unVectorSize;++i){
                pDaqName =   mxCreateString(vectorNames[i].c_str());
                HANDLE_MEM_DEF(pDaqName);
                mxSetCell(a_Outputs[0],i,pDaqName);
            }
        }
        else{
            for(i=0;i<unVectorSize;++i){
                mexPrintf("%s\n",vectorNames[i].c_str());
            }
        }

        goto returnPoint;
    }
    else if(strcmp("--get-data-for-entry",pcOption)==0){
        mexPrintf("Option:\"--get-data-for-entry\" is not implemented currently!\n");
#if 0
        char* cpcTreeAndBranchName;
        char *cpcRootFileName;
        int i, nDataLen, nMaxEvents;
        int type_field,time_field,event_field,isAvailable_field,data_field;
        structCommonNew bufForData;
        std::vector<structCommonNew> vectorOfData;

        if( (a_nNumInps<3)||(!mxIsChar(a_Inputs[2]))  ){
            mexPrintf("Provide root file  name and daq entry name!\n");
            goto returnPoint;
        }

        nStrLen=(int)(mxGetM(a_Inputs[1])*mxGetN(a_Inputs[1]));
        cpcRootFileName=(char*)alloca(nStrLen+4);
        mxGetString(a_Inputs[1],cpcRootFileName,nStrLen+1);

        nStrLen=(int)(mxGetM(a_Inputs[2])*mxGetN(a_Inputs[2]));
        cpcTreeAndBranchName=(char*)alloca(nStrLen+4);
        mxGetString(a_Inputs[2],cpcTreeAndBranchName,nStrLen+1);

        argv[3] = const_cast<char*>(TO_DO_GET_DATA_FOR_ENTRY);
        argv[4] = const_cast<char*>(OPTION_NAME_ROOT_FILE);
        argv[5] = cpcRootFileName;

        argv[6] = const_cast<char*>(OPTION_NAME_DAQ_NAME);
        argv[7] = cpcTreeAndBranchName;
        argv[8] = NULL;

        nPid2=StartChildAndWaitReady(argv);
        if(nPid2<=0){goto returnPoint;}

        nPidIndex = nPid2%PID_TABLE_SIZE;
        s_vnPids[nPidIndex]=nPid2;
        s_fifoPidsToWait.AddElement(nPid2);
        s_semaForWaitThread.post();

        while((s_vnPids[nPidIndex]==nPid2)&&WaitForDataOrControl()){
        //while(WaitForDataOrControl()){
            read(s_vnPipesData[0],&nDataLen,4);
            if(nDataLen<1){continue;}
            else if(nDataLen>((int)sizeof(structCommon))){nDataLen = (int)sizeof(structCommon);}
            read(s_vnPipesData[0],&bufForData,nDataLen);
            vectorOfData.push_back(bufForData);
        }

        nMaxEvents = (int)vectorOfData.size();
        if(nMaxEvents<1){goto returnPoint;}

        a_Outputs[0] = mxCreateStructMatrix(1,nMaxEvents,scnNumberOfFields,svcpcFieldNames);
        HANDLE_MEM_DEF(a_Outputs[0]);

        type_field = mxGetFieldNumber(a_Outputs[0],svcpcFieldNames[0]);
        time_field = mxGetFieldNumber(a_Outputs[0],svcpcFieldNames[1]);
        event_field = mxGetFieldNumber(a_Outputs[0],svcpcFieldNames[2]);
        isAvailable_field = mxGetFieldNumber(a_Outputs[0],svcpcFieldNames[3]);
        data_field = mxGetFieldNumber(a_Outputs[0],svcpcFieldNames[4]);

        for(i=0;i<nMaxEvents;++i){
            DataToMatlab(vectorOfData[i], a_Outputs[0], i,type_field,time_field, event_field,isAvailable_field,data_field);
        }
#endif
        goto returnPoint;
    }

    else if(strcmp("--get-data-for-time-interval",pcOption)==0){
        ::std::vector< ::std::vector<structCommonNew> > outVector;
        ::std::vector<SAddInfoForSearch> addInfo;
        mxArray *pBranchName;
        std::string daqNameS;
        std::string daqNameAll;
        int nPipe;
        int nStartTime, nEndTime;
        int nIndexBranch,nBranchCount, nBranchCountMin1;
        int nIndexEvent,nEventInBranchCount;
        int nIndexF;
        int type_field,time_field,event_field,isAvailable_field,data_field;
        char vcStartTime[32], vcEndTime[32];
        mwIndex vSubs[2];
        DataForMultiRootMex aClbkData(outVector,addInfo);
        bool bDebug(false);
        int nStatus;

        if( (a_nNumInps<4)||(!mxIsCell(a_Inputs[1])) ){
            mexPrintf("Provide daq entries names!\n");
            goto returnPoint;
        }

        nStartTime = GetNumericData(a_Inputs[2]);
        nEndTime = GetNumericData(a_Inputs[3]);

        snprintf(vcStartTime,31,"%d",nStartTime);
        snprintf(vcEndTime,31,"%d",nEndTime);

        nBranchCount =(int) mxGetNumberOfElements(a_Inputs[1]);
        DEBUG_ROOT_APP(1,"nBranchCount=%d, startTime=%d, endTime=%d",nBranchCount,nStartTime,nEndTime);
        if(nBranchCount<1){
            mexPrintf("No branch is provided!");
            goto returnPoint;
        }
        aClbkData.setBranchsCount(nBranchCount);

        nBranchCountMin1 = nBranchCount-1;

        for(nIndexBranch=0;nIndexBranch<nBranchCountMin1;++nIndexBranch){
            pBranchName = mxGetCell(a_Inputs[1],nIndexBranch);
            nStrLen = mxGetNumberOfElements(pBranchName);
            daqNameS.resize(nStrLen+4);
            mxGetString(pBranchName,const_cast<char*>(daqNameS.data()),nStrLen+1);
            DEBUG_ROOT_APP(0,"str=%s, nStrLen=%d",daqNameS.c_str(),nStrLen);
            daqNameAll += daqNameS.c_str();
            daqNameAll += ";";
        }

        pBranchName = mxGetCell(a_Inputs[1],nBranchCountMin1);
        nStrLen = mxGetNumberOfElements(pBranchName);
        daqNameS.resize(nStrLen+4);
        mxGetString(pBranchName,const_cast<char*>(daqNameS.data()),nStrLen+1);
        DEBUG_ROOT_APP(1,"str=%s, nStrLen=%d",daqNameS.c_str(),nStrLen);
        daqNameAll += daqNameS.c_str();

        if( (a_nNumInps>4) && mxIsChar(a_Inputs[4]) ){
            char vcIsDebug[16];
            mxGetString(a_Inputs[4],vcIsDebug,15);
            if(strncmp(vcIsDebug,"--debug-app",15)==0){bDebug=true;}
        }

        mexPrintf("daqNameAll=%s\ndebug=%s\n",daqNameAll.c_str(),bDebug?"true":"false");
        mexEvalString("drawnow");

        argv[3] = const_cast<char*>(TO_DO_GET_DATA_FOR_TIME_INTERVAL);
        argv[4] = const_cast<char*>(OPTION_NAME_START_TIME);
        argv[5] = vcStartTime;

        argv[6] = const_cast<char*>(OPTION_NAME_ALL_DAQ_NAMES);
        argv[7] = const_cast<char*>(daqNameAll.c_str());
        argv[8] = const_cast<char*>(OPTION_NAME_END_TIME);
        argv[9] = vcEndTime;
        argv[10] = NULL;
        if(bDebug){argv[10] = const_cast<char*>(OPTION_NAME_WAIT_FOR_DEBUG);}
        argv[11] = NULL;


        nPid2=StartChildAndWaitReady(argv);
        if(nPid2<=0){goto returnPoint;}
        read(s_vnPipesCtrl[0],&nStatus,4);

        nPidIndex = nPid2%PID_TABLE_SIZE;
        s_vnPids[nPidIndex]=nPid2;
        s_fifoPidsToWait.AddElement(nPid2);
        s_semaForWaitThread.post();


        while(s_vnPids[nPidIndex]==nPid2){
            nPipe = WaitForDataOrControl();
            if(nPipe<=0){break;}
            if(MultiRootReaderHandlerMex(&aClbkData,nPipe)){break;}
        }

        s_vnPids[nPidIndex]=0;

        DEBUG_ROOT_APP((aClbkData.maxEventsCount&&nBranchCount)?1:0,"nBranchCount=%d,nMaxEvents=%d\n",nBranchCount,(int)aClbkData.maxEventsCount);
        if(aClbkData.maxEventsCount<1){
            a_Outputs[0] = mxCreateStructMatrix(nBranchCount,aClbkData.maxEventsCount,scnNumberOfFields,svcpcFieldNames);
            goto returnPoint;
        }

        a_Outputs[0] = mxCreateStructMatrix(nBranchCount,aClbkData.maxEventsCount,scnNumberOfFields,svcpcFieldNames);
        HANDLE_MEM_DEF(a_Outputs[0]);

        type_field = mxGetFieldNumber(a_Outputs[0],svcpcFieldNames[0]);
        time_field = mxGetFieldNumber(a_Outputs[0],svcpcFieldNames[1]);
        event_field = mxGetFieldNumber(a_Outputs[0],svcpcFieldNames[2]);
        isAvailable_field = mxGetFieldNumber(a_Outputs[0],svcpcFieldNames[3]);
        data_field = mxGetFieldNumber(a_Outputs[0],svcpcFieldNames[4]);

        for(nIndexBranch=0;nIndexBranch<nBranchCount;++nIndexBranch){
            vSubs[0]=nIndexBranch;
            nEventInBranchCount = (int)outVector[nIndexBranch].size();
            for(nIndexEvent=0;nIndexEvent<nEventInBranchCount;++nIndexEvent){
                vSubs[1]=nIndexEvent;
                nIndexF = mxCalcSingleSubscript(a_Outputs[0],2,vSubs);
                DataToMatlab((outVector[nIndexBranch])[nIndexEvent], a_Outputs[0],nIndexF,type_field,time_field, event_field,isAvailable_field,data_field);
            }

        }  // for(nIndexBranch=0;nIndexBranch<nBranchCount;++nIndexBranch){

        goto returnPoint;

    }
    else {
        mexPrintf("Wrong option provided!\n");
        goto returnPoint;
    }



returnPoint:
    if(s_vnPipesData2[0]>0){close(s_vnPipesData2[0]);s_vnPipesData2[0]=-1;}
    if(s_vnPipesData2[1]>0){close(s_vnPipesData2[1]);s_vnPipesData2[1]=-1;}
    if(s_vnPipesCtrl[0]>0){close(s_vnPipesCtrl[0]);s_vnPipesCtrl[0]=-1;}
    if(s_vnPipesCtrl[1]>0){close(s_vnPipesCtrl[1]);s_vnPipesCtrl[1]=-1;}
    if(s_vnPipesInfo[0]>0){close(s_vnPipesInfo[0]);s_vnPipesInfo[0]=-1;}
    if(s_vnPipesInfo[1]>0){close(s_vnPipesInfo[1]);s_vnPipesInfo[1]=-1;}
    return;
}


static void MexCleanupFunction(void)
{
    if(s_pThreadForWait){
        s_nThreadMustRun = 0;
        s_semaForWaitThread.post();
        s_pThreadForWait->join();
        delete s_pThreadForWait;
        s_pThreadForWait = NULL;
        sigaction(SIGUSR1,&s_sigusr1Initial,NULL);
    }
    s_nLibraryStarted = 0;
}


static void WaitThreadFunction(void)
{
    int nPid2;
    int nPidIndex;
    int nStatus;
    int nWaitResult;

    dprintf(STDOUT_FILENO,"MATLAB: ln:%d: \n",__LINE__);

    while(s_nThreadMustRun){
        s_semaForWaitThread.wait();
        while(s_fifoPidsToWait.Extract(&nPid2) && s_nThreadMustRun){
            nPidIndex = nPid2%PID_TABLE_SIZE;

            nWaitResult = 0;
            while(nWaitResult>=0){
                nWaitResult=waitpid( nPid2, &nStatus, WUNTRACED | WCONTINUED );

                if(WIFEXITED(nStatus) && (nWaitResult>=0)){
                    s_vnPids[nPidIndex]=0;
                    usleep(300000);
                    pthread_kill(s_nMatlabMainThreadHandle,SIGUSR1);
                    break;
                }
            }  // while(nWaitResult>=0){

        }  // write(s_vnPipesCtrl[1],&nReturn,4);
    }  // while(s_nThreadMustRun){
}


static bool MultiRootReaderHandlerMex(DataForMultiRootMex* a_pClbkData, int a_nPipe)
{
    int nIndex;

    if(a_nPipe<=0){return true;}

    if( (read(a_nPipe,&nIndex,4)!=4) || (nIndex<0)||(nIndex>=a_pClbkData->numberOfBranches)){
        return true;
    }

    if(a_nPipe==s_vnPipesData2[0]){
        if(read(s_vnPipesData2[0],a_pClbkData->forReading.rawBuffer(),a_pClbkData->addInfo[nIndex].memorySize)!=a_pClbkData->addInfo[nIndex].memorySize){return true;}

        (a_pClbkData->vectorForData)[nIndex].push_back(a_pClbkData->forReading);
        ++a_pClbkData->addInfo[nIndex].numberOfEvents;

        if(a_pClbkData->addInfo[nIndex].numberOfEvents>a_pClbkData->maxEventsCount){
            a_pClbkData->maxEventsCount=a_pClbkData->addInfo[nIndex].numberOfEvents;
        }
    }
    else if(a_nPipe==s_vnPipesInfo[0]){
        if(read(s_vnPipesInfo[0],&(a_pClbkData->addInfo[nIndex].info),sizeof(structBranchDataInfo))!=sizeof(structBranchDataInfo)){return true;}
        a_pClbkData->forReading.setBranchInfo(a_pClbkData->addInfo[nIndex].info);
        a_pClbkData->addInfo[nIndex].memorySize=structCommonNew::getMemorySize(a_pClbkData->addInfo[nIndex].info);
    }

    return false;

}


static int s_nMaxDescrPlus1=0;
#define ERROR_RP_WHEN_NO_RUN    "3. unable to run binary: " HELPER_BINARY_NAME "\n"


static int StartChildAndWaitReady(char* a_argv[])
{
    int nPid;

    pipe(s_vnPipesData2);
    pipe(s_vnPipesCtrl);
    pipe(s_vnPipesInfo);
    nPid = fork();

    if(nPid){

        s_nMaxDescrPlus1 = (s_vnPipesData2[0]>s_vnPipesCtrl[0]) && (s_vnPipesData2[0]>s_vnPipesInfo[0]) ?
                    (s_vnPipesData2[0]+1) : ( s_vnPipesCtrl[0]>s_vnPipesInfo[0]?(s_vnPipesCtrl[0]+1):(s_vnPipesInfo[0]+1));

    }
    else{
        char vectPipes[128];
        snprintf(vectPipes,127,"%d,%d,%d,%d,%d,%d",s_vnPipesData2[1],s_vnPipesCtrl[1],s_vnPipesData2[0],s_vnPipesCtrl[0],s_vnPipesInfo[1],s_vnPipesInfo[0]);
        a_argv[2]=vectPipes;
        execvp (a_argv[0], a_argv);
        return -1; // should not return
    }

    return nPid;
    //return -1;
}


static int WaitForDataOrControl()
{
    fd_set rFds;
    fd_set eFds;
    int nSelectReturn;

    FD_ZERO(&rFds);
    FD_SET(s_vnPipesData2[0], &rFds);
    FD_SET(s_vnPipesCtrl[0], &rFds);
    if(s_vnPipesInfo[0]>0){FD_SET(s_vnPipesInfo[0], &rFds);}
    FD_ZERO(&eFds);
    FD_SET(s_vnPipesData2[0], &eFds);
    FD_SET(s_vnPipesCtrl[0], &eFds);
    if(s_vnPipesInfo[0]>0){FD_SET(s_vnPipesInfo[0], &eFds);}
    nSelectReturn = ::select(s_nMaxDescrPlus1, &rFds, NULL,&eFds, NULL);

    if(nSelectReturn<0){
        mexPrintf("Binary finished current stage with error\n");mexEvalString("drawnow");
        return 0;
    }
    else if( FD_ISSET(s_vnPipesData2[0], &rFds) ){ return s_vnPipesData2[0]; }
    else if( FD_ISSET(s_vnPipesInfo[0], &rFds) ){ return s_vnPipesInfo[0]; }
    else{
        mexPrintf("Binary finished current stage\n");mexEvalString("drawnow");
        return 0;
    }

    return 0;
}


static int DataToMatlab(const structCommonNew& a_data, mxArray* a_dataMaytlab, int nIndexF,
                        int type_field,int time_field,int event_field,int isAvailable_field,int data_field)
{
    int32_t *pnInt32;
    void *pDataMatlab;
    mxArray *pType,*pTime, *pEvent,*pIsAvailable,*pMatData;
    mxClassID clsId;
    size_t unItemSize;
    //int nIndexF = mxCalcSingleSubscript(a_dataMaytlab,2,a_subs);

    pTime = mxCreateNumericMatrix(1,1,mxINT32_CLASS,mxREAL);
    HANDLE_MEM_DEF(pTime);
    pnInt32 = (int32_t*)mxGetData( pTime );
    *pnInt32 = a_data.time();
    mxSetFieldByNumber(a_dataMaytlab,nIndexF,time_field,pTime);

    pEvent = mxCreateNumericMatrix(1,1,mxINT32_CLASS,mxREAL);
    HANDLE_MEM_DEF(pEvent);
    pnInt32 = (int32_t*)mxGetData( pEvent );
    *pnInt32 = a_data.gen_event();
    mxSetFieldByNumber(a_dataMaytlab,nIndexF,event_field,pEvent);

    pIsAvailable = mxCreateNumericMatrix(1,1,mxINT32_CLASS,mxREAL);
    HANDLE_MEM_DEF(pIsAvailable);
    pnInt32 = (int32_t*)mxGetData( pIsAvailable );
    //*pnInt32 = a_data.buffer?1:0;
    *pnInt32 = 1;
    mxSetFieldByNumber(a_dataMaytlab,nIndexF,isAvailable_field,pIsAvailable);

    switch(a_data.type()){
    case dataTypeInt:
        //mexPrintf("!!!!!!!!!!!!!!!!!!!!!! Int_t\n");
        pType = mxCreateString("Int_t");
        clsId = mxINT32_CLASS;
        unItemSize = 4;
        break;

    case dataTypeFloat:
        pType = mxCreateString("Float_t");
        clsId = mxSINGLE_CLASS;
        unItemSize = 4;
        break;

    default:
        //mexPrintf("!!!!!!!!!!!!!!!!!!!!!! default\n");
        pType = mxCreateString("Unknown");
        clsId = mxUNKNOWN_CLASS;
        unItemSize = 1;
        break;
    }

    HANDLE_MEM_DEF(pType);
    mxSetFieldByNumber(a_dataMaytlab,nIndexF,type_field,pType);

    pMatData = mxCreateNumericMatrix(1,a_data.count(),clsId,mxREAL);
    HANDLE_MEM_DEF(pMatData);

    pDataMatlab = mxGetData( pMatData );
    memcpy(pDataMatlab,a_data.dataVoidPtr(),unItemSize*a_data.count());
    mxSetFieldByNumber(a_dataMaytlab,nIndexF,data_field,pMatData);


    return 0;
}


static double GetNumericData(const mxArray* a_numeric)
{
    mxClassID classId=mxGetClassID(a_numeric);

    switch(classId)
    {
    case mxDOUBLE_CLASS:
        return *mxGetPr(a_numeric);
    case mxSINGLE_CLASS:
        return (double)(*((float*)mxGetData( a_numeric )));
        break;
    case mxINT8_CLASS:
        return (double)(*((int8_t*)mxGetData( a_numeric )));
        break;
    case mxUINT8_CLASS:
        return (double)(*((uint8_t*)mxGetData( a_numeric )));
        break;
    case mxINT16_CLASS:
        return (double)(*((int16_t*)mxGetData( a_numeric )));
        break;
    case mxUINT16_CLASS:
        return (double)(*((uint16_t*)mxGetData( a_numeric )));
        break;

    case mxINT32_CLASS:
        return (double)(*((int32_t*)mxGetData( a_numeric )));
        break;
    case mxUINT32_CLASS:
        return (double)(*((uint32_t*)mxGetData( a_numeric )));
        break;
    case mxINT64_CLASS:
        return (double)(*((int64_t*)mxGetData( a_numeric )));
        break;
    case mxUINT64_CLASS:
        return (double)(*((uint64_t*)mxGetData( a_numeric )));
        break;
    default:
        break;
    }

    return 0;
}


static void PrintHelp(void)
{
    mexPrintf(
                "--help\n"
                "--get-file-entries fileName\n"
                "--get-data-for-entry fileName entryName\n"
                "--get-data-for-time-interval entryNamesVector time1 time2\n"
                "--set-log-level verbosity\n"
                "--get-log-level \n");
}
