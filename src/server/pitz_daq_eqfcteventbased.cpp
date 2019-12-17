/*
 * Remaining questions
 *  1) pitzbpmadc_rpc_server.cc, Lns: 1025-1042 (imast@?)
 */

#include "pitz_daq_eqfcteventbased.hpp"
#include <zmq.h>
#include "pitz_daq_singleentrydoocs.hpp"
#include <event_based_common_header.h>
#include <eq_client.h>
#include "pitz_daq_eqfcteventbased.cpp.hpp"
#include <thread>
#include <common/inthash.hpp>

#ifndef HANDLE_LOW_MEMORY
#define HANDLE_LOW_MEMORY(_memory,...) do{if(!(_memory)){exit(1);}}while(0)
#endif

using namespace pitz::daq;


EqFct* eq_create(int a_eq_code, void* /*a_arg*/)
{
    ::EqFct* pRet = NEWNULLPTR;
    //getchar();

    switch (a_eq_code)
    {
    case CODE_EVENT_BASED_DAQ:
        pRet = new pitz::daq::EqFctEventBased;
        break;
    default: break;
    }
    return pRet;
}


/*////////////////////////////////////////////////////////////////////*/


pitz::daq::EqFctEventBased::EqFctEventBased()
{
}


pitz::daq::EqFctEventBased::~EqFctEventBased()
{
}


int pitz::daq::EqFctEventBased::fct_code()
{
    return CODE_EVENT_BASED_DAQ;
}


pitz::daq::SNetworkStruct* pitz::daq::EqFctEventBased::CreateNewNetworkStruct()
{
    return new SNetworkStructZmqDoocs(this);
}


pitz::daq::SingleEntry* pitz::daq::EqFctEventBased::CreateNewEntry(entryCreationType::Type a_creationType,const char* a_entryLine)
{
    const char* cpcLine;
    SingleEntryZmqDoocs* pEntry = new SingleEntryZmqDoocs(a_creationType,a_entryLine,&cpcLine);

    if(!pEntry){return pEntry;}

    return pEntry;
}


void pitz::daq::EqFctEventBased::DataGetterThread(SNetworkStruct* a_pNet)
{
    SNetworkStructZmqDoocs* pNetZmq = static_cast< SNetworkStructZmqDoocs* >(a_pNet);
    DEC_OUT_PD(SingleData)* dataFromNetwork;
    SingleEntryZmqDoocs* pEntry;
    time_t  lastUpdateTime = 0, currentTime;
    size_t unNumbeOfEntries(0);
    int i;
    int nReturn;
    const ::std::list< SingleEntry* >& entriesList = a_pNet->daqEntries();
    ::std::list< SingleEntry* >::const_iterator pIter, pIterEnd;
    // which one is the best container
    //::common::IntHash< SingleEntryZmqDoocs* >  validEntries(entriesList.size() * 2);
    //::std::vector< SingleEntryZmqDoocs* > validEntries;
    ::std::list< SingleEntryZmqDoocs* > validEntries;
    ::std::list< SingleEntryZmqDoocs* >::iterator listIter, listIterTmp, listIterEnd;

    while(shouldWork() && a_pNet->shouldRun() ){

        time(&currentTime);

        if(currentTime-lastUpdateTime>PITZ_DAQ_LIST_UPDATE_DEFAULT_TIME){

            validEntries.clear();

            this->ReadLockEntries2();

            pNetZmq->ResizeItemsCount();
            pIterEnd = entriesList.end();
            for(pIter=entriesList.begin();pIter!=pIterEnd;++pIter){
                pEntry = static_cast< SingleEntryZmqDoocs* >( *pIter );
                if(pEntry->LoadOrValidateData(pNetZmq->m_pContext)){
                    if(pEntry->lockEntryForNetwork()){
                        validEntries.push_back(pEntry);
                        //validEntries.AddNewElement(pEntryCheck->socket(),pEntryCheck);
                    }
                }
            }

            this->ReadUnlockEntries2();

            unNumbeOfEntries = validEntries.size();
            if(unNumbeOfEntries<1){
                SleepMs(1);
                return;
            }

            lastUpdateTime = currentTime;

        }  // if(currentTime-lastUpdateTime>PITZ_DAQ_LIST_UPDATE_DEFAULT_TIME){

        unNumbeOfEntries = validEntries.size();
        if(unNumbeOfEntries<1){
            SleepMs(1);
            return;
        }

        i = 0;
        //for(auto pEntry : validEntries){
        for(listIter=validEntries.begin(),listIterEnd=validEntries.end(),pEntry=validEntries.front();listIter!=listIterEnd;++listIter,pEntry=*listIter){
            pNetZmq->m_pItems[i].revents = 0;
            pNetZmq->m_pItems[i].socket = pEntry->socket();
            pNetZmq->m_pItems[i].events = ZMQ_POLLIN;
            ++i;
        }

        nReturn=zmq_poll(pNetZmq->m_pItems,static_cast<int>(unNumbeOfEntries),-1);

        if(nReturn<=0){
            continue;
        }

        i = 0;
        listIterEnd=validEntries.end();
        for(listIter=validEntries.begin();listIter!=listIterEnd;){
            pEntry=*listIter;
            if(pNetZmq->m_pItems[i].revents & ZMQ_POLLIN){
                if( (dataFromNetwork=pEntry->ReadData())){
                    AddJobForRootThread(dataFromNetwork,pEntry);
                }
                else{
                    // set network error
                }
            }

            if(pEntry->resetNetworkLockAndReturnIfDeletable()){
                listIterTmp = listIter;
                ++listIterTmp;
                validEntries.erase(listIter);
                delete pEntry;
                listIter = listIterTmp;
            }
            else{
                ++listIter;
            }
            ++i;
        }
    }  // while(shouldWork() && a_pNet->shouldRun()){

}


