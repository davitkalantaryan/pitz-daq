

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

#define TIMEOUT_MS              100
#define TEST_SPECTRUM_NUMBER    47

struct DATA_struct
{
    int              endian;
    int              branch_num;
    int              seconds;
    int              gen_event;
    int              samples;
    float            f[2048];
};


int main()
{
    ssize_t  result;
    MCsender aSender;
    DATA_struct aData;
    time_t currentTime;
    int nEventNumber = 0;
    int nEerrors=0;

    aData.endian = 1;
    aData.samples = 2048;
    aData.branch_num = TEST_SPECTRUM_NUMBER;

    while(1){
        aData.seconds = static_cast<int>(time(&currentTime));
        aData.gen_event = ++nEventNumber;
        result = sendto(aSender.sock, &aData, sizeof(DATA_struct), 0, reinterpret_cast<struct sockaddr *>(&(aSender.saddr)), aSender.socklen);

        //::std::cout << "iteration:" << ++nNumberOfIteration <<", errors:"<<nEerrors << ::std::endl;

        if(result != sizeof(DATA_struct)){
            ++nEerrors;
            ::std::cerr << "sendto returned: " << result << ::std::endl;
            ::std::cout << "iteration:" << nEventNumber <<", errors:"<<nEerrors << ::std::endl;
        }

        usleep(TIMEOUT_MS*1000);
    }


    //return 0;
}

