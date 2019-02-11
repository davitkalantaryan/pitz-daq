//
// file:		desy_interlocknotifier_client.cpp
// created on:	2018 Jun 22
//

#ifndef LOG_LEVEL_DECLARED
#define LOG_LEVEL_DECLARED
#endif

#include "desy/interlocknotifier/client.hpp"


desy::interlockNotifier::Client::~Client()
{
	DisconnectFromServer();
}

int desy::interlockNotifier::Client::ConnectToServer(const char* a_serverName)
{
	uint16_t unEndian;
	int nSndRcv,nReturn = m_socket.connectC(a_serverName, INTERLOCK_NOTIFIER_SERVER_PORT,2000);

	if(nReturn){m_socket.closeC();return nReturn;}

	m_socket.setTimeout(10000);

	nSndRcv=m_socket.readC(&unEndian, 2);
	if(nSndRcv!=2){m_socket.closeC();return -1;}
	if(unEndian==1){m_isEndianDiffer=0;}
	else{m_isEndianDiffer=1;}

	unEndian = 1;
	nSndRcv=m_socket.writeC(&unEndian, 2);
	if(nSndRcv!=2){m_socket.closeC();return -2;}

	m_socket.setTimeout(-1);

	return 0;
}


void desy::interlockNotifier::Client::DisconnectFromServer()
{
	m_socket.closeC();
}


int desy::interlockNotifier::Client::ReceiveInterlockData(i4_mod_ctrl_t* a_pBuffer)
{
	int nSndRcv;

	nSndRcv = m_socket.readC(a_pBuffer, sizeof(i4_mod_ctrl_t));
	if(nSndRcv != ((int)sizeof(i4_mod_ctrl_t))){return -1;}
	if(!IsInterlock(*a_pBuffer)){return 1;}
	return 0;
}


bool desy::interlockNotifier::Client::IsInterlock(const i4_mod_ctrl_t& a_ilockData)
{
	static int snIteration = 0;
	DEBUG_APP(2,"evNum=%d",(int)a_ilockData.eventCounter);
	return (snIteration++%10)==0;
}