
#include <common/common_argument_parser.hpp>
#include "bin_for_mexdaq_browser_common.h"
#include <stdio.h>
#include <signal.h>
#include <iostream>
#include "pitz/daq/data/getter/tosocketpipe.hpp"
#ifdef _WIN32
#else
#include <unistd.h>
#endif
#include <string.h>


#if 0
#define TO_DO_GET_FILE_ENTRIES              "get-file-entries"
#define TO_DO_GET_DATA_FOR_ENTRY            "get-data-for-entry"
#define TO_DO_GET_DATA_FOR_TIME_INTERVAL    "get-data-for-time-interval"

#define OPTION_NAME_HELP                    "--help"
#define SHORTCUT_OPTION_NAME_HELP           "-h"
#define OPTION_NAME_ROOT_FILE               "--root-file-name"
#define SHORTCUT_OPTION_NAME_ROOT_FILE      "-rfn"
#define OPTION_NAME_START_TIME              "--start-time"
#define SHORTCUT_OPTION_NAME_START_TIME     "-st"
#define OPTION_NAME_END_TIME                "--end-time"
#define SHORTCUT_OPTION_NAME_END_TIME       "-et"
#define OPTION_NAME_DAQ_NAME                "--daq-name"
#define SHORTCUT_OPTION_NAME_DAQ_NAME       "-dn"
#endif

#define _INDEX_FILE_DIR_        "//afs/ifh.de/group/pitz/doocs/data/DAQdata/INDEX"
#ifndef DEBUG_ROOT_APP
#define DEBUG_ROOT_APP(_logLevel,...)   do{printf("ln:%d -> ",__LINE__);printf(__VA_ARGS__);printf("\n");}while(0)
#endif

static void PrintAdditionalHelp();

using namespace pitz;
using namespace pitz::daq;

//void SignalHandler(int){}  // todo: should be understood

