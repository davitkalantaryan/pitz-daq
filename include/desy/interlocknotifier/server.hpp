//
// file:		desy/interlocknotifier/server.hpp
// created on:	2018 Jun 21
//

#ifndef __desy_interlocknotifier_server_hpp__
#define __desy_interlocknotifier_server_hpp__


#include <common/common_servertcp.hpp>
#include <common/listspecialandhashtbl.hpp>
#include <cpp11+/shared_mutex_cpp14.hpp>
#include <stdint.h>
//#include <common/common_unnamedsemaphorelite.hpp>
#include <cpp11+/thread_cpp11.hpp>
#include <cpp11+/mutex_cpp11.hpp>
#include "desy/interlocknotifier/server_client_common.h"

namespace desy { namespace interlockNotifier{


struct ClientItem
{
	ClientItem	*prev, *next;
	void* key; size_t keyLength;
	sockaddr_in			addressInfo;
	common::SocketTCP	socket;
	uint64_t			isCreation : 1;
	uint64_t			isEndianDiffer : 1;
};

class Server
{
public:
	Server();
	~Server();

	int  RunServerThreads();
	void StopServerThreads();

private:
	void ThreadFunctionForNotification();
	void ThreadFunctionForListen();
	//void ThreadFunctionForAddAndRemove();
	void AddClient(common::SocketTCP& clientSock, const sockaddr_in*remoteAddr);

	int  WaitInfoFromDriver(i4_mod_ctrl_t* a_buffer);

private:
	common::ServerTCP							m_serverTcp;
	common::ListspecialAndHashtbl<ClientItem*>	m_listClients;
	//common::ListSpecial<ClientItem*>			m_listAddRem;
    //STDN::shared_mutex							m_mutexForList;
	STDN::mutex									m_mutexForList;
	//common::UnnamedSemaphoreLite				m_semaForAddRemove;
	STDN::thread								m_threadForListen;
	STDN::thread								m_threadForNotification;
	//STDN::thread								m_threadForAddRemove;
	uint64_t									m_isRun		: 1;
	uint64_t									m_isNotifierStarted : 1;
};

}} // namespace desy { namespace interlockNotifier{


#endif  // #ifndef __desy_interlocknotifierserver_hpp__