/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
pitz::daq::SNetworkStructZmqDoocs::SNetworkStructZmqDoocs( EqFctCollector* a_pParentCollector )
    :
      SNetworkStruct(a_pParentCollector)
{
    m_pContext = zmq_ctx_new();
    HANDLE_LOW_MEMORY(m_pContext, "Unable to create ZMQ context");
    m_pItems = NEWNULLPTR2;
    m_unCreatedItemsCount = 0;
}


pitz::daq::SNetworkStructZmqDoocs::~SNetworkStructZmqDoocs()
{
    free(m_pItems);

    if(m_pContext){
        zmq_ctx_destroy(m_pContext);
    }

}


void pitz::daq::SNetworkStructZmqDoocs::ResizeItemsCount()
{
    const size_t newCount(m_daqEntries.size());
    if(newCount>m_unCreatedItemsCount){
        zmq_pollitem_t* pItemsTmp = static_cast<zmq_pollitem_t*>(realloc(m_pItems,sizeof(zmq_pollitem_t)*newCount));
        HANDLE_LOW_MEMORY(pItemsTmp);
        m_pItems = pItemsTmp;
        m_unCreatedItemsCount = newCount;
    }
}

/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
pitz::daq::SingleEntryZmqDoocs::SingleEntryZmqDoocs(entryCreationType::Type a_creationType,const char* a_entryLine, TypeConstCharPtr* a_pHelper)
    :
      SingleEntryDoocs(a_creationType,a_entryLine,a_pHelper)
{
    m_hostName = "";
    m_pSocket = NEWNULLPTR;
    m_expectedReadHeader2 = 0;
    m_nPort = PITZ_DAQ_UNKNOWN_ZMQ_PORT;
    m_isDataLoaded = 0;
    m_isValid = 0;
    m_reserved = 0;
    m_pBufferForHeader2 = NEWNULLPTR;
}


pitz::daq::SingleEntryZmqDoocs::~SingleEntryZmqDoocs()
{
    ::std::cout << __FUNCTION__ << ::std::endl;
    if(m_pSocket){
        zmq_close(m_pSocket);
        m_pSocket = NEWNULLPTR;
    }
}


void* SingleEntryZmqDoocs::socket()const
{
    return m_pSocket;
}


