

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

#define TIMEOUT_MS      5000
#define DEFAULT_SCHEDULER_HOST  "picus4"
#define TEST_SPECTRUM_NUMBER    0

struct DATA_struct
{
    int              endian;
    int              branch_num;
    int              seconds;
    int              gen_event;
    int              samples;
    float            f[2048];
};

static void PrintHelp(void);

static volatile int s_nWork = 0;
static volatile int s_nFixedPrint = 0;

int main(int argc, char* argv[])
{
    socklen_t m_socklen = sizeof(struct sockaddr_in);
    struct timeval tv;
    ip_mreq     aMreq;
    sockaddr_in m_saddr;
    int nReturn = 1;
    int nErrno;
    int nNumberOfErrors = 0;
    int nReceived;
    int nPort;
    int m_sock = -1;
    int nError;
    int reuse = 1;
    int nNumberOfIterations=1, i, j;
    int nSpectrumToTest = TEST_SPECTRUM_NUMBER;
    int nPreviousEventNumber=0, nPrinted;
    time_t currentTime;
    const char* cpcSchedulerHost = DEFAULT_SCHEDULER_HOST;
    char vcGroupNameBuf[128];
    DATA_struct bufferForReceive;

    if((argc>1)&&((strcmp(argv[1],"--help")==0)||(strcmp(argv[1],"-h")==0))){PrintHelp();nReturn=0;goto returnPoint;}
    if(argc>1){cpcSchedulerHost=argv[1];}

    printf("version: 3, schedulerHost=\"%s\".\n",cpcSchedulerHost);

    mcast_common_apis::GroupNameAndPortAlg1(cpcSchedulerHost,vcGroupNameBuf,127,&nPort);
    printf("group:port -> %s:%d\n",vcGroupNameBuf,nPort);


    m_sock = socket(AF_INET, SOCK_DGRAM, 0);if(m_sock<0){goto returnPoint;}
    nError = setsockopt(m_sock,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));if(nError<0){goto returnPoint;}

    /* set up destination address */
    m_saddr.sin_family=AF_INET;
    m_saddr.sin_addr.s_addr=htonl(INADDR_ANY); /* N.B.: differs from sender */
    m_saddr.sin_port=htons(static_cast<uint16_t>(nPort));

    nError = bind(m_sock,reinterpret_cast<struct sockaddr *>(&m_saddr),sizeof(m_saddr));if ( nError < 0){ goto returnPoint; }

    aMreq.imr_multiaddr.s_addr=inet_addr(vcGroupNameBuf);
    aMreq.imr_interface.s_addr=htonl(INADDR_ANY);

    if(argc>2){
        nNumberOfIterations=atoi(argv[2]);if(argc>3){nSpectrumToTest=atoi(argv[3]);}
    }
    printf("Datagram size %d Byte. Number of iterations for test: %d\n",static_cast<int>(sizeof(DATA_struct)),nNumberOfIterations);

    if (setsockopt(m_sock,IPPROTO_IP,IP_ADD_MEMBERSHIP,&aMreq,sizeof(aMreq)) < 0){
        fprintf(stderr,"Unable to join mcast group\n");
        goto returnPoint;
    }
    tv.tv_sec = TIMEOUT_MS/1000;
    tv.tv_usec = (TIMEOUT_MS % 1000)*1000;
    if (setsockopt(m_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval)) < 0){
        fprintf(stderr,"Unable to set socket timeout!\n");
        goto returnPoint;
    }

    s_nWork = 1;

    s_nFixedPrint=printf("genEnent[branch:%d]=",nSpectrumToTest);
    nPrinted=0;

    for(i=0;((i<nNumberOfIterations)||(nNumberOfIterations<0))&&s_nWork;){
        nReceived=static_cast<int>(recvfrom(m_sock,&bufferForReceive,sizeof(DATA_struct),0,reinterpret_cast<struct sockaddr *>(&m_saddr),&m_socklen));
        if(nReceived!=sizeof(DATA_struct)){
            printf("received from %s(McastGroup:%s)  -> %d\n",cpcSchedulerHost,vcGroupNameBuf,nReceived);
            goto returnPoint;
        }
        if(bufferForReceive.branch_num!=nSpectrumToTest){continue;}
        for(j=0;j<nPrinted;++j){printf("\b");}nPrinted=0; // right way is for(;nPrinted>0;--nPrinted){printf("\b");}
        //if(((bufferForReceive.gen_event-nPreviousEventNumber)!=1)&&((bufferForReceive.gen_event-nPreviousEventNumber)!=0)){
        if(((bufferForReceive.gen_event-nPreviousEventNumber)!=1)&&nPreviousEventNumber){
            for(j=0;j<s_nFixedPrint;++j){printf("\b");}
            time(&currentTime);
            fprintf(stderr,"!!! %.24s difference not 1, old=%d, new=%d, numberOfErrors=%d\n",
                    ctime(&currentTime),nPreviousEventNumber,bufferForReceive.gen_event,++nNumberOfErrors);
            printf("genEnent[branch:%d]=",nSpectrumToTest);
            nPrinted=0;
        }
        else if((bufferForReceive.gen_event==nPreviousEventNumber)&&nPrinted){continue;}

        nPrinted=printf("%d",bufferForReceive.gen_event);fflush(stdout);
        nPreviousEventNumber = bufferForReceive.gen_event;
        ++i;
    }
    printf("\nNumberOfErrorsDuringTest=%d\n",nNumberOfErrors);

    nReturn = 0;
returnPoint:
    if(nReturn){
        nErrno = errno;
        perror(" ");
        fprintf(stderr,"errno:%d\n",nErrno);
    }

    if(m_sock>=0){::shutdown(m_sock, 2);::close(m_sock);}

    return 0;
}



static void PrintHelp(void)
{
    printf("Usage: $test_udp_receiver picus4 100000 47  # here first arg is the host of muticaster, second number of packagese (-1: infinite), 3rd is channel");
}
