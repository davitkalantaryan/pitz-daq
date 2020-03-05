

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <mcast_common_apis.h>
#include "../tmp/MCclass.h"
#include <iostream>
#include <common/common_argument_parser.hpp>
#include "mcast_fnc_test_sender_receiver_common.h"

#define TIMEOUT_MS              100


int main(int a_argc, char* a_argv[])
{
    ssize_t  result;
    MCsender aSender;
    DATA_structBase* pData = nullptr;
    time_t currentTime;
    int nErrors=0;
    int nLastPrint = 0, nSymbIndex;
    uint64_t unSleepMs = TIMEOUT_MS;
    unsigned int sleepSec;
    useconds_t sleepTimeUs; // 100 ms
    size_t unDataSize = sizeof(struct DATA_struct);
    ::common::argument_parser aParser;

    //aParser
    //        << "-h,--help:display this message"
    //        << "-ds,--data-size:size of data to send during each iteration"
    //        << "-sp,--send-period-ms:time between 2 sends in ms";
    aParser <<  "-h,--help:display this message";
    aParser.
            AddOption("-ds,--data-size:size of data to send during each iteration").
            AddOption("-sp,--send-period-ms:time between 2 sends in ms");

    aParser.ParseCommandLine(a_argc, a_argv);

    if(aParser["-h"]){
        std::cout<< aParser.HelpString()<<std::endl;
        return 0;
    }

    if(aParser["-ds"]){
        unDataSize = static_cast<size_t>(atoi(aParser["-ds"]));
    }

    if(aParser["-sp"]){
        unSleepMs = static_cast<uint64_t>(strtoll(aParser["-sp"],nullptr,10));
    }

    pData = CreateDataStructWithSize(unDataSize,&unDataSize);
    ::std::cout << "DataSize=" << unDataSize<<",SendPeriod="<<unSleepMs<< ::std::endl;

    if(!pData){
        perror("");
        ::std::cerr<< "Unable to create buffer for network data\n";
        return 1;
    }

    sleepSec = static_cast<unsigned int>(unSleepMs / 1000);
    sleepTimeUs = static_cast<useconds_t>((unSleepMs % 1000) * 1000);

    pData->endian = 1;
    pData->samples = static_cast<int>((unDataSize-sizeof(DATA_structBase))/sizeof (float));
    pData->branch_num = TEST_SPECTRUM_NUMBER;
    pData->gen_event = 0;


    while(1){
        pData->seconds = static_cast<int>(time(&currentTime));
        ++pData->gen_event;
        result = sendto(aSender.sock,pData, sizeof(DATA_struct), 0, reinterpret_cast<struct sockaddr *>(&(aSender.saddr)), aSender.socklen);

        if(result != sizeof(DATA_struct)){
            ++nErrors;
            ::std::cerr << "\nsendto returned: " << result << ::std::endl;
            ::std::cout << "iteration:" << pData->gen_event <<", errors:"<<nErrors << ::std::endl;
            nLastPrint = 0;
        }
        else{
            for(nSymbIndex=0;nSymbIndex<nLastPrint;++nSymbIndex){printf("\b");}
            nLastPrint = printf("iterations=%d, errors=%d",pData->gen_event,nErrors);
            fflush(stdout);
        }

        if(sleepSec>0){sleep(sleepSec);}
        if(sleepTimeUs>0){usleep(sleepTimeUs);}
    }


    //return 0;
}

