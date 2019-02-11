/*
 * File: mcast_common_apis.h
 * File implements common functionalities for
 * DAQ UDP multicast sender receiver.
 *
 * These classes initialy created by L. Hakobyan
 *
 * File created by B. Petrosyan (bagrat.petrosyan@desy.de) and D. Kalantaryan (davit.kalantaryan@desy.de)
 * Creation date: 12.01.2016
 *
 * Both classes (publisher/listener) should be modified.
 * For time beeing to have fast solution,
 * following changes are done
 *  a) New base class added to implement common functionalities
 *  b) Some functions rewritten to have them more clear
 *  c) Modifications started to compile this in windows also
 *
 * This file implements base class for both listener and sender
 *
 */
#ifndef MCAST_COMMON_APIS_H
#define MCAST_COMMON_APIS_H

#define LMC_PORT_NUMBER  8888

#ifdef WIN32
#include <Winsock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#else  // #ifdef WIN32
#include <alloca.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <memory.h>
#endif  // #ifdef WIN32

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#ifndef E_UNKNOWN_HOST
#define E_UNKNOWN_HOST		-38	/* can not find host */
#endif  // #ifndef E_UNKNOWN_HOST

#ifndef INADDR_NONE
#define INADDR_NONE     0xffffffff
#endif


typedef class mcast_common_apis
{
public:
        /*
         * CalculateGroupNameAndPortAlg1: initialise client, connect to server
         * Parameter:
         *	name:		hostname, on which server is running
         *	port:		port number
         * Return:
         * 	0:	ok
         *   != 0:	error
         */
    static int GroupNameAndPortAlg1(const char* a_cpcSchedulerHost, char* a_groupNameBuf,int a_bufferLen,int* a_portPtr)
    {
        unsigned long ha;
        const char* pcFirstDot;

        if( ( ha = inet_addr(a_cpcSchedulerHost) ) == INADDR_NONE )
        {
            struct hostent* hostent_ptr = gethostbyname(a_cpcSchedulerHost);
            if( !hostent_ptr ){return E_UNKNOWN_HOST;}
            a_cpcSchedulerHost = inet_ntoa(*(struct in_addr *)hostent_ptr->h_addr_list[0]);
            if( ( ha = inet_addr(a_cpcSchedulerHost) ) == INADDR_NONE ){return E_UNKNOWN_HOST;}
        }

        pcFirstDot = strchr(a_cpcSchedulerHost,'.');
        if(!pcFirstDot) return -3;

        snprintf(a_groupNameBuf,a_bufferLen,"239%s",pcFirstDot);

        // Port calculation
        *a_portPtr = LMC_PORT_NUMBER;

#if 0
        nValue = atoi(++pcFirstDot);
        *a_portPtr += nValue;

        pcFirstDot = strchr(pcFirstDot,'.');
        if(!pcFirstDot) return 0;
        nValue = atoi(++pcFirstDot);
        *a_portPtr += nValue;

        pcFirstDot = strchr(pcFirstDot,'.');
        if(!pcFirstDot) return 0;
        nValue = atoi(++pcFirstDot);
        *a_portPtr += nValue;
#endif

        return 0;
    }
}MultiCastBase;

#endif // MCAST_COMMON_APIS_H
