
// mclistener.hpp
// to include ->  #include "mclistener.hpp"
// created on 2018 Jan 22

#ifndef common_mclistener_hpp
#define common_mclistener_hpp

#ifdef _WIN32
#include <Winsock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#define errnoNew	WSAGetLastError()
typedef SOCKET  socket2_t;
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#define errnoNew	errno
typedef int  socket2_t;
#endif

#include <mcast_common_apis.h>

#ifndef NEGATIVE_ERROR_
#define	NEGATIVE_ERROR_(x)		(((x)>0) ? -(x) : (x))
#endif

#ifndef E_SELECT
#define E_SELECT		-28	/* error by select */
#endif

#ifndef SOCKET_TIMEOUT_
#define SOCKET_TIMEOUT_ -2001
#endif

#ifndef EINTR_ERROR_
#define	EINTR_ERROR_			NEGATIVE_ERROR_(EINTR)
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

class  MClistener
{
public:
	MClistener();
	~MClistener();

	int ConnectToTheMGroup(const char* schedulerHost);
	int SetSocketTimeout(int timeoutMs);
	void CloseSock();

        int recvC(void* buff, int buffLen)const;


	static bool Init();
	static void Cleanup();

private:
        socket2_t   m_sock;
        socket2_t   m_nReserved1;
	sockaddr_in m_saddr;
};


#endif  // #ifndef __mclistener_hpp__