DEC_OUT_PD(SingleData)* SingleEntryZmqDoocs::ReadData()
{
    int nReturn;
    int nDataType;
    dmsg_hdr_t aDcsHeader;
    struct dmsg_header_v1* pHeaderV1;
    DEC_OUT_PD(SingleData)* pMemory=nullptr;

    size_t     more_size;
    int        more;

#ifdef USE_ZMQ_MESSAGES
    zmq_msg_t  aMsg;
    zmq_msg_init (&aMsg);
    nReturn=zmq_msg_recv(&aMsg,pItems[0].socket,0);
    // ...
#endif

    m_isValid = 0;

    nReturn=zmq_recv(this->m_pSocket,&aDcsHeader,sizeof(dmsg_hdr_t),0);
    if(nReturn<4){
        // todo: set proper error code
        //return PITZ_DAQ_EV_BASED_DCS_ZMQ_SYNTAX_ERROR;
        return NEWNULLPTR;
    }

    if(nReturn != aDcsHeader.size){
        // this is not header give chance for header
        //return PITZ_DAQ_EV_BASED_DCS_ZMQ_UNKNOWN_HEADER;
        // todo: set proper error code
        return NEWNULLPTR;
    }

    switch(aDcsHeader.vers){
    case 1:
        pHeaderV1 = reinterpret_cast<struct dmsg_header_v1*>(&aDcsHeader);
        nDataType = pHeaderV1->type;
        break;
    default:
        //return PITZ_DAQ_EV_BASED_DCS_ZMQ_UNKNOWN_HEADER;
        // todo: set proper error code
        return NEWNULLPTR;
    }

    if(nDataType != m_branchInfo.dataType){
        //return PITZ_DAQ_DATA_TYPE_MISMATCH;
        // todo: set proper error code
        return NEWNULLPTR;
    }

    if(m_expectedReadHeader2){
        more_size = sizeof(more);
        nReturn = zmq_getsockopt (m_pSocket, ZMQ_RCVMORE, &more, &more_size);

        if(!more){
            // todo: set proper error code
            return NEWNULLPTR;
        }

        nReturn=zmq_recv(this->m_pSocket,m_pBufferForHeader2,m_expectedReadHeader2,0);
        if(nReturn!=static_cast<int>(m_expectedReadHeader2)){
            // todo: set proper error code
            return NEWNULLPTR;
        }
    }

    if(m_onlyNetDataBufferSize>0){
        more_size = sizeof(more);
        nReturn = zmq_getsockopt (m_pSocket, ZMQ_RCVMORE, &more, &more_size);

        if(!more){
            // todo: set proper error code
            return NEWNULLPTR;
        }

        pMemory = this->GetNewMemoryForNetwork();
        if(!pMemory){
            ERROR_OUT_APP("Unable to get buffer for network");
            return NEWNULLPTR;
        }

        nReturn=zmq_recv(this->m_pSocket,wrPitzDaqDataFromEntry(pMemory),m_onlyNetDataBufferSize,0);
        if(nReturn!=static_cast<int>(m_onlyNetDataBufferSize)){
            // todo: set proper error code
            this->SetMemoryBack(pMemory);
            return NEWNULLPTR;
        }

        pMemory->eventNumber = static_cast<decltype (pMemory->eventNumber)>(aDcsHeader.ident);
        pMemory->timestampSeconds = aDcsHeader.sec;
    }

    m_isValid = 1;

    return pMemory;
}


bool SingleEntryZmqDoocs::LoadOrValidateData(void* a_pContext)
{
    int nReturn;
    int nType;
    ::std::string propToSubscribe;
    ::std::string zmqEndpoint;
    EqCall eqCall;
    EqData dataIn, dataOut;
    EqAdr eqAddr;

    eqAddr.adr(m_doocsUrl);
    propToSubscribe = eqAddr.property();
    eqAddr.set_property("SPN");
    dataIn.set (1, 0.0f, 0.0f, static_cast<time_t>(0), propToSubscribe.c_str(), 0);

    nReturn=eqCall.set(&eqAddr,&dataIn,&dataOut);
    m_isValid = 0;
    if(nReturn){
        // we have error
        ::std::string errorString = dataOut.get_string();
        ::std::cerr << errorString << ::std::endl;
        m_isDataLoaded = 0;
        return false;
    }

    if(m_isDataLoaded){
        // todo: make check
        return true;
    }

    nType=dataOut.type();

    switch(nType){
    case DATA_INT:{
        this->m_nPort = dataOut.get_int();
    }break;
    case DATA_A_USTR:{
        float f1, f2;
        time_t tm;
        char         *sp;
        dataOut.get_ustr (&this->m_nPort, &f1, &f2, &tm, &sp, 0);
        //this->m_dataType = static_cast<int>(f1);
        if(this->m_branchInfo.dataType != static_cast<decltype (this->m_branchInfo.dataType)>(f1) ){
            DEBUG_APP_INFO(0,"Data type for entry %s is changed from %d to %d",
                           daqName(),static_cast<int>(this->m_branchInfo.dataType),static_cast<int>(f1));
            this->m_branchInfo.dataType = static_cast<decltype (this->m_branchInfo.dataType)>(f1);
        }
    }break;
    default:
        return false;
    }  // switch(nType){

    nReturn=eqCall.get_option(&eqAddr,&dataIn,&dataOut,EQ_HOSTNAME);
    if(nReturn<0){
        return false;
    }

    this->m_hostName = dataOut.get_string();

    zmqEndpoint = ::std::string("tcp://") + this->m_hostName + ":" + ::std::to_string(this->m_nPort);
    this->m_pSocket = zmq_socket(a_pContext,ZMQ_SUB);
    if(!this->m_pSocket){
        return false;
    }
    nReturn = zmq_setsockopt (this->m_pSocket, ZMQ_SUBSCRIBE,nullptr, 0);
    if(nReturn){
        return false;
    }

    nReturn = zmq_connect (this->m_pSocket, zmqEndpoint.c_str());
    if(nReturn){
        return false;
    }

    if(m_rootFormatStr){
        if(!this->ApplyEntryInfo(0)){return false;}
    }
    else{
        m_rootFormatStr = this->ApplyEntryInfo(0);
        if(!m_rootFormatStr){return false;}
    }

    m_isValid = 1;
    m_isDataLoaded = 1;

    return true;
}