int main(int a_argc, char* a_argv[])
{
    int argc(a_argc-1);
    char** argv(a_argv+1);
    common::argument_parser argParser;
    int nReturn(-10);
    pitz::daq::data::getter::ToPipe* pGetter=NULL;
    const char* cpcPrivatePipeInitial;

    SleepMsInt(10000);

#if 0
    struct sigaction  sigusr1Action;
    sigusr1Action.sa_flags = 0;
    sigemptyset(&sigusr1Action.sa_mask);
    sigusr1Action.sa_restorer = NULL;
    sigusr1Action.sa_handler = &SignalHandler;
    sigaction(SIGSEGV,&sigusr1Action,NULL);
#endif

    dprintf(STDOUT_FILENO,"version 4.1.0. argc=%d" "\n",a_argc);

    for(int i(1);i<a_argc;++i){dprintf(STDOUT_FILENO,"argv[%d]: %s\n",i,a_argv[i]);}

    argParser.
        AddOption(OPTION_NAME_ROOT_FILE "," SHORTCUT_OPTION_NAME_ROOT_FILE ":Config file for server").
        AddOption(OPTION_NAME_START_TIME "," SHORTCUT_OPTION_NAME_START_TIME).
        AddOption(OPTION_NAME_END_TIME "," SHORTCUT_OPTION_NAME_END_TIME).
        AddOption(OPTION_NAME_USE_PRIV_PIPE "," SHORTCUT_OPTION_NAME_USE_PRIV_PIPE).
        AddOption(OPTION_NAME_ALL_DAQ_NAMES "," SHORTCUT_OPTION_NAME_ALL_DAQ_NAMES).
        AddOption(OPTION_NAME_LOG_LEVEL "," SHORTCUT_OPTION_NAME_LOG_LEVEL)<<
        OPTION_NAME_WAIT_FOR_DEBUG "," SHORTCUT_OPTION_NAME_WAIT_FOR_DEBUG ":wait for debugger to connect"<<
        OPTION_NAME_HELP "," SHORTCUT_OPTION_NAME_HELP ":display this help" <<
        OPTION_NAME_DUMMY "," OPTION_NAME_DUMMY ": ";

    argParser.ParseCommandLine(argc, argv);

    if(argParser[OPTION_NAME_WAIT_FOR_DEBUG] ){
        sigset_t sigSet;
        int nSigNum;

        sigemptyset(&sigSet);
        sigaddset(&sigSet,SIGINT);
        // let us use depricated API here (instead of sigaction)
        signal(SIGINT, SIG_IGN);
        dprintf(STDOUT_FILENO,"waiting for signal %d(SIGINT) [ kill -%d %d ] \n",SIGINT,SIGINT,(int)getpid());
        sigwait(&sigSet,&nSigNum);
        signal(SIGINT, SIG_DFL);
    }

    if(argParser[OPTION_NAME_HELP] || (argc<1)){
        if(!argParser[OPTION_NAME_HELP]){std::cerr<<"No option is provided!\n";}
        std::cout << argParser.HelpString() << std::endl;
        PrintAdditionalHelp();
        nReturn = 0;
        goto returnPoint;
    }

    cpcPrivatePipeInitial = argParser[OPTION_NAME_LOG_LEVEL];
    if(cpcPrivatePipeInitial){ log::g_nLogLevel = (ptrdiff_t)atoi(cpcPrivatePipeInitial);}

    cpcPrivatePipeInitial = argParser[OPTION_NAME_USE_PRIV_PIPE];
    if(!cpcPrivatePipeInitial){
        dprintf(STDOUT_FILENO,"WARNING: No pipe provided\n");
        goto returnPoint;
    }


    if(strcmp(argv[0],TO_DO_MAKE_INTERACTIVE_LOOP)==0){
        pGetter = new data::getter::ToSocketPipe;
        pGetter->SetPipes(cpcPrivatePipeInitial);
        pGetter->StartLoop();
    }
    else{
        pGetter = new data::getter::ToPipeSingle(::write);
        pGetter->SetPipes(cpcPrivatePipeInitial);

        if(strcmp(argv[0],TO_DO_GET_FILE_ENTRIES)==0){
            const char* cpcRootFileName = argParser[OPTION_NAME_ROOT_FILE];
            if(!cpcRootFileName){
                std::cerr<<"RootFileName should be provided!\n";
                std::cout << argParser.HelpString() << std::endl;
                PrintAdditionalHelp();
                goto returnPoint;
            }
            pGetter->GetEntriesInfo(cpcRootFileName);
        }
        else if(strcmp(argv[0],TO_DO_GET_DATA_FOR_ENTRIES)==0){
            ::std::vector< ::std::string > branchNames;
            const char* cpcRootFileName = argParser[OPTION_NAME_ROOT_FILE];
            const char* cpcAllDaqNames = argParser[OPTION_NAME_ALL_DAQ_NAMES];
            if((!cpcRootFileName)||(!cpcAllDaqNames)){
                std::cerr<<"RootFileName and AllDaqNames should be provided!\n";
                std::cout << argParser.HelpString() << std::endl;
                PrintAdditionalHelp();
                goto returnPoint;
            }
            data::getter::GetBranchNames(cpcAllDaqNames,&branchNames);
            pGetter->GetMultipleEntries(cpcRootFileName,branchNames);
        }
        else if(strcmp(argv[0],TO_DO_GET_DATA_FOR_TIME_INTERVAL)==0){
            ::std::vector< ::std::string > branchNames;
            const char* cpcAllDaqNames = argParser[OPTION_NAME_ALL_DAQ_NAMES];
            const char *cpcStartTime = argParser[OPTION_NAME_START_TIME];
            const char *cpcEndTime = argParser[OPTION_NAME_END_TIME];
            int i,nNumberOfBranches,nStartTime, nEndTime;

            cpcStartTime = argParser[OPTION_NAME_START_TIME];
            if((!cpcAllDaqNames)||(!cpcStartTime)||(!cpcEndTime)){
                std::cerr<<"AllDaqNames, StartTime and EndTime should be provided!\n";
                std::cout << argParser.HelpString() << std::endl;
                PrintAdditionalHelp();
                goto returnPoint;
            }

            nStartTime = atoi(cpcStartTime);
            nEndTime = atoi(cpcEndTime);
            data::getter::GetBranchNames(cpcAllDaqNames,&branchNames);
            for(i=0,nNumberOfBranches=(int)branchNames.size();i<nNumberOfBranches;++i){DEBUG_ROOT_APP(1,"%.2d branchName: %s",i,branchNames[i].c_str());}
            pGetter->GetMultipleEntriesTI(branchNames,nStartTime,nEndTime);
        }
        else{
            std::cerr<<"wrong option! option is: " << argv[0]<<"\n";
            std::cout << argParser.HelpString() << std::endl;
            PrintAdditionalHelp();
            goto returnPoint;
        }
    }


    nReturn = 0;
returnPoint:
    if(nReturn==0){
        SleepMsInt(2000);
    }
    //pGetter->ClosePipes();
    delete pGetter;
    return nReturn;
}



static void PrintAdditionalHelp()
{
    std::cout
            <<"Use following options \n"
            <<"   1. " TO_DO_GET_FILE_ENTRIES "\n"
            <<"   2. " TO_DO_GET_DATA_FOR_ENTRIES  "\n"
            <<"   3. " TO_DO_GET_DATA_FOR_TIME_INTERVAL  "\n\nExamples\n"
            <<"   1. " TO_DO_GET_FILE_ENTRIES " " OPTION_NAME_ROOT_FILE
              " /acs/pitz/daq/2018/07/pitzdiag.adc/PITZ_DATA.pitzdiag.adc.2018-07-09-1307.root\n"
            <<"   2. " TO_DO_GET_DATA_FOR_ENTRIES " " OPTION_NAME_ROOT_FILE
              " /acs/pitz/daq/2018/07/pitzdiag.adc/PITZ_DATA.pitzdiag.adc.2018-07-09-1307.root "
              OPTION_NAME_ALL_DAQ_NAMES  " \"DAQ_NAME1;DAQ_NAME2\"\n"
            <<"   3. " TO_DO_GET_DATA_FOR_TIME_INTERVAL " " OPTION_NAME_ALL_DAQ_NAMES  " \"DAQ_NAME1,DAQ_NAME2\" "
              OPTION_NAME_START_TIME " 1234567 " OPTION_NAME_END_TIME " 1234569\n";
}
