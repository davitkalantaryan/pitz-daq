//
// file:		main_interlock_notifier_client.cpp
// created on:	2018 Jun 22
//

#ifndef LOG_LEVEL_DECLARED
#define LOG_LEVEL_DECLARED
#endif

int g_nLogLevel = 1;

#include "desy/interlocknotifier/client.hpp"
#include "common/common_argument_parser.hpp"
#include <iostream>

#define OPTION_NAME_LOG_LEVEL	"--log-level"
#define OPTION_NAME_SERVER_INFO	"--host-name"

int main(int a_argc, char* a_argv[])
{
	char** argv = a_argv + 1;
	int argc = a_argc - 1;
	const char *cpcLogLevel,*cpcServerIpOrHostName;
	i4_mod_ctrl_t aIlockData;
	common::argument_parser argParser;
	int nReturn(-1),nError;
	desy::interlockNotifier::Client aClient;

	argParser.
		AddOption(OPTION_NAME_SERVER_INFO ",-hn:server ip v4 or hostName").
		AddOption(OPTION_NAME_LOG_LEVEL ",-ll:log level") <<
		"--help,-h:display this help";

	argParser.ParseCommandLine(argc, argv);

	cpcLogLevel = argParser[OPTION_NAME_LOG_LEVEL];
	if (cpcLogLevel) { g_nLogLevel = atoi(cpcLogLevel); }

	DEBUG_APP(1, "Interlock notifier client. Version 1");

	if (argParser["-h"]) {
		std::cout << argParser.HelpString() << std::endl;
		nReturn = 0;
		goto returnPoint;
	}

	cpcServerIpOrHostName = argParser[OPTION_NAME_SERVER_INFO];
	if (!cpcServerIpOrHostName) { 
		std::cerr << "Server info shouyld be specified!" << std::endl;
		std::cout << argParser.HelpString() << std::endl;
		goto returnPoint;
	}

	common::socketN::Initialize();

	while(1){
		if (aClient.ConnectToServer(cpcServerIpOrHostName)) {
			Sleep(2000);
			continue;
		}

		DEBUG_APP(0,"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! connected to %s", cpcServerIpOrHostName);

		while (1) {
			nError = aClient.ReceiveInterlockData(&aIlockData);
			//if (nError != 0) { DEBUG_APP(1, "nError=%d, nInterlockData=%d", nError, 644); }
			DEBUG_APP(2, "nError=%d, nInterlockData=%d", nError, 644);
			if(nError>0){ // we received data, but this is not interlock
				DEBUG_APP(2, "No! nError = %d",nError);
				continue;
			}
			else if (nError<0) { 
				break; 
			}
			// todo handle interlock
			DEBUG_APP(1, "should be handlede!");
		}

		aClient.DisconnectFromServer();
		DEBUG_APP(0, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! disconnected from to %s", cpcServerIpOrHostName);
	}

	nReturn = 0;
returnPoint:
	aClient.DisconnectFromServer();
	common::socketN::Cleanup();
	return nReturn;
}



