//
// file:		main_interlock_notifier_server.cpp
// created on:	2018 Jun 21
//

#ifndef LOG_LEVEL_DECLARED
#define LOG_LEVEL_DECLARED
#endif

#include "desy/interlocknotifier/server.hpp"
#include "common/common_argument_parser.hpp"
#include <iostream>

#define OPTION_NAME_LOG_LEVEL	"--log-level"


int main(int a_argc, char* a_argv[])
{
	char** argv = a_argv + 1;
	int argc = a_argc - 1;
	const char* cpcLogLevel;
	common::argument_parser argParser;
	desy::interlockNotifier::Server aServer;
	int nReturn(-1);

	argParser.
		AddOption(OPTION_NAME_LOG_LEVEL ",-ll:log level") <<
		"--help,-h:display this help";

	argParser.ParseCommandLine(argc, argv);

	cpcLogLevel = argParser[OPTION_NAME_LOG_LEVEL];
	if (cpcLogLevel) { g_nLogLevel = atoi(cpcLogLevel); }

	DEBUG_APP(1, "Interlock notifier server. Version 1, sizeof(i4_mod_ctrl_t)=%d",(int)sizeof(i4_mod_ctrl_t));

	if (argParser["-h"]) {
		std::cout << argParser.HelpString() << std::endl;
		nReturn = 0;
		goto returnPoint;
	}

	common::socketN::Initialize();

	aServer.RunServerThreads();

	while(1){
		Sleep(5000);
	}

	nReturn = 0;
returnPoint:
	common::socketN::Cleanup();

	return nReturn;
}
