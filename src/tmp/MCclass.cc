/*
 * Created by L. Hakobyan
 *
 * Modified by D. Kalantaryan on 11.01.2016
 *
 */
#include <iostream>
#include	<cstdio>
#include	<cstdlib>
#include <cmath>
#include <cstring>

#include	<unistd.h>
#include	<fcntl.h>
#include	<sys/ioctl.h>
#include	<sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#include "MCclass.h"

#ifndef SOCKET_ERROR_UNI
#ifdef WIN32
#define SOCKET_ERROR_UNI SOCKET_ERROR
#else
#define SOCKET_ERROR_UNI -1
#endif
#endif


using namespace std;


//========================================================================================

struct tm * MCsender::localtime_rL(const time_t *clock, struct tm *res)
{
        struct tm *tmp;
        tmp = localtime (clock);

        res->tm_sec   = tmp->tm_sec;
        res->tm_min   = tmp->tm_min;
        res->tm_hour  = tmp->tm_hour;
        res->tm_mday  = tmp->tm_mday; 
        res->tm_mon   = tmp->tm_mon;
        res->tm_year  = tmp->tm_year;
        res->tm_wday  = tmp->tm_wday;
        res->tm_yday  = tmp->tm_yday;
        res->tm_isdst = tmp->tm_isdst;
        return res;
}

void MCsender::printtostderrL(char* dev, char* str)
{
	struct tm           t;
	long                atime;
	char                buf [80];

	fseek (stderr, 0, SEEK_END);
	atime = time (0);
	localtime_rL(&atime, &t);
	strftime (buf, (int) 80,"%H:%M.%S  %e.%m.%Y", &t);
	fprintf (stderr, "%-16s %-20s -> %s\n",dev, buf, str);
}

int MCsender::GroupNameAndPortAlg1_self(char* a_groupNameBuf,int a_bufferLen,int* a_portPtr)
{
    int nError;
    char* pcHostNameBuf = (char*)alloca(a_bufferLen+1);

    //gethostbyname_r(pcHostNameBuf,&aHostEnt,(char*)&aHostEnt,sizeof(struct hostent),&hostent_ptr,&nError);
    nError = gethostname(pcHostNameBuf, a_bufferLen);
    if(nError) return nError;
    nError = mcast_common_apis::GroupNameAndPortAlg1(pcHostNameBuf,a_groupNameBuf,a_bufferLen,a_portPtr);
    return nError;
}

MCsender::MCsender(bool arg, int port)
{
	char	str[40];
	char	str1[4][40];
	int   i = 0;
	
	char delims[] = ".";
	char *result = NULL;
	
	loopch       = (char)(arg);
		
        sprintf(m_charptr10,"%s"," ");
	
        gethostname((char*)m_HOST_NAME, 40);
        m_hostent_ = gethostbyname(m_HOST_NAME);
        sprintf(str,"%s",inet_ntoa(*(struct in_addr *)m_hostent_->h_addr_list[0]));

	result = strtok( str, delims );
	while( result != NULL ) 
	{
		sprintf(&str1[i][0],"%s",result);
   	result = strtok( NULL, delims );
		i++;
	}

        sprintf(m_HELLO_GROUP,"238.2.%s.%s",&str1[2][0],&str1[3][0]);

        fprintf(stderr,"HELLO_GROUP=%s\n",m_HELLO_GROUP);
	
	if( (port <= 0) || (port > 65535) )
	{	
		sprintf(str,"0x4%s9",&str1[3][0]);
                m_HELLO_PORT = (uint16_t)(strtoul(str,NULL,16));
	}
	else
	{
                m_HELLO_PORT = (uint16_t)(port);
	}	
	
        fprintf(stderr,"HELLO_PORT=0x%x\n",m_HELLO_PORT);
        fprintf(stderr,"HELLO_PORT=%u\n",m_HELLO_PORT);

	sock = -1;
	
	init_socket();	
}


MCsender::MCsender() : sock(-1)
{
    int nPort;
    loopch       = (char)false;

    gethostname((char*)m_HOST_NAME, 40);
    m_hostent_ = gethostbyname(m_HOST_NAME);
    GroupNameAndPortAlg1_self(m_HELLO_GROUP,40,&nPort);
    m_HELLO_PORT = nPort;
    init_socket_new();
}


