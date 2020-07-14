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
#include <iostream>
#include "pitz_daq_singleentry.cpp.hpp"
#include <pitz_daq_data_daqdev_common.h>

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
	DEC_OUT_PD(Header)* dataFromNetwork;
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
		if(!pEntry->isDataLoaded()){
            pEntry->LoadOrValidateData(pNetZmq->m_pContext);
        }
		if(pEntry->isDataLoaded()){
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
                pEntry->IncrementError(NETWORK_READ_ERROR, "Unable to get data via zmq");
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
		HANDLE_LOW_MEMORY(pItemsTmp,"Low memory");
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
	m_bitwise64Reserved = 0;
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


void pitz::daq::SingleEntryZmqDoocs::FreeUsedMemory(DEC_OUT_PD(Header)* a_usedMemory)
{
	FreePitzDaqBuffer(a_usedMemory);
}


void* SingleEntryZmqDoocs::socket()const
{
    return m_pSocket;
}


DEC_OUT_PD(Header)* SingleEntryZmqDoocs::ReadData()
{
    int nReturn;
	int32_t nDataType;
    dmsg_hdr_t aDcsHeader;
	DEC_OUT_PD(Header)* pMemory=nullptr;
    size_t     more_size;
    int        more;
    DEC_OUT_PD(Header) aHeader;

	if(m_samples != (m_nNextDataMaxSamples)){
		m_samples = (m_nNextDataMaxSamples);
	}

    nReturn=zmq_recv(this->m_pSocket,&aDcsHeader,sizeof(dmsg_hdr_t),0);
    if(nReturn<4){
		goto errorReturn;
    }

    switch(aDcsHeader.vers){
    case 1:{
        struct dmsg_header_v1* pHeaderV1 = reinterpret_cast<struct dmsg_header_v1*>(&aDcsHeader);
        short exthd = static_cast<short>(DMSG_HDR_EXT);
        uint64_t ullnSec = static_cast<uint64_t>(pHeaderV1->sec) & 0x0fffffffful;
        if (pHeaderV1->size & exthd) {
            // extended header with 64 bit seconds
            // sechi contains seconds' high 32 bit word
			ullnSec |= static_cast<uint64_t>( static_cast<uint64_t>(pHeaderV1->sechi) & 0x0fffffffful) << 32;
			pHeaderV1->size &= ~exthd;
        }
		if(nReturn != pHeaderV1->size){
			// this is not header give chance for header
			goto errorReturn;
		}
        nDataType = static_cast<decltype (nDataType)>(pHeaderV1->type);
		aHeader.gen_event = static_cast<decltype (aHeader.gen_event)>(pHeaderV1->ident);
		aHeader.seconds = static_cast<decltype (aHeader.seconds)>(ullnSec);
    }break;
    default:
		goto errorReturn;
    }

    if(nDataType != m_dataType.value()){
        IncrementError(DATA_TYPE_MISMATCH_ERROR,"Data type mismatch");
		goto errorReturn;
    }

    if(m_secondHeaderLength){
        more_size = sizeof(more);
        nReturn = zmq_getsockopt (m_pSocket, ZMQ_RCVMORE, &more, &more_size);

        if(!more){
			goto errorReturn;
        }

        nReturn=zmq_recv(this->m_pSocket,m_pBufferForSecondHeader,m_secondHeaderLength,0);
        if(nReturn!=static_cast<int>(m_secondHeaderLength)){
			goto errorReturn;
        }
    }

	pMemory = CreatePitzDaqSingleDataHeader(m_expectedDataLength);
    more_size = sizeof(more);
    nReturn = zmq_getsockopt (m_pSocket, ZMQ_RCVMORE, &more, &more_size);

    if(!more){
		goto errorReturn;
    }

	nReturn=zmq_recv(this->m_pSocket,wrPitzDaqDataFromHeader(pMemory),m_expectedDataLength,0);

	if(nReturn<1){
		goto errorReturn;
	}
	else if(nReturn < static_cast<int>(m_expectedDataLength)){
		m_expectedDataLength = static_cast< decltype (m_expectedDataLength) >(nReturn);
		m_nNextDataMaxSamples = nReturn/m_nSingleItemSize;
		m_samples = (m_nNextDataMaxSamples);
	}
	else if(nReturn > static_cast<int>(m_expectedDataLength)) {
		m_expectedDataLength = static_cast< decltype (m_expectedDataLength) >(nReturn);
		m_nNextDataMaxSamples = nReturn/m_nSingleItemSize;
	}

	*pMemory = aHeader;

    return pMemory;

errorReturn:
	this->m_isDataLoaded = 0;
	if(pMemory){this->FreeUsedMemory(pMemory);}
	return nullptr;

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
	struct PrepareDaqEntryInputs in;
	struct PrepareDaqEntryOutputs out;
	//DEC_OUT_PD(TypeAndCount)      branchInfo={static_cast<int32_t>(m_dataType.value()),(m_itemsCountPerEntry2)};

	memset(&in,0,sizeof(in));
	memset(&out,0,sizeof(out));

    eqAddr.adr(m_doocsUrl.value());
    nReturn = eqCall.get(&eqAddr,&dataIn,&dataOut);

    if(nReturn){
        // we have error
        ::std::string errorString = dataOut.get_string();
        ::std::cerr << errorString << ::std::endl;
        m_isDataLoaded = 0;
        return false;
    }
	out.inOutSamples = dataOut.length();
	//m_samples = (out.inOutSamples);

    //eqAddr.adr(m_doocsUrl.value());
    propToSubscribe = eqAddr.property();
    eqAddr.set_property("SPN");
    dataIn.set (1, 0.0f, 0.0f, static_cast<time_t>(0), propToSubscribe.c_str(), 0);

    nReturn=eqCall.set(&eqAddr,&dataIn,&dataOut);
    if(nReturn){
        // we have error
        ::std::string errorString = dataOut.get_string();
        ::std::cerr << errorString << ::std::endl;
        m_isDataLoaded = 0;
        return false;
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
		in.dataType = static_cast<int32_t>(f1);
		if(this->m_dataType.value() != in.dataType ){
			if(this->m_dataType.value()>0){
				return false;
			}
			this->m_dataType.set(in.dataType);
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

	nReturn = zmq_connect (this->m_pSocket, m_zmqEndpoint.value());
    if(nReturn){
        return false;
    }

	if(!PrepareDaqEntryBasedOnType(&in,&out)){return false;}

	m_nSingleItemSize = static_cast<int>(out.oneItemSize);
	m_nNextDataMaxSamples = m_samples = (out.inOutSamples);
	m_expectedDataLength = out.oneItemSize*static_cast<uint32_t>(out.inOutSamples);
    m_isDataLoaded = 1;

    return true;
}


void SingleEntryZmqDoocs::InitializeRootTree()
{
	if(!isDataLoaded()){
		const SNetworkStructZmqDoocs* pNetZmq = static_cast< const SNetworkStructZmqDoocs* >( networkParent() );
		LoadOrValidateData(pNetZmq->m_pContext);
	}

	if(!CheckBranchExistanceAndCreateIfNecessary()){return;}

	if(m_expectedDataLength>0){
		DEC_OUT_PD(Header)* pDataToFill;
		int64_t timeSeconds, macroPulse;
		EqData dataIn, dataOut;
		EqAdr doocsAdr;
		EqCall doocsCall;
		int nError;
		bool bDeleteData;

		doocsAdr.adr(m_doocsUrl.value());
		if( (nError=doocsCall.get(&doocsAdr,&dataIn,&dataOut)) ){
			printftostderr(__FUNCTION__,nError,"DOOCS error (code:%d,str:\"%s\")",nError,dataOut.get_string().c_str());
			return;
		}

		pDataToFill = GetDataPointerFromEqData(m_expectedDataLength,&dataOut,&timeSeconds,&macroPulse,&bDeleteData);
		if(pDataToFill){
			pDataToFill->gen_event = static_cast< decltype (pDataToFill->gen_event) >(macroPulse);
			pDataToFill->seconds = static_cast< decltype (pDataToFill->seconds) >(timeSeconds);
			if(pDataToFill->gen_event<1){
				pDataToFill->gen_event = static_cast< decltype (pDataToFill->gen_event) >(GetEventNumberFromTime(timeSeconds));
			}
			this->FillRaw(pDataToFill);
			if(bDeleteData){
				FreePitzDaqBuffer(pDataToFill);
			}
		}
	}
}


void SingleEntryZmqDoocs::FinalizeRootTree()
{
	// if we have non correspondend root format string then not possible to save last data
	if(m_expectedDataLength>0){
		DEC_OUT_PD(Header)* pDataToFill;
		int64_t timeSeconds, macroPulse;
		EqData dataIn, dataOut;
		EqAdr doocsAdr;
		EqCall doocsCall;
		int nError;
		bool bDeleteData;

		doocsAdr.adr(m_doocsUrl.value());
		if( (nError=doocsCall.get(&doocsAdr,&dataIn,&dataOut)) ){
			printftostderr(__FUNCTION__,nError,"DOOCS error (code:%d,str:\"%s\")",nError,dataOut.get_string().c_str());
			return;
		}

		pDataToFill = GetDataPointerFromEqData(m_expectedDataLength,&dataOut,&timeSeconds,&macroPulse,&bDeleteData);
		if(pDataToFill){
			pDataToFill->gen_event = static_cast< decltype (pDataToFill->gen_event) >(macroPulse);
			pDataToFill->seconds = static_cast< decltype (pDataToFill->seconds) >(timeSeconds);
			if(pDataToFill->gen_event<1){
				pDataToFill->gen_event = static_cast< decltype (pDataToFill->gen_event) >(GetEventNumberFromTime(timeSeconds));
			}
			this->FillRaw(pDataToFill);
			if(bDeleteData){
				FreePitzDaqBuffer(pDataToFill);
			}
		}
	}
}
