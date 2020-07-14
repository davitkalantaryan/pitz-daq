//
// file:		desy/interlocknotifier/server.hpp
// created on:	2018 Jun 21
//

#ifndef __pitz_daq_data_browserserver_hpp__
#define __pitz_daq_data_browserserver_hpp__

#include <pitz/daq/data/browser_server_client_common.h>
#include <pitz/daq/base.hpp>
#include <pitz/daq/data/bypipe/base.hpp>
#include <common/common_servertcp.hpp>
#include <common/listspecialandhashtbl.hpp>
#include <cpp11+/shared_mutex_cpp14.hpp>
#include <stdint.h>
//#include <common/common_unnamedsemaphorelite.hpp>
#include <cpp11+/thread_cpp11.hpp>
#include <cpp11+/mutex_cpp11.hpp>
#include <pitz/daq/data/bypipe/base.hpp>

namespace pitz { namespace daq{ namespace data{

struct HashingItem
{
    ComunicationStruct comStr;
    char   hostName[ MAX_HOSTNAME_LENGTH ];
};

struct ClientItem
{
    common::SocketTCP                       sockets[byPipe::pipePurpose::Count];
    HashingItem*                            pHashItem;
    common::listN::ListItem<ClientItem*>*   pListItem;
    uint64_t                                numberOfSocketsConnected : 10;
    uint64_t                                isEndianDiffer : 1;

    ClientItem(){this->numberOfSocketsConnected=this->isEndianDiffer=0;this->pHashItem=NULL;this->pListItem=NULL;}
};


//struct SPidItem{
//    common::listN::ListItem<int>* pPidItem;
//};


class BrowserServer : public pitz::daq::Base
{
public:
    BrowserServer();
    ~BrowserServer();

	int  RunServerThreads();
    void StopServerThreads(bool a_bKillProcesses);

private:
	void ThreadFunctionForListen();
    void ThreadFunctionForWait();
	void AddClient(common::SocketTCP& clientSock, const sockaddr_in*remoteAddr);

private:
    common::ServerTCP                                   m_serverTcp;
    common::List<ClientItem*>                           m_listClients;
    common::HashTbl<ClientItem*>*                       m_pHashByComStruct;
    common::List<int>*                                  m_pListPid;
    common::HashTbl< common::listN::ListItem<int>* >*   m_pPidsHash;
    STDN::mutex                                         m_mutexForClients;
    STDN::mutex                                         m_mutexForPids;
    STDN::thread                                        m_threadForListen;
    STDN::thread                                        m_threadForWait;
    pthread_t                                           m_waiterThreadHandle;
    volatile uint64_t                                   m_isWork		: 1;
    volatile uint64_t                                   m_isWorkWaiter	: 1;
    volatile uint64_t                                   m_isError		: 1;
    volatile uint64_t                                   m_isListenerStarted : 1;
    volatile uint64_t                                   m_isWaiterStarted : 1;
};

}}} // namespace desy { namespace daq{


#endif  // #ifndef __pitz_daq_data_browserserver_hpp__
