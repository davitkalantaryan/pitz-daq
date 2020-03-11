/*
 * Remaining questions
 *  1) pitzbpmadc_rpc_server.cc, Lns: 1025-1042 (imast@?)
 */

#include "pitz_daq_eqfcteventbased.hpp"
#include <zmq.h>
#include "pitz_daq_singleentrydoocs_base.hpp"
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
    return new SingleEntryZmqDoocs(a_creationType,a_entryLine,&cpcLine);
}


void pitz::daq::EqFctEventBased::DataGetterFunctionWithWait(const SNetworkStruct* a_pNet, const ::std::vector<SingleEntry*>& a_entries)
{
    const SNetworkStructZmqDoocs* pNetZmq = static_cast< const SNetworkStructZmqDoocs* >(a_pNet);
    DEC_OUT_PD(SingleData2)* dataFromNetwork;
    SingleEntryZmqDoocs* pEntry;
    int nReturn;
    size_t unIndex;
    size_t unValidEntriesCount;
    const size_t cunEntriesCount(a_entries.size());
    // which one is the best container
    //::common::IntHash< SingleEntryZmqDoocs* >  validEntries(entriesList.size() * 2);
    //::std::vector< SingleEntryZmqDoocs* > validEntries;
    ::std::vector< SingleEntryZmqDoocs* > validEntries;

    for(unIndex=0;unIndex<cunEntriesCount;++unIndex){
        pEntry = static_cast< SingleEntryZmqDoocs* >( a_entries[unIndex] );
        if(!pEntry->isValid()){
            pEntry->LoadOrValidateData(pNetZmq->m_pContext);
        }
        if(pEntry->isValid()){
            validEntries.push_back(pEntry);
        }
    }

    unValidEntriesCount = validEntries.size();
    if(unValidEntriesCount<1){return ;}
    if(!pNetZmq->ResizeItemsCount(unValidEntriesCount)){return;}

    for(unIndex=0;unIndex<unValidEntriesCount;++unIndex){
        pNetZmq->m_pItems[unIndex].fd = 0;
        pNetZmq->m_pItems[unIndex].revents = 0;
        pNetZmq->m_pItems[unIndex].socket = validEntries[unIndex]->socket();
        pNetZmq->m_pItems[unIndex].events = ZMQ_POLLIN;
    }

    nReturn=zmq_poll(pNetZmq->m_pItems,static_cast<int>(unValidEntriesCount),-1);
    if(nReturn<=0){return ;}

    for(unIndex=0;unIndex<unValidEntriesCount;++unIndex){
        pEntry=validEntries[unIndex];
        if(pNetZmq->m_pItems[unIndex].revents & ZMQ_POLLIN){
            if( (dataFromNetwork=pEntry->ReadData())){
                AddJobForRootThread(dataFromNetwork,pEntry);
            }
            else{
                // set network error
                pEntry->SetError(NETWORK_READ_ERROR, "Unable to get data via zmq");
            }

        }

    }

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
    StopThreadThenDeleteAndClearEntries();

    free(m_pItems);

    if(m_pContext){
        zmq_ctx_destroy(m_pContext);
    }

}


bool pitz::daq::SNetworkStructZmqDoocs::ResizeItemsCount(size_t a_unNewSize)const
{
    if(a_unNewSize>m_unCreatedItemsCount){
        zmq_pollitem_t* pItemsTmp = static_cast<zmq_pollitem_t*>(realloc(m_pItems,sizeof(zmq_pollitem_t)*a_unNewSize));
        HANDLE_LOW_MEMORY(pItemsTmp);
        m_pItems = pItemsTmp;
        m_unCreatedItemsCount = a_unNewSize;
    }

    return true;
}

/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#define SPECIAL_KEY_ENDPOINT           "zmq_endpoint"
#define SPECIAL_KEY_READ1              "read1"
#define SPECIAL_KEY_READ2              "read2"

