
#include "mailsender.h"
#include <stdarg.h>
#include <stdio.h>
#include <memory.h>
#include <errno.h>
#include <string.h>

#ifdef _WIN32/*In the case of windows*/
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#define CHECK_FOR_SOCK_INVALID(_a_socket_)	((_a_socket_) == INVALID_SOCKET)
#else/* #ifdef WIN32  In the case of Linux, solaris, .. */

#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <memory.h>
#define CHECK_FOR_SOCK_INVALID(_a_socket_)	((_a_socket_) < 0)
#define closesocket	close

#endif/* #ifdef WIN32 */


#if defined(_WIN32) & defined(_MSC_VER)
#pragma comment(lib, "Ws2_32.lib")
#if _MSC_VER>1400
#pragma warning (disable:4996)
#endif
#endif


#ifdef __cplusplus
extern "C" {
#endif



int SendMail1(const char* a_hostname,const char* a_from,
			  int a_nReceivers, const char* a_tos[], int a_nCCs, const char* a_ccs[],
			  const char* a_subject, const char* a_body )
{	
	const char* host2 = NULL;
	struct sockaddr_in addr;
	unsigned long ha;
	int rtn = -1;
	int nSocket;
	int i, nStrLen;

	if( !a_hostname || *a_hostname == '\0'){  // one of following can be done
		//hostname = "hotmail.com" ;
		//hostname = "smtp.gmail.com" ;
		//if (::gethostname(l_host, MAX_HOSTNAME_LENGTH) < 0) { return E_UNKNOWN_HOST; }
		//host2 = l_host;
                //return -1;

                host2=a_hostname = "smtp.desy.de" ;
	}
	else { host2 = a_hostname; }
	
	nSocket = (int)socket(AF_INET, SOCK_STREAM, 0);
	if (CHECK_FOR_SOCK_INVALID(nSocket)) {return E_NO_SOCKET;}

	memset((char *)&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons((u_short)MAIL_SEND_PORT);
	addr.sin_addr.s_addr = 0;

	if ((ha = inet_addr(host2)) == INADDR_NONE){
		struct hostent* hostent_ptr = gethostbyname(a_hostname);
		if (!hostent_ptr){closesocket( nSocket );return E_UNKNOWN_HOST;}
		host2 = inet_ntoa(*(struct in_addr *)hostent_ptr->h_addr_list[0]);
		if ((ha = inet_addr(host2)) == INADDR_NONE){closesocket( nSocket );return E_UNKNOWN_HOST;}
	}

	memcpy((char *)&addr.sin_addr, (char *)&ha, sizeof(ha));

	rtn = connect(nSocket, (struct sockaddr*)&addr, sizeof(addr));

	if (rtn != 0) {
        if (!SOCKET_INPROGRESS2(rtn)) { closesocket( nSocket );return E_NO_CONNECT; } // SOCKET_INPROGRESS2 will never happen
	}

	
	///////////////////////////////////////////////////////////////////////////////////////////
	//send( nSocket, "EHLO null\n", 10, 0 ); // Should I use HELO or EHLO?
	send( nSocket, "EHLO Testname\n", 14, 0 ); // Should I use HELO or EHLO?
	
	send( nSocket, "STARTTLS\n", 9, 0 );// <----- starting TLS?
	
	//sendmail_write( m_nSocket, "MAIL FROM: %s\n", from );    // from
	send( nSocket, "MAIL FROM: ", 11, 0 );
	nStrLen = (int)strlen( a_from );
	send( nSocket, a_from, nStrLen, 0 );
	send( nSocket, "\n", 1, 0 );
	
	//sendmail_write( m_nSocket, "RCPT TO: %s\n",   to );      // to
	for( i = 0; i < a_nReceivers; ++i )
	{
		send( nSocket, "RCPT TO: ", 9, 0 );
		nStrLen = (int)strlen( a_tos[i] );
		send( nSocket, a_tos[i], nStrLen, 0 );
		send( nSocket, "\n", 1, 0 );
	}

	
	// ccs
	for( i = 0; i < a_nCCs; ++i )
	{
		send( nSocket, "RCPT TO: ", 9, 0 );
		nStrLen = (int)strlen( a_ccs[i] );
		send( nSocket, a_ccs[i], nStrLen, 0 );
		send( nSocket, "\n", 1, 0 );
	}
	
	
	send( nSocket, "DATA\n", 5, 0 ); // begin data
	
	//// next comes mail headers
	
	send( nSocket, "From: ", 6, 0 );
	//from = "A D A V";
	nStrLen = (int)strlen( a_from );
	send( nSocket, a_from, nStrLen, 0 );
	send( nSocket, "\n", 1, 0 );


	/// to list
	send( nSocket, "To: ", 4, 0 );
	nStrLen = (int)strlen( a_tos[0] );
	send( nSocket, a_tos[0], nStrLen, 0 );
	if( a_nReceivers > 1 )
	{
		for( i = 1; i < a_nReceivers; ++i )
		{
			send( nSocket, ",", 1, 0 );
			nStrLen = (int)strlen( a_tos[i] );
			send( nSocket, a_tos[i], nStrLen, 0 );
		}
	}
	send( nSocket, "\n", 1, 0 );
	

	// cc list if any
	if( a_nCCs )
	{
		send( nSocket, "cc: ", 4, 0 );
		nStrLen = (int)strlen( a_ccs[0] );
		send( nSocket, a_ccs[0], nStrLen, 0 );

		if( a_nCCs > 1 )
		{
			for( i = 1; i < a_nCCs; ++i )
			{
				send( nSocket, ",", 1, 0 );
				nStrLen = (int)strlen( a_ccs[i] );
				send( nSocket, a_ccs[i], nStrLen, 0 );
			}
		}

		send( nSocket, "\n", 1, 0 );
	}

	

	//Subject
	send( nSocket, "Subject: ", 9, 0 );
	nStrLen = (int)strlen( a_subject );
	send( nSocket, a_subject, nStrLen, 0 );
	send( nSocket, "\n\n", 2, 0 );
	
	
	//   data (body)
	nStrLen = (int)strlen( a_body );
	send( nSocket, a_body, nStrLen, 0 );
	send( nSocket, "\n.\n", 3, 0 );
	
	
	// terminate
	send( nSocket, "QUIT\n", 5, 0 );
	///////////////////////////////////////////////////////////////////////////////////////////

	// Closing the socket
	closesocket( nSocket );	
	return 0;
}


int InitSocketLibrary(void)
{
#ifdef _WIN32

	WORD wVersionRequested;
	WSADATA wsaData;
	int nReturn;

	wVersionRequested = MAKEWORD(2, 2);
	nReturn = WSAStartup(wVersionRequested, &wsaData);
	if (nReturn != 0){return nReturn;}

	/* Confirm that the WinSock DLL supports 2.2.		*/
	/* Note that if the DLL supports versions greater	*/
	/* than 2.2 in addition to 2.2, it will still return*/
	/* 2.2 in wVersion since that is the version we		*/
	/* requested.										*/

	if (LOBYTE(wsaData.wVersion) != 2 ||
		HIBYTE(wsaData.wVersion) != 2){
		WSACleanup();
		return -1;
	}

#endif  // #ifdef _WIN32

	return 0;
}


void CleanSocketLibrary(void)
{
#ifdef _WIN32
	WSACleanup();
#endif
}


#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#include <vector>
#include <string>

int SendEmailCpp(const char* a_from,
                 const char* a_tos, const char* a_subject, const char* a_body,int a_nCCs, const char* a_ccs[], const char* a_hostname)
{
    std::vector<std::string> vectReceivers;
    char* pcTos = strdup(a_tos);
    char *pcStart(pcTos), *pcTerm;
    const char** ppTos;
    int i, nReceivers;
    int nReturn (-1);

    if(!pcTos){
        goto returnPoint;
    }


    pcTerm = strchr(pcStart,';');
    if(pcTerm){*pcTerm=0;}
    vectReceivers.push_back(pcStart);

    while(pcTerm){
        pcStart = pcTerm+1;
        pcTerm = strchr(pcStart,';');
        if(pcTerm){*pcTerm=0;}
        vectReceivers.push_back(pcStart);
    }

    nReceivers = (int)vectReceivers.size();
    ppTos=(const char**)alloca(sizeof(const char*)*nReceivers);

    for(i=0;i<nReceivers;++i){
        ppTos[i]=vectReceivers[i].c_str();
    }

    nReturn=SendMail1(a_hostname,a_from,nReceivers,ppTos,a_nCCs,a_ccs,a_subject,a_body);
returnPoint:
    free(pcTos);
    return nReturn;
}


#endif  // #ifdef __cplusplus
