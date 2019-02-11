

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TIMEOUT_MS      5000
#define LMC_PORT_NUMBER  8888
#define DEFAULT_SCHEDULER_HOST  "picus4"

int main(int argc, char* argv[])
{
    socklen_t m_socklen = sizeof(struct sockaddr_in);
    struct timeval tv;
    ip_mreq     aMreq;
    sockaddr_in m_saddr;
    int nReturn = -1;
    int nReceived;
    int nPort;
    int m_sock = -1;
    int nError;
    int reuse = 1;
    unsigned long ha;
    const char* pcFirstDot;
    const char* cpcSchedulerHost = DEFAULT_SCHEDULER_HOST;
    char vcGroupNameBuf[128];
    char vcBufferForReceive[8200];

    if(argc>1){cpcSchedulerHost=argv[1];}

    printf("schedulerHost=\"%s\"\n",cpcSchedulerHost);

    if( ( ha = inet_addr(cpcSchedulerHost) ) == INADDR_NONE ){
        struct hostent* hostent_ptr = gethostbyname(cpcSchedulerHost);
        if( !hostent_ptr ){goto returnPoint;}
        cpcSchedulerHost = inet_ntoa(*(struct in_addr *)hostent_ptr->h_addr_list[0]);
        if( ( ha = inet_addr(cpcSchedulerHost) ) == INADDR_NONE ){goto returnPoint;}
    }

    pcFirstDot = strchr(cpcSchedulerHost,'.');
    if(!pcFirstDot) {goto returnPoint;}

    snprintf(vcGroupNameBuf,128,"239%s",pcFirstDot);
    // Port calculation
    nPort = LMC_PORT_NUMBER;
    printf("group:port -> %s:%d\n",vcGroupNameBuf,nPort);


    m_sock = (int)socket(AF_INET, SOCK_DGRAM, 0);if(m_sock<0){goto returnPoint;}
    nError = setsockopt(m_sock,SOL_SOCKET,SO_REUSEADDR,(char *)&reuse,sizeof(reuse));if(nError<0){goto returnPoint;}

    /* set up destination address */
    m_saddr.sin_family=AF_INET;
    m_saddr.sin_addr.s_addr=htonl(INADDR_ANY); /* N.B.: differs from sender */
    m_saddr.sin_port=htons(nPort);

    nError = bind(m_sock,(struct sockaddr *) &m_saddr,sizeof(m_saddr));if ( nError < 0){ goto returnPoint; }

    aMreq.imr_multiaddr.s_addr=inet_addr(vcGroupNameBuf);
    aMreq.imr_interface.s_addr=htonl(INADDR_ANY);

    if (setsockopt(m_sock,IPPROTO_IP,IP_ADD_MEMBERSHIP,(char *)&aMreq,sizeof(aMreq)) < 0){goto returnPoint;}
    tv.tv_sec = TIMEOUT_MS/1000;
    tv.tv_usec = (TIMEOUT_MS % 1000)*1000;
    if (setsockopt(m_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval)) < 0){goto returnPoint;}

    nReceived=recvfrom(m_sock,vcBufferForReceive,8200,0,(sockaddr *)&m_saddr,&m_socklen);
    printf("received from %s(McastGroup:%s)  -> %d\n",cpcSchedulerHost,vcGroupNameBuf,nReceived);

    nReturn = 0;
returnPoint:
    if(nReturn){
        perror(" ");
        fprintf(stderr,"errno: %d\n",errno);
    }
    else{
        //
    }

    if(m_sock>=0){::shutdown(m_sock, 2);::close(m_sock);}

    return 0;
}