pitz::daq::SingleEntryZmqDoocs::SingleEntryZmqDoocs(entryCreationType::Type a_creationType,const char* a_entryLine, TypeConstCharPtr* a_pHelper)
    :
      SingleEntryDoocsBase(a_creationType,a_entryLine,a_pHelper),
      m_zmqEndpoint(SPECIAL_KEY_ENDPOINT)
{
    //bool bCallIniter = false, bIsAddedByUser = false;
    bool bCallIniter = false;

    AddNewParameterToEnd(&m_zmqEndpoint,false,false);

    switch(a_creationType)
    {
    case entryCreationType::fromOldFile:
        break;
    case entryCreationType::fromConfigFile:
        bCallIniter = true;
        break;
    case entryCreationType::fromUser:
        bCallIniter = true;
        //bIsAddedByUser = true;
        break;
    default:
        throw errorsFromConstructor::type;
    }

    if(bCallIniter){
        //LoadFromLine(a_entryLine,true,bIsAddedByUser);
        m_zmqEndpoint.FindAndGetFromLine(a_entryLine);
    }

    m_pSocket = NEWNULLPTR;
    m_secondHeaderLength = 0;
    m_expectedDataLength = 0;
    m_isDataLoaded = 0;
    m_reserved64bit = 0;
    m_pBufferForSecondHeader = NEWNULLPTR;
}


pitz::daq::SingleEntryZmqDoocs::~SingleEntryZmqDoocs()
{
    ::std::cout << __FUNCTION__ << " m_pSocket=" << m_pSocket << ::std::endl;
    if(m_pSocket){
        zmq_close(m_pSocket);
        m_pSocket = NEWNULLPTR;
    }
}


void* SingleEntryZmqDoocs::socket()const
{
    return m_pSocket;
}


DEC_OUT_PD(SingleData2)* SingleEntryZmqDoocs::ReadData()
{
    int nReturn;
    int nDataType;
    dmsg_hdr_t aDcsHeader;
    struct dmsg_header_v1* pHeaderV1;
    DEC_OUT_PD(SingleData2)* pMemory=nullptr;
    size_t     more_size;
    int        more;
    DEC_OUT_PD(Header) aHeader;


    nReturn=zmq_recv(this->m_pSocket,&aDcsHeader,sizeof(dmsg_hdr_t),0);
    if(nReturn<4){
        goto returnPoint;
    }

    if(nReturn != aDcsHeader.size){
        // this is not header give chance for header
        goto returnPoint;
    }

    switch(aDcsHeader.vers){
    case 1:
        pHeaderV1 = reinterpret_cast<struct dmsg_header_v1*>(&aDcsHeader);
        nDataType = pHeaderV1->type;
        aHeader.eventNumber = static_cast<decltype (aHeader.eventNumber)>(pHeaderV1->ident);
        aHeader.timestampSeconds = static_cast<decltype (aHeader.timestampSeconds)>(pHeaderV1->sec);
        break;
    default:
        goto returnPoint;
    }

    if(nDataType != m_dataType.value()){
        SetError(DATA_TYPE_MISMATCH_ERROR,"Data type mismatch");
        goto returnPoint;
    }

    if(m_secondHeaderLength){
        more_size = sizeof(more);
        nReturn = zmq_getsockopt (m_pSocket, ZMQ_RCVMORE, &more, &more_size);

        if(!more){
            goto returnPoint;
        }

        nReturn=zmq_recv(this->m_pSocket,m_pBufferForSecondHeader,m_secondHeaderLength,0);
        if(nReturn!=static_cast<int>(m_secondHeaderLength)){
            goto returnPoint;
        }
    }

    pMemory = CreateDataWithOffset2(0);
    pMemory->data = CreatePitzDaqBuffer(m_expectedDataLength);
    more_size = sizeof(more);
    nReturn = zmq_getsockopt (m_pSocket, ZMQ_RCVMORE, &more, &more_size);

    if(!more){
        this->FreeUsedMemory(pMemory);
        pMemory = nullptr;
        goto returnPoint;
    }

    nReturn=zmq_recv(this->m_pSocket,pMemory->data,m_expectedDataLength,0);
    if(nReturn!=static_cast<int>(m_expectedDataLength)){
        this->FreeUsedMemory(pMemory);
        pMemory = nullptr;
        goto returnPoint;
    }


returnPoint:
    if(pMemory){
        pMemory->header = aHeader;
        SetValid();
    }
    else{SetInvalid();}
    return pMemory;

}


