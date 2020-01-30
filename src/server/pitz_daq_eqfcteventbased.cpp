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
#include <pitz_daq_data_handling_types.h>

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


bool pitz::daq::EqFctEventBased::DataGetterFunctionWithWait(const SNetworkStruct* a_pNet, const ::std::vector<SingleEntry*>& a_entries)
{
    const SNetworkStructZmqDoocs* pNetZmq = static_cast< const SNetworkStructZmqDoocs* >(a_pNet);
    DEC_OUT_PD(SingleData)* dataFromNetwork;
    SingleEntryZmqDoocs* pEntry;
    time_t  currentTime;
    int nHandled(0);
    int nReturn;
    size_t unIndex;
    size_t unValidEntriesCount;
    const size_t cunEntriesCount(a_entries.size());
    // which one is the best container
    //::common::IntHash< SingleEntryZmqDoocs* >  validEntries(entriesList.size() * 2);
    //::std::vector< SingleEntryZmqDoocs* > validEntries;
    ::std::vector< SingleEntryZmqDoocs* > validEntries;

    time(&currentTime);
    if(currentTime-pNetZmq->m_lastUpdateTime>PITZ_DAQ_LIST_UPDATE_DEFAULT_TIME){
        for(unIndex=0;unIndex<cunEntriesCount;++unIndex){
            pEntry = static_cast< SingleEntryZmqDoocs* >( a_entries[unIndex] );
            if(pEntry->LoadOrValidateData(pNetZmq->m_pContext)){
                validEntries.push_back(pEntry);
            }
        }
    }
    else{
        for(unIndex=0;unIndex<cunEntriesCount;++unIndex){
            pEntry = static_cast< SingleEntryZmqDoocs* >( a_entries[unIndex] );
            if(pEntry->isValid()){
                validEntries.push_back(pEntry);
            }
        }
    }

    unValidEntriesCount = validEntries.size();
    if(unValidEntriesCount<1){return false;}
    if(unValidEntriesCount>pNetZmq->m_unCreatedItemsCount){
        zmq_pollitem_t* pItemsNew = static_cast<zmq_pollitem_t*>(realloc(pNetZmq->m_pItems,unValidEntriesCount));
        if(!pItemsNew){
            return false;
        }
    }

    for(unIndex=0;unIndex<unValidEntriesCount;++unIndex){
        pNetZmq->m_pItems[unIndex].revents = 0;
        pNetZmq->m_pItems[unIndex].socket = validEntries[unIndex]->socket();
        pNetZmq->m_pItems[unIndex].events = ZMQ_POLLIN;
    }

    nReturn=zmq_poll(pNetZmq->m_pItems,static_cast<int>(unValidEntriesCount),-1);
    if(nReturn<=0){return false;}

    for(unIndex=0;unIndex<unValidEntriesCount;++unIndex){
        pEntry=validEntries[unIndex];
        if(pNetZmq->m_pItems[unIndex].revents & ZMQ_POLLIN){
            if( (dataFromNetwork=pEntry->ReadData())){
                AddJobForRootThread(dataFromNetwork,pEntry);
                ++nHandled;
            }
            else{
                // set network error
                pEntry->SetError(1);
            }

        }

    }

    return nHandled?true:false;
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
    m_lastUpdateTime = 0;
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
    //m_expectedReadHeader2 = 0;
    m_expectedRead1 = 0;
    m_expectedRead2 = 0;
    m_nPort = PITZ_DAQ_UNKNOWN_ZMQ_PORT;
    m_isDataLoaded = 0;
    m_isValid = 0;
    m_reserved = 0;
    m_pBufferForHeader = NEWNULLPTR;
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
#if 1

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
        return NEWNULLPTR2;
    }

    if(nReturn != aDcsHeader.size){
        // this is not header give chance for header
        return NEWNULLPTR2;
    }

    switch(aDcsHeader.vers){
    case 1:
        pHeaderV1 = reinterpret_cast<struct dmsg_header_v1*>(&aDcsHeader);
        nDataType = pHeaderV1->type;
        break;
    default:
        return NEWNULLPTR2;
    }

    if(nDataType != m_branchInfo.dataType){
        return NEWNULLPTR;
    }

    if(m_expectedRead1){
        more_size = sizeof(more);
        nReturn = zmq_getsockopt (m_pSocket, ZMQ_RCVMORE, &more, &more_size);

        if(!more){
            return nullptr;
        }

        nReturn=zmq_recv(this->m_pSocket,m_pBufferForHeader,m_expectedRead1,0);
        if(nReturn!=static_cast<int>(m_expectedRead1)){
            return nullptr;
        }
    }

    if(m_expectedRead2){
        pMemory = CreateDataWithOffset(0,m_expectedRead2);
        more_size = sizeof(more);
        nReturn = zmq_getsockopt (m_pSocket, ZMQ_RCVMORE, &more, &more_size);

        if(!more){
            this->FreeUsedMemory(pMemory);
            return nullptr;
        }

        nReturn=zmq_recv(this->m_pSocket,wrPitzDaqDataFromEntry(pMemory),m_expectedRead2,0);
        if(nReturn!=static_cast<int>(m_expectedRead2)){
            this->FreeUsedMemory(pMemory);
            return nullptr;
        }
    }

    m_isValid = 1;

    return pMemory;

#else
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
#endif
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
        //if(!this->ApplyEntryInfo(0)){return false;}
        if(!PrepareDaqEntryBasedOnType(0,&m_branchInfo,&m_unOnlyDataBufferSize,&m_unTotalRootBufferSize,NEWNULLPTR2,NEWNULLPTR2,NEWNULLPTR2)){return false;}
    }
    else{
        m_rootFormatStr = PrepareDaqEntryBasedOnType(1,&m_branchInfo,&m_unOnlyDataBufferSize,&m_unTotalRootBufferSize,NEWNULLPTR2,NEWNULLPTR2,NEWNULLPTR2);
        if(!m_rootFormatStr){return false;}
    }

    m_isValid = 1;
    m_isDataLoaded = 1;

    return true;
}


