//
// file:		main_interlock_notifier_server.cpp
// created on:	2018 Jun 21
//

#include <common_daq_definations.h>
#include <pitz/daq/data/browserserver.hpp>
#include <common/common_argument_parser.hpp>
#include <iostream>
#include <signal.h>

#define OPTION_NAME_LOG_LEVEL	"--log-level"
#ifndef NULL_ACTION
#define NULL_ACTION	((struct sigaction*)0)
#endif

using namespace pitz::daq;

static volatile int s_nWork = 0;
static void SignalHandler(int a_signal);

int main(int a_argc, char* a_argv[])
{
	char** argv = a_argv + 1;
	int argc = a_argc - 1;
	const char* cpcLogLevel;
	common::argument_parser argParser;
    data::BrowserServer aServer;
	int nReturn(-1);



    struct sigaction newAction;

        newAction.sa_handler = SignalHandler;
    #if !defined(_WIN32) || defined(_WLAC_USED)
        newAction.sa_flags = 0;
        sigemptyset(&newAction.sa_mask);
        newAction.sa_restorer = NULL;


        sigaction(SIGPIPE, &newAction, NULL);
    #ifdef _USE_LOG_FILES
        sigaction(SIGTSTP, &newAction, NULL);
    #endif
    #else
        s_mainThreadHandle = GetCurrentThread();
    #endif

        //sigaction(SIGSEGV, &newAction, NULL_ACTION);
        sigaction(SIGABRT, &newAction, NULL_ACTION);
        sigaction(SIGFPE, &newAction, NULL_ACTION);
        sigaction(SIGILL, &newAction, NULL_ACTION);
        sigaction(SIGINT, &newAction, NULL_ACTION);
        sigaction(SIGTERM, &newAction, NULL_ACTION);

#if 0
        struct sigaction  sigusr1Action;
    sigusr1Action.sa_flags = 0;
    sigemptyset(&sigusr1Action.sa_mask);
    sigusr1Action.sa_restorer = NULL;
    sigusr1Action.sa_handler = &SignalHandler;
    sigaction(SIGPIPE,&sigusr1Action,NULL);
    sigaction(SIGINT,&sigusr1Action,NULL);
#endif

	argParser.
		AddOption(OPTION_NAME_LOG_LEVEL ",-ll:log level") <<
		"--help,-h:display this help";

	argParser.ParseCommandLine(argc, argv);

	cpcLogLevel = argParser[OPTION_NAME_LOG_LEVEL];
    if (cpcLogLevel) { log::g_nLogLevel = atoi(cpcLogLevel); }

    MAKE_REPORT_GLOBAL(0, "DAQ browser server. Version 1");

	if (argParser["-h"]) {
		std::cout << argParser.HelpString() << std::endl;
		nReturn = 0;
		goto returnPoint;
	}

	common::socketN::Initialize();

    if(aServer.RunServerThreads()){goto returnPoint;}

    s_nWork = 1;
    while(s_nWork){
        SleepMsInt(5000);
	}

    //getchar();

	nReturn = 0;
returnPoint:
	common::socketN::Cleanup();

	return nReturn;
}


static void SignalHandler(int a_signal)
{
    MAKE_REPORT_GLOBAL(3,"signal: %d\n",a_signal);
    switch(a_signal){
    case SIGINT:
        s_nWork = 0;
        break;
    default:
        break;
    }
}
