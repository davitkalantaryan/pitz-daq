
/*
 * Created by L. Hakobyan
 * Modified by D. Kalantaryan on 11.01.2016
 *
 */


#include "mclistener.hpp"

#ifndef SOCKET_ERROR_UNI
#ifdef WIN32
#define SOCKET_ERROR_UNI SOCKET_ERROR
#else
#define SOCKET_ERROR_UNI -1
#endif
#endif


//========================================================================================

MClistener::MClistener()
    :
      m_sock(-1)
{
    m_socklen = sizeof(struct sockaddr_in);
}


MClistener::~MClistener()
{
    CloseSock();
}


void MClistener::CloseSock()
{
    if(m_sock >= 0)
    {
        ::shutdown(m_sock, 2);
#ifdef _WIN32
		::closesocket(m_sock); // discard a socket
#else
        ::close(m_sock); // discard a socket
#endif
        m_sock = -1;
    }

}


int MClistener::ConnectToTheMGroup(const char* a_schedulerHost)
{
    ip_mreq     aMreq;
    int nError;
    int reuse = 1;
    int nPort;
    char vcMcastGroup[64];

    //m_hostent_ = gethostbyname(m_HOST_NAME);
	nError=mcast_common_apis::GroupNameAndPortAlg1(a_schedulerHost,vcMcastGroup,63,&nPort);
	if(nError){return nError;}

    // Creating UDP socket
    m_sock = (int)socket(AF_INET, SOCK_DGRAM, 0);
    if(m_sock < 0) { return m_sock; }

    nError = setsockopt(m_sock,SOL_SOCKET,SO_REUSEADDR,(char *)&reuse,sizeof(reuse));
    if ( nError< 0){
        CloseSock();
        return nError;
    }

    /* set up destination address */
    m_saddr.sin_family=AF_INET;
    m_saddr.sin_addr.s_addr=htonl(INADDR_ANY); /* N.B.: differs from sender */
    m_saddr.sin_port=htons(nPort);

    //nError = setsockopt(m_sock, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loopch, sizeof(loopch));

    /* allow multiple sockets to use the same PORT number */


    /* bind to receive address */
    nError = bind(m_sock,(struct sockaddr *) &m_saddr,sizeof(m_saddr));
    if ( nError < 0){
        CloseSock();
        return nError;
    }

    aMreq.imr_multiaddr.s_addr=inet_addr(vcMcastGroup);
    aMreq.imr_interface.s_addr=htonl(INADDR_ANY);

    if (setsockopt(m_sock,IPPROTO_IP,IP_ADD_MEMBERSHIP,(char *)&aMreq,sizeof(aMreq)) < 0){
        ERR_LOG(" ");
    }

#if 0
#ifdef	_WIN32
	unsigned long on = 1;
	ioctlsocket(m_sock, FIONBIO, &on );
#else  /* #ifdef	WIN32 */
	int status;
	if( ( status = fcntl(m_sock, F_GETFL, 0 ) ) != -1 )
	{
		status |= O_NONBLOCK;
		fcntl(m_sock, F_SETFL, status );
	}
#endif  /* #ifdef	WIN32 */
#endif  // #if 0

    return 0;

}


int MClistener::SetSocketTimeout(int a_nTimeoutMs)
{

	char* pInput;
	int nInputLen;

#ifdef _WIN32
	DWORD dwTimeout = a_nTimeoutMs;
	pInput = (char*)&dwTimeout;
	nInputLen = sizeof(DWORD);
#else
	struct timeval tv;
	tv.tv_sec = a_nTimeoutMs/1000;
	tv.tv_usec = (a_nTimeoutMs % 1000)*1000;
        pInput = (char*)&tv;
	nInputLen = sizeof(struct timeval);
#endif

	if (setsockopt(m_sock, SOL_SOCKET, SO_RCVTIMEO, pInput, nInputLen) < 0) {
		//perror("Error");
		return -1;
	}

	return 0;
}


int MClistener::recvC(void* a_buff, int a_buffLen)
{
        return recvfrom(m_sock,(char*)a_buff,a_buffLen,0,(sockaddr *)&m_saddr,&m_socklen);
}


bool MClistener::Init()
{
#ifdef _WIN32
	WORD wVersionRequested;
	WSADATA wsaData;

	wVersionRequested = MAKEWORD(2, 2);

	if (WSAStartup(wVersionRequested, &wsaData) != 0){return false;}

	/* Confirm that the WinSock DLL supports 2.2.		*/
	/* Note that if the DLL supports versions greater	*/
	/* than 2.2 in addition to 2.2, it will still return*/
	/* 2.2 in wVersion since that is the version we		*/
	/* requested.										*/

	if (LOBYTE(wsaData.wVersion) != 2 ||
		HIBYTE(wsaData.wVersion) != 2){
		WSACleanup();
		return false;
	}

	return true;
#else
	return true;
#endif
}


void MClistener::Cleanup()
{
#ifdef _WIN32
	WSACleanup();
#endif
}

//========================================================================================

