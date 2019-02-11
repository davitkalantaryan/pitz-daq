//
// file:		desy_interlocknotifierserver.cpp
// created on:	2018 Jun21
//

#include <common_daq_definations.h>
#include "pitz/daq/data/browserserver.hpp"
#include "../tools/bin_for_mexdaq_browser_common.h"
#include <stdio.h>
#include <common/newlockguards.hpp>
#include <iostream>
#ifdef _WIN32
#else
#include <sys/wait.h>
#endif

using namespace pitz::daq;

#if 0
common::ServerTCP                                   m_serverTcp;
common::List<ClientItem*>                           m_listClients;
common::HashTbl<ClientItem*>*                       m_pHashByComStruct;
common::List<ClientItem*>*                          m_pListPid;
common::HashTbl< common::listN::ListItem<int>* >*   m_pPidsHash;
STDN::mutex                                         m_mutexForList;
STDN::thread                                        m_threadForListen;
STDN::thread                                        m_threadForWait;
volatile uint64_t                                   m_isWork		: 1;
volatile uint64_t                                   m_isError		: 1;
volatile uint64_t                                   m_isListenerStarted : 1;
volatile uint64_t                                   m_isWaiterStarted : 1;
#endif


data::BrowserServer::BrowserServer()
{
    m_waiterThreadHandle = (pthread_t)0;
    m_pHashByComStruct = new common::HashTbl<ClientItem*>;
    m_pListPid = new common::List<int>;
    m_pPidsHash = new common::HashTbl< common::listN::ListItem<int>* > ;
    m_isWork = m_isWorkWaiter=m_isError=m_isListenerStarted= m_isWaiterStarted=0;
    m_isInited = 1;
}


data::BrowserServer::~BrowserServer()
{
    StopServerThreads(true);
    delete m_pHashByComStruct;
    delete m_pListPid;
    delete m_pPidsHash ;
}


void data::BrowserServer::AddClient(common::SocketTCP& a_clientSock, const sockaddr_in* a_remoteAddr)
{
    HashingItem hashItem;
    ClientItem* pClientItem = NULL;
    int32_t nIndex;
    common::NewLockGuard<STDN::mutex>  aGuard;
    int nSndRcv;
	uint16_t  unEndian;

    memset(&hashItem,0,sizeof(HashingItem));

    MAKE_REPORT_GLOBAL(0,"connection: IP=%s (hostName=%s)",
                       common::socketN::GetIPAddress(a_remoteAddr),
                       common::socketN::GetHostName(a_remoteAddr,hashItem.hostName,MAX_HOSTNAME_LENGTH));

	unEndian = 1;
	nSndRcv = a_clientSock.writeC(&unEndian, 2);
	if(nSndRcv!=2){a_clientSock.closeC();return;}

    nSndRcv = a_clientSock.readC(&hashItem.comStr,sizeof(hashItem.comStr));
    if(nSndRcv!=sizeof(hashItem.comStr)){a_clientSock.closeC();return;}
    nIndex = hashItem.comStr.index;
    hashItem.comStr.index=0;

    if( (nIndex<0) || (nIndex>=byPipe::pipePurpose::Count) ){
        MAKE_ERROR_THIS("wrong pipe index (%d) provided by %s",nIndex,hashItem.hostName);
        return;
    }

    aGuard.SetAndLockMutex(&m_mutexForClients);

    if( !m_pHashByComStruct->FindEntry(&hashItem,sizeof(hashItem),&pClientItem) ){
        pClientItem = new ClientItem;
        HANDLE_MEM_DEF(pClientItem," ");
        pClientItem->pHashItem = (HashingItem*)m_pHashByComStruct->AddEntry2(&hashItem,sizeof(hashItem),pClientItem);
        pClientItem->pListItem = m_listClients.AddData(pClientItem);
    }
    else if(pClientItem->sockets[nIndex]>0){
        pClientItem->sockets[nIndex].closeC();
    }

    pClientItem->sockets[nIndex].SetNewSocketDescriptor((int)a_clientSock);
    a_clientSock.ResetSocketWithoutClose();
    if(++(pClientItem->numberOfSocketsConnected)==byPipe::pipePurpose::Count){
        int nPid;

        m_listClients.RemoveData(pClientItem->pListItem);
        m_pHashByComStruct->RemoveEntry(pClientItem->pHashItem,sizeof(HashingItem));

        aGuard.UnsetAndUnlockMutex();

        nPid = fork();
        if(nPid){
            common::listN::ListItem<int>* pPidListItem;
            aGuard.SetAndLockMutex(&m_mutexForPids);
            pPidListItem = m_pListPid->AddData(nPid);
            m_pPidsHash->AddEntry2(&nPid,4,pPidListItem);
            aGuard.UnsetAndUnlockMutex();
            for(nIndex=0;nIndex<byPipe::pipePurpose::Count;++nIndex){pClientItem->sockets[nIndex].closeC();}
        }
        else{
            char* argv[16];
            int nOffSet(0);
            char vectPipes[128];
            char vcLogLevel[32];

            for(nIndex=0;nIndex<byPipe::pipePurpose::Count;++nIndex){
                nOffSet += (snprintf(vectPipes+nOffSet,128-nOffSet,"%d, ",(int)pClientItem->sockets[nIndex])-1);
                pClientItem->sockets[nIndex].ResetSocketWithoutClose();
            }

            MAKE_REPORT_THIS(1,"Pipe string: \"%s\"",vectPipes);
            snprintf(vcLogLevel,127,"%d",(int)log::g_nLogLevel);

            argv[0] = const_cast<char*>(HELPER_BINARY_NAME);
            argv[1] = const_cast<char*>(TO_DO_MAKE_INTERACTIVE_LOOP);
            argv[2] = const_cast<char*>(OPTION_NAME_USE_PRIV_PIPE);
            argv[3]=vectPipes;
            argv[4] = const_cast<char*>(OPTION_NAME_LOG_LEVEL);
            argv[5] =vcLogLevel;
            if(m_isDebug){argv[6] = const_cast<char*>(OPTION_NAME_WAIT_FOR_DEBUG);argv[7] = NULL;}
            else{argv[6] = NULL;}

            delete pClientItem;

            //StopServerThreads(false);
            //aGuard.SetAndLockMutex(&m_mutexForClients);
            while(m_listClients.first()){
                pClientItem = m_listClients.first()->data;
                m_pHashByComStruct->RemoveEntry(pClientItem->pHashItem,sizeof(HashingItem));
                m_listClients.RemoveData(pClientItem->pListItem);
                for(nIndex=0;nIndex<byPipe::pipePurpose::Count;++nIndex){pClientItem->sockets[nIndex].closeC();}
                delete pClientItem;
            }
            //aGuard.UnsetAndUnlockMutex();

            delete m_pHashByComStruct;m_pHashByComStruct=NULL;
            delete m_pListPid;m_pListPid=NULL;
            delete m_pPidsHash ;m_pPidsHash=NULL;

            execvp (argv[0], argv);
        }
    }
    else{aGuard.UnsetAndUnlockMutex();}

}