bool SingleEntryZmqDoocs::LoadOrValidateData(void* a_pContext)
{
    int nReturn;
    int nType;
    int nPort;
    ::std::string propToSubscribe;
    ::std::string hostName;
    EqCall eqCall;
    EqData dataIn, dataOut;
    EqAdr eqAddr;
    uint32_t singleItemSize;
    DEC_OUT_PD(TypeAndCount)      branchInfo={m_dataType.value(),(m_itemsCountPerEntry)};

    eqAddr.adr(m_doocsUrl.value());
    nReturn = eqCall.get(&eqAddr,&dataIn,&dataOut);

    if(nReturn){
        // we have error
        ::std::string errorString = dataOut.get_string();
        ::std::cerr << errorString << ::std::endl;
        m_isDataLoaded = 0;
        return false;
    }
    branchInfo.itemsCountPerEntry = dataOut.length();
    m_itemsCountPerEntry = (branchInfo.itemsCountPerEntry);

    //eqAddr.adr(m_doocsUrl.value());
    propToSubscribe = eqAddr.property();
    eqAddr.set_property("SPN");
    dataIn.set (1, 0.0f, 0.0f, static_cast<time_t>(0), propToSubscribe.c_str(), 0);

    nReturn=eqCall.set(&eqAddr,&dataIn,&dataOut);
    SetInvalid();
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
        nPort = dataOut.get_int();
    }break;
    case DATA_A_USTR:{
        float f1, f2;
        time_t tm;
        char         *sp;
        dataOut.get_ustr (&nPort, &f1, &f2, &tm, &sp, 0);
        if(this->m_dataType.value() != static_cast<decltype (branchInfo.type)>(f1) ){
        //    DEBUG_APP_INFO(0,"Data type for entry %s is changed from %d to %d",
        //                   daqName(),static_cast<int>(this->m_branchInfo.dataType),static_cast<int>(f1));
            this->m_dataType.set( (branchInfo.type = static_cast<decltype (branchInfo.type)>(f1)) );
        }
    }break;
    default:
        return false;
    }  // switch(nType){

    nReturn=eqCall.get_option(&eqAddr,&dataIn,&dataOut,EQ_HOSTNAME);
    if(nReturn<0){
        return false;
    }

    hostName = dataOut.get_string();

    m_zmqEndpoint.setValue( ::std::string("tcp://") + hostName + ":" + ::std::to_string(nPort) );
    this->m_pSocket = zmq_socket(a_pContext,ZMQ_SUB);
    if(!this->m_pSocket){
        return false;
    }
    nReturn = zmq_setsockopt (this->m_pSocket, ZMQ_SUBSCRIBE,nullptr, 0);
    if(nReturn){
        return false;
    }

    nReturn = zmq_connect (this->m_pSocket, m_zmqEndpoint.value().c_str());
    if(nReturn){
        return false;
    }

    if(!PrepareDaqEntryBasedOnType2(0,branchInfo.type,NEWNULLPTR,&branchInfo,&singleItemSize,&m_secondHeaderLength,NEWNULLPTR2,NEWNULLPTR2)){return false;}

    m_expectedDataLength = singleItemSize*static_cast<uint32_t>(branchInfo.itemsCountPerEntry);
    SetValid();
    m_isDataLoaded = 1;

    return true;
}


