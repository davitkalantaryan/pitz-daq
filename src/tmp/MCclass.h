/*
 * File initialy created by L. Hakobyan
 *
 * Modified by D. Kalantaryan on 11.01.2016
 * Both classes should be changed completly.
 * For time beeing to have fast solution,
 * following changes have been done
 *  a) New base class added to implement common functionalities
 *  b) Some functions redone to have them more clear
 *  c) Modifications started to compile this in windows also
 *
 */
#ifndef MCclass_h
#define MCclass_h

#ifdef WIN32
#include <win_raw_socket.h> // This file is created by me. The file helps in correct way to include ...
#else  // #ifdef WIN32
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#endif  // #ifdef WIN32

#include <mcast_common_apis.h>
#include <string>
#include <stdio.h>
#include <string.h>

#ifndef E_SELECT
#define E_SELECT		-28	/* error by select */
#endif

#ifndef _SOCKET_TIMEOUT_
#define _SOCKET_TIMEOUT_ -2001
#endif

#ifndef _NEGATIVE_ERROR_
#define	_NEGATIVE_ERROR_(x)		(((x)>0) ? -(x) : (x))
#endif

#ifndef _EINTR_ERROR_
#define	_EINTR_ERROR_			_NEGATIVE_ERROR_(EINTR)
#endif


#ifndef LAST_PATH
#define LAST_PATH(_fullPath_) (  strrchr((_fullPath_),'/') ? \
    (strrchr((_fullPath_),'/')+1) : ( strrchr((_fullPath_),'\\')?(strrchr((_fullPath_),'\\')+1):(_fullPath_) )  )
#endif


#ifndef DEBUG_IF
#define DEBUG_IF(_cond_,_out_,...) \
    do{if((_cond_)){fprintf((_out_),"fl:%s,ln:%d,fnc:%s -> ",LAST_PATH(__FILE__),__LINE__,__FUNCTION__);\
    fprintf((_out_),__VA_ARGS__);fprintf((_out_),"\n");}}while(0)
#endif

#ifndef ERR_LOG
#define ERR_LOG(...) DEBUG_IF(1,stderr,__VA_ARGS__)
#endif

#ifndef DEBUG_LOG_LEVEL
extern int g_nLogLevel;
#define DEBUG_LOG_LEVEL(_level_,...) DEBUG_IF(((_level_)<=g_nLogLevel),stdout,__VA_ARGS__)
#endif


class  MCsender
{
private:
        char	m_HOST_NAME[40];
        char  m_HELLO_GROUP[40];
        uint16_t	m_HELLO_PORT;
	
        struct hostent *m_hostent_;
	
        char       m_charptr1[400];
        char       m_charptr10[400];

public:
        MCsender(bool /*= true*/, int /*= 0*/);
        MCsender();
  ~MCsender();

  static int GroupNameAndPortAlg1_self(char* a_groupNameBuf,int a_bufferLen,int* a_portPtr);
	void init_socket();
        int init_socket_new(); // New with clear code
	struct tm * localtime_rL(const time_t *, struct tm *);	
	void printtostderrL (char*, char*);	
	int  check_sock();		
	int  get_sock();
	void close_sock();	
public:
	int sock, status;
	socklen_t	socklen;
	struct sockaddr_in saddr;
	char      loopch;
	unsigned char   ttl;
};



//============================================================
#if 0
class  MClistener
{
public:
    MClistener();
    ~MClistener();

    int ConnectToTheMGroup(const char* schedulerHost);
    void CloseSock();

    int recvC(void* buff, int buffLen, int timeoutMs);

private:
    int m_sock;
    socklen_t	m_socklen;
    sockaddr_in m_saddr;
};
#endif

#endif // #ifndef MCclass_h
