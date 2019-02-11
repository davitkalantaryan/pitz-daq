
// Created by D. Kalantaryan (davit.kalantaryan@desy.de)

#ifndef __mailsender_h
#define __mailsender_h

#include <stddef.h>

// should be declspec(dllexport or import for windows, or nothing)
#define MAIL_API_LINK

#define		MAIL_SEND_PORT		25
#define		E_NO_CONNECT2		-8	/* can not connect to server */

#define E_FATAL			-1	/* fatal error */
#define E_NO_BIND		-7	/* can not bind address to port */
#define E_NO_CONNECT	-8	/* can not connect to server */
#define E_NO_LISTEN		-14	/* can not listen */
#define E_NO_SOCKET		-18	/* no socket generated */
#define E_RECEIVE		-25	/* error by receive */
#define E_SELECT		-28	/* error by select */
#define E_SEND			-29	/* error by send */
#define E_UNKNOWN_HOST		-38	/* can not find host */
#define E_WINSOCK_VERSION	-42	/* WINSOCK DLL version not requested */
#define	E_CONN_CLOSED		-46	/* connecttion closed by peer */


#ifdef	_WIN32
#define	SOCKET_INPROGRESS2(e)	(WSAGetLastError() == WSAEWOULDBLOCK)
#else
#if defined(EALREADY) && defined(EAGAIN)
#define	SOCKET_INPROGRESS2(e)	(e == EINPROGRESS || e == EALREADY || e == EAGAIN)
#else
#ifdef  EALREADY
#define	SOCKET_INPROGRESS2(e)	(e == EINPROGRESS || e == EALREADY)
#else
#ifdef  EAGAIN
#define	SOCKET_INPROGRESS2(e)	(e == EINPROGRESS || e == EAGAIN)
#else
#define	SOCKET_INPROGRESS2(e)	(e == EINPROGRESS)
#endif
#endif
#endif
#endif

#ifdef __cplusplus
extern "C"{
#endif


MAIL_API_LINK int SendMail1(const char* smtpServerName, const char* from,
			  int nReceivers, const char* tos[], int nCCs, const char* a_ccs[],
			  const char* subject, const char* body);
MAIL_API_LINK int InitSocketLibrary(void);
MAIL_API_LINK void CleanSocketLibrary(void);


#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

int SendEmailCpp(const char* from,
                 const char* tos, const char* a_subject, const char* a_body,int nCCs=0, const char** ccs=NULL, const char* a_hostname=NULL);

#endif



#endif   // #ifndef __mailsender_h
