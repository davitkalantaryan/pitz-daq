//
// file:		desy_interlocknotifierserver.cpp
// created on:	2018 Jun21
//

#ifndef LOG_LEVEL_DECLARED
#define LOG_LEVEL_DECLARED
#endif

int g_nLogLevel = 0;

#include "desy/interlocknotifier/server.hpp"
#include <stdio.h>
#include <common/newlockguards.hpp>
#include <iostream>
#include "driver/i4/user/daq_driver.h"


desy::interlockNotifier::Server::Server()
{
	m_isRun = 0;
}


desy::interlockNotifier::Server::~Server()
{
}


void desy::interlockNotifier::Server::AddClient(common::SocketTCP& a_clientSock, const sockaddr_in* a_remoteAddr)
{
	ClientItem* pItem;
    common::NewLockGuard<STDN::mutex>  aGuard;
	int nSndRcv,nSocketDsc = (int)a_clientSock;
	uint16_t  unEndian;
	char vcBuffer[1024];

	DEBUG_APP(0,"connection: IP=%s (hostName=%s)",common::socketN::GetIPAddress(a_remoteAddr),common::socketN::GetHostName(a_remoteAddr,vcBuffer,1023));

	unEndian = 1;
	nSndRcv = a_clientSock.writeC(&unEndian, 2);
	if(nSndRcv!=2){a_clientSock.closeC();return;}

	nSndRcv = a_clientSock.readC(&unEndian, 2);
	if(nSndRcv!=2){a_clientSock.closeC();return;}

	pItem = new ClientItem;
	HANDLE_MEMORY_DEF(pItem);

	pItem->addressInfo = *a_remoteAddr;

	if(unEndian==1){pItem->isEndianDiffer=0;}
	else{pItem->isEndianDiffer=1;}

	pItem->socket.SetNewSocketDescriptor(nSocketDsc);
	a_clientSock.ResetSocketWithoutClose();

	aGuard.SetAndLockMutex(&m_mutexForList);
	m_listClients.AddData(pItem, &nSocketDsc, sizeof(int));
	DEBUG_APP(1,"Number of clients: %d", m_listClients.count());
	aGuard.UnsetAndUnlockMutex();

}


int desy::interlockNotifier::Server::RunServerThreads()
{
	m_isRun = 1;
	m_isNotifierStarted = 0;
	//m_threadForAddRemove = STDN::thread(&Server::ThreadFunctionForAddAndRemove,this);
	//m_threadForListen = STDN::thread(&common::ServerTCP::StartServer,&m_serverTcp,&Server::AddClient,INTERLOCK_NOTIFIER_SERVER_PORT);
	m_threadForListen = STDN::thread(&Server::ThreadFunctionForListen,this);
	//m_serverTcp.StartServer(this,&Server::AddClient,INTERLOCK_NOTIFIER_SERVER_PORT);
	m_threadForNotification = STDN::thread(&Server::ThreadFunctionForNotification, this);
	while(m_isRun && (m_isNotifierStarted==0)){Sleep(1);}
	return m_isNotifierStarted ? 0 : -1;
}


void desy::interlockNotifier::Server::StopServerThreads()
{
	if(!m_isNotifierStarted){return;}
	m_isRun = 0;
	m_serverTcp.StopServer();
	m_threadForNotification.join();
}


void desy::interlockNotifier::Server::ThreadFunctionForListen()
{
	m_serverTcp.StartServer(this, &Server::AddClient, INTERLOCK_NOTIFIER_SERVER_PORT);
}


void desy::interlockNotifier::Server::ThreadFunctionForNotification()
{
	i4_mod_ctrl_t    aValueToSend;
	ClientItem *pItem, *pItemNext;
    //common::NewSharedLockGuard<STDN::shared_mutex>  aSharedGuard;
	common::NewLockGuard<STDN::mutex>  aGuard;
	int nSndRcv,nError;
	char vcBuffer[1024];

	nError = i4_daq_open(0);
	if (nError != ERROR_NONE){
		DEBUG_APP(0, "error open interlock daq driver status: %d", nError);
		return;
	}

	m_isNotifierStarted = 1;

	while(m_isRun){
		nError=WaitInfoFromDriver(&aValueToSend);
		if(nError){
			//Sleep(10000);
			//continue;
			exit(1);
		}
		aGuard.SetAndLockMutex(&m_mutexForList);
		pItem = m_listClients.first();
		while(pItem){
			pItemNext = pItem->next;
			nSndRcv=pItem->socket.writeC(&aValueToSend, sizeof(aValueToSend));
			if(nSndRcv!=sizeof(aValueToSend)){
				DEBUG_APP(0, "disconnected: IP=%s (hostName=%s)",common::socketN::GetIPAddress(&pItem->addressInfo), common::socketN::GetHostName(&pItem->addressInfo,vcBuffer,1023));
				m_listClients.RemoveData(pItem);
				delete pItem;
				DEBUG_APP(1,"Number of clients: %d", m_listClients.count());
			}
			pItem = pItemNext;
		}
		aGuard.UnsetAndUnlockMutex();
	}  // while(m_isRun){

	i4_daq_close();

}



int desy::interlockNotifier::Server::WaitInfoFromDriver(i4_mod_ctrl_t* a_buffer)
{
	int r;

	while(true){
		r = i4_daq_waitIrq(5000);

		if (r == ERROR_NONE)
		{
			i4_snapshot_t* snapshot = i4_daq_getSnapshot();
			if (!snapshot)
			{
				// fatal error, no pointer to data
				std::cout << "Error, NULL-snapshot returned by daq driver!" << std::endl;
				return -1;
			}
			else{
				// Got interlock Data!
				DEBUG_APP(2,"got data! size=%d",(int)sizeof(*a_buffer));

				memcpy(a_buffer,&(snapshot->ctrlModule),sizeof(i4_mod_ctrl_t));
				return 0;
			}

			// all is ok .. do something with the data

		} else if (r == ERROR_TIMEOUT) {
			DEBUG_APP(2,"timeout");
			// std::cout << "Error, Timeout!" << std::endl;
			//return 0;
		} else {
			// fatal error
			std::cout << "Error, error returned by daq driver!" << std::endl;
			return -1;
		}

	}



/*
	int nReturn = 10000 + rand() % 6000;
	Sleep(nReturn);
	return nReturn;
*/
}