int data::BrowserServer::RunServerThreads()
{
    m_isWork = 1;
    m_isWorkWaiter = 1;
    m_isError = 0;
    m_threadForListen = STDN::thread(&BrowserServer::ThreadFunctionForListen,this);//ThreadFunctionForWait
    m_threadForWait = STDN::thread(&BrowserServer::ThreadFunctionForWait,this);//ThreadFunctionForWait

    while( m_isWork && (m_isError==0) &&
           ((m_isListenerStarted==0)||(m_isWaiterStarted==0)) )
    {SleepMsInt(1);}

    if( m_isError ){StopServerThreads(true);return -1;}
    return 0;
}


void data::BrowserServer::StopServerThreads(bool a_bKillProcesses)
{
    int nIndex,nPid;
    ClientItem* pClientItem;
    common::NewLockGuard<STDN::mutex>  aGuard;
    bool bProcessKIlled(false);

    if(!m_isWork){return;}
    m_isWork = 0;
	m_serverTcp.StopServer();

    aGuard.SetAndLockMutex(&m_mutexForClients);
    while(m_listClients.first()){
        pClientItem = m_listClients.first()->data;
        m_pHashByComStruct->RemoveEntry(pClientItem->pHashItem,sizeof(HashingItem));
        m_listClients.RemoveData(pClientItem->pListItem);
        for(nIndex=0;nIndex<byPipe::pipePurpose::Count;++nIndex){pClientItem->sockets[nIndex].closeC();}
        delete pClientItem;
    }
    aGuard.UnsetAndUnlockMutex();

    aGuard.SetAndLockMutex(&m_mutexForPids);
    while(m_pListPid->first()){
        nPid = m_pListPid->first()->data;
        if(a_bKillProcesses){kill(nPid,SIGKILL);bProcessKIlled=true;}
        m_pPidsHash->RemoveEntry(&nPid,4);
    }
    aGuard.UnsetAndUnlockMutex();

    if(bProcessKIlled){SleepMsInt(500);}

    m_isWorkWaiter = 0;
    if(m_waiterThreadHandle){pthread_kill(m_waiterThreadHandle,SIGINT);}

    m_threadForListen.join();
    m_threadForWait.join();
}


void data::BrowserServer::ThreadFunctionForListen()
{    
    m_isListenerStarted=1;
threadEntryPoint:
    try{
        if(m_isWork){m_serverTcp.StartServer(this, &BrowserServer::AddClient, PITZ_DAQ_BROWSER_SERVER_DEFAULT_PORT);}
    }
    catch(...){
        MAKE_ERROR_THIS("Unknown error accured!");
        goto threadEntryPoint;
    }
    m_isListenerStarted=0;
}


void data::BrowserServer::ThreadFunctionForWait()
{
    common::listN::ListItem<int>* pPidListItem;
    common::NewLockGuard<STDN::mutex>  aGuard;
    int nStatLoc, nPid;

    m_waiterThreadHandle = pthread_self();

    m_isWaiterStarted=1;
threadEntryPoint:
    try{
        while(m_isWork||m_isWorkWaiter){
            //nPid=wait(&nStatLoc);
            nPid=waitpid( nPid, &nStatLoc, WUNTRACED | WCONTINUED | WNOHANG );SleepMsInt(2000);
            if(nPid<0){continue;}
            if( WIFEXITED(nStatLoc)||WIFSIGNALED(nStatLoc) ) {
                aGuard.SetAndLockMutex(&m_mutexForPids);
                if( m_pPidsHash->RemoveEntry2(&nPid,4,&pPidListItem) ){
                    m_pListPid->RemoveData(pPidListItem);
                }
                aGuard.UnsetAndUnlockMutex();
            }
        }
    }
    catch(...){
        aGuard.UnsetAndUnlockMutex();
        MAKE_ERROR_THIS("Unknown error accured!");
        goto threadEntryPoint;
    }
    m_isWaiterStarted=0;
}