MCsender::~MCsender()
{
	if(sock >= 0)
	{
		::shutdown(sock, 2);
		close(sock); // discard a socket	
	}
	sock = -1;
}


void MCsender::init_socket()
{
	int      check_number;
	int      resultopt;
	
	if(sock >= 0)
	{
		::shutdown(sock, 2);
		close(sock); // discard a socket	
	}
	sock = -1;
	
	memset((char *) &saddr, 0, sizeof(struct sockaddr_in));	

   // open a UDP socket
	check_number = 1;
	do
	{
		usleep(1000);
   	sock = socket(AF_INET, SOCK_DGRAM, 0);
		if(sock < 0) { check_number++; }					
	} while( (sock < 0) && (check_number < 4) );
	if(sock < 0) 
	{ 
                sprintf(m_charptr1,"%s","opening datagram socket");
                if( strcmp(m_charptr1,m_charptr10) )
		{
                        sprintf(m_charptr10,"%s",m_charptr1);
                        printtostderrL((char *)"ERROR",m_charptr1);
		}			
		return ;		
	}	
	
	saddr.sin_family = AF_INET;
        saddr.sin_addr.s_addr = inet_addr(m_HELLO_GROUP);
        saddr.sin_port = htons(m_HELLO_PORT);
	socklen = sizeof(struct sockaddr_in);		
	
	check_number = 1;

	do
	{
		usleep(1000);
		resultopt = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loopch, sizeof(loopch));
		if(resultopt < 0) { check_number++; }					
	} while( (resultopt < 0) && (check_number < 4) );
	if(resultopt < 0) 
	{ 
                sprintf(m_charptr1,"%s","setting IP_MULTICAST_LOOP:");
                if( strcmp(m_charptr1,m_charptr10) )
		{
                        sprintf(m_charptr10,"%s",m_charptr1);
                        printtostderrL((char *)"ERROR",m_charptr1);
		}			
		close(sock);
		sock = -1;				
		return ;		
	}
	
	ttl = (unsigned char)16;
	check_number = 1;
	
	do
	{
		usleep(1000);
		resultopt = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&ttl, sizeof(ttl));
		if(resultopt < 0) { check_number++; }					
	} while( (resultopt < 0) && (check_number < 4) );
	if(resultopt < 0) 
	{ 
                sprintf(m_charptr1,"%s","setting IP_MULTICAST_TTL:");
                if( strcmp(m_charptr1,m_charptr10) )
		{
                        sprintf(m_charptr10,"%s",m_charptr1);
                        printtostderrL((char *)"ERROR",m_charptr1);
		}			
		close(sock);
		sock = -1;				
		return ;		
	}	
		
	return ;
}


int MCsender::init_socket_new()
{
    int      resultopt;

    // Check if socken already created then destroy it
    close_sock();

    // initialize sockaddr struct 1
    memset((char *) &saddr, 0, sizeof(struct sockaddr_in));

    // open a UDP socket
    usleep(1000); ///?
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0) return sock;

    // initialize sockaddr struct 2
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr(m_HELLO_GROUP);
    saddr.sin_port = htons(m_HELLO_PORT);
    socklen = sizeof(struct sockaddr_in);

    // Set socket option to multicast socket
    usleep(1000); ///?
    resultopt = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loopch, sizeof(loopch));
    if(resultopt<0)
    {
        close_sock();
        return resultopt;
    }

    // Set ttl (Time To Leave)
    ttl = (unsigned char)16;
    usleep(1000); ///?
    resultopt = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&ttl, sizeof(ttl));
    if(resultopt<0)
    {
        close_sock();
        return resultopt;
    }

    // Set ttl (Time To Leave)
#if 0
    nIpMltcstAll = 0;
    usleep(1000); ///?
    resultopt = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_ALL, (char *)&nIpMltcstAll, sizeof(nIpMltcstAll));
    if(resultopt<0)
    {
        close_sock();
        return resultopt;
    }
#endif

    return 0;
}


int MCsender::check_sock()
{
	int          optval;
	socklen_t    optlen;
	int          rc;
	
	optlen = sizeof(optval);

	rc = getsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&optval, &optlen);

	return rc;
}

int MCsender::get_sock()
{
	return sock;
}

void MCsender::close_sock()
{
	if(sock >= 0)
	{
		::shutdown(sock, 2);
		close(sock); // discard a socket	
	}
	sock = -1;
}
