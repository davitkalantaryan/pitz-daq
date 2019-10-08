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
    SingleEntryZmqDoocs* pEntry = new SingleEntryZmqDoocs(a_creationType,a_entryLine);

    if(!pEntry){return pEntry;}

    return pEntry;
}


void pitz::daq::EqFctEventBased::DataGetterThread(SNetworkStruct* a_pNet)
{
    SNetworkStructZmqDoocs* pNetZmq = static_cast< SNetworkStructZmqDoocs* >(a_pNet);
    SingleEntryZmqDoocs* pEntryCheck;
    zmq_pollitem_t *pItemsTmp;
    //::std::list< SingleEntryZmqDoocs* > validList; // instead of this hash table should be used
    time_t  lastUpdateTime = 0, currentTime;
    size_t unNumbeOfEntries(0);
    int i;
    int nReturn;
    const ::std::list< SingleEntry* >& entriesList = a_pNet->daqEntries();
    ::std::list< SingleEntry* >::const_iterator pIter, pIterEnd;

    while(shouldWork() && a_pNet->shouldRun() ){

        time(&currentTime);

        if(currentTime-lastUpdateTime>PITZ_DAQ_LIST_UPDATE_DEFAULT_TIME){
            validList.clear();
            //m_mutexForEntries.lock_shared();
            if(entriesList.size()<1){/*m_mutexForEntries.unlock_shared();*/return;}

            validList.clear();

            this->ReadLockEntries2();

            pIterEnd = entriesList.end();
            for(pIter=entriesList.begin();pIter!=pIterEnd;++pIter){
                pEntryCheck = static_cast< SingleEntryZmqDoocs* >( *pIter );
                if(pEntryCheck->LoadOrValidateData(pNetZmq->m_pContext)){
                    validList.push_back(pEntryCheck);
                }
            }

            this->ReadUnlockEntries2();

            unNumbeOfEntries = validList.size();
            if(unNumbeOfEntries<1){
                return;
            }

            if(unNumbeOfEntries>pNetZmq->m_unCreatedItemsCount){
                pItemsTmp = static_cast<zmq_pollitem_t*>(realloc(pNetZmq->m_pItems,sizeof(zmq_pollitem_t)*unNumbeOfEntries));
                HANDLE_LOW_MEMORY(pItemsTmp);
                pNetZmq->m_pItems = pItemsTmp;
                pNetZmq->m_unCreatedItemsCount = unNumbeOfEntries;
            }
            lastUpdateTime = currentTime;

        }

        if(unNumbeOfEntries<1){
            ::std::this_thread::sleep_for( ::std::chrono::seconds(1) );
            continue;
        }

        i = 0;
        for(auto pEntry : validList){
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
        for(auto pEntry : validList){
            if(pNetZmq->m_pItems[i].revents & ZMQ_POLLIN){
                //pEntry->RemoveDoocsProperty();
                pEntry->ReadData();
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

/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
pitz::daq::SingleEntryZmqDoocs::SingleEntryZmqDoocs(entryCreationType::Type a_creationType,const char* a_entryLine)
    :
      SingleEntryDoocs(a_creationType,a_entryLine)
{
    m_hostName = "";
    m_pSocket = NEWNULLPTR;
    m_expectedReadHeader2 = 0;
    //m_nKnownDataType = PITZ_DAQ_UNSPECIFIED_DATA_TYPE;
    m_nPort = PITZ_DAQ_UNKNOWN_ZMQ_PORT;
    //m_nReserved = 0;
    m_isValid = 0;
    m_isDataLoaded = 0;
    m_reserved = 0;
    m_pBufferForHeader2 = NEWNULLPTR;
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

    if(m_bufferSize>0){
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

        nReturn=zmq_recv(this->m_pSocket,wrPitzDaqDataFromEntry(pMemory),m_bufferSize,0);
        if(nReturn!=static_cast<int>(m_bufferSize)){
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
    int nSamplesFromServer;
    int nDataTypeFromServer;
    EqCall eqCall;
    EqData dataIn, dataOut;
    EqAdr eqAddr;

    eqAddr.adr(m_doocsUrl);
    nReturn = eqCall.get(&eqAddr,&dataIn,&dataOut);

    if(nReturn){
        return false;
    }

    nDataTypeFromServer = dataOut.type();

    if((nDataTypeFromServer!=m_branchInfo.dataType)&&(m_branchInfo.dataType!=PITZ_DAQ_UNSPECIFIED_DATA_TYPE)){
        // todo: error reporting
        return false;
    }
    m_branchInfo.dataType = nDataTypeFromServer;

    nSamplesFromServer = dataOut.length();
    if((nSamplesFromServer!=m_branchInfo.itemsCountPerEntry)&&(m_branchInfo.itemsCountPerEntry!=PITZ_DAQ_UNSPECIFIED_NUMBER_OF_SAMPLES)){
        // todo: error reporting
        return false;
    }
    m_branchInfo.itemsCountPerEntry = nSamplesFromServer;
    if((m_branchInfo.itemsCountPerEntry==PITZ_DAQ_UNSPECIFIED_NUMBER_OF_SAMPLES)||(m_branchInfo.itemsCountPerEntry==0)){
        m_branchInfo.itemsCountPerEntry = nSamplesFromServer;
    }
    //else if(m_nSamples>m_nKnownSamples){
    //    // todo: only warn user
    //    m_nSamples = m_nKnownSamples;
    //}

    if(m_isDataLoaded){
        // todo: make check
        return true;
    }

    int nType;
    ::std::string propToSubscribe;
    ::std::string zmqEndpoint;

    m_isValid = 0;

    eqAddr.adr(m_doocsUrl);
    propToSubscribe = eqAddr.property();
    eqAddr.set_property("SPN");
    dataIn.set (1, 0.0f, 0.0f, static_cast<time_t>(0), propToSubscribe.c_str(), 0);

    nReturn=eqCall.set(&eqAddr,&dataIn,&dataOut);
    if(nReturn){
        // we have error
        ::std::string errorString = dataOut.get_string();
        ::std::cerr << errorString << ::std::endl;
        return false;
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
        this->m_branchInfo.dataType = static_cast<int>(f1);
    }break;
    default:
        return false;
    }  // switch(nType){

    //eqAddr.set_property(propToSubscribe);
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

    if( !GetExpectedSizesAndCreateBuffers() ){
        return false;
    }

    this->SetEntryInfo(m_unOffset,m_branchInfo);
    m_isValid = 1;
    m_isDataLoaded = 1;

    return true;
}


bool SingleEntryZmqDoocs::GetExpectedSizesAndCreateBuffers()
{
    m_expectedReadHeader2=0;
    //m_expectedReadData=0;
    m_bufferSize = 0;

    //EqDataBlock* db;
    //char* dp;

    switch (m_branchInfo.dataType) {

    case DATA_INT:
        m_bufferSize = static_cast<uint32_t>(m_branchInfo.itemsCountPerEntry) * sizeof(int);
        break;

    case DATA_FLOAT:
        m_bufferSize = static_cast<uint32_t>(m_branchInfo.itemsCountPerEntry) * sizeof(float);
        break;

    case DATA_DOUBLE:
        //memcpy(&db->data_u.DataUnion_u.d_double, dp, sizeof(db->data_u.DataUnion_u.d_double));
        //break;

    case DATA_IIII:
        //memcpy(&db->data_u.DataUnion_u.d_iiii, dp, sizeof(db->data_u.DataUnion_u.d_iiii));
        //break;

    case DATA_IFFF:
        //memcpy(&db->data_u.DataUnion_u.d_ifff, dp, sizeof(db->data_u.DataUnion_u.d_ifff));
        //break;

    case DATA_TTII:
        //memcpy(&db->data_u.DataUnion_u.d_ttii, dp, sizeof(db->data_u.DataUnion_u.d_ttii));
        //break;

    case DATA_XYZS:
        //memcpy(&db->data_u.DataUnion_u.d_xyzs, dp, sizeof(db->data_u.DataUnion_u.d_xyzs));
        //db->data_u.DataUnion_u.d_xyzs.loc.loc_val = dp + sizeof(XYZS);
        //break;

    case DATA_SPECTRUM:
        //memcpy(&db->data_u.DataUnion_u.d_spectrum, dp, sizeof(db->data_u.DataUnion_u.d_spectrum));
        //db->data_u.DataUnion_u.d_spectrum.comment.comment_val = dp + sizeof(SPECTRUM);
        //db->data_u.DataUnion_u.d_spectrum.d_spect_array.d_spect_array_val = (float*)(dp + sizeof(SPECTRUM) + STRING_LENGTH);
        //break;

        m_expectedReadHeader2 = sizeof(struct SPECTRUM) + STRING_LENGTH ;
        m_bufferSize = static_cast<uint32_t>(m_branchInfo.itemsCountPerEntry) * sizeof(float);  // in order to keep socket clean
        break;

    case DATA_GSPECTRUM:
        //memcpy(&db->data_u.DataUnion_u.d_gspectrum, dp, sizeof(db->data_u.DataUnion_u.d_gspectrum));
        //db->data_u.DataUnion_u.d_gspectrum.comment.comment_val = dp + sizeof(GSPECTRUM);
        //db->data_u.DataUnion_u.d_gspectrum.d_gspect_array.d_gspect_array_val = (float*)(dp + sizeof(GSPECTRUM) + STRING_LENGTH);
        //break;

    case DATA_A_SHORT:
        //db->data_u.DataUnion_u.d_short_array.d_short_array_val = (short*)dp;
        //db->data_u.DataUnion_u.d_short_array.d_short_array_len = len;
        //break;

    case DATA_A_INT:
        //db->data_u.DataUnion_u.d_int_array.d_int_array_val = (int*)dp;
        //db->data_u.DataUnion_u.d_int_array.d_int_array_len = len;
        //break;

    case DATA_A_LONG:
        //db->data_u.DataUnion_u.d_llong_array.d_llong_array_val = (long long*)dp;
        //db->data_u.DataUnion_u.d_llong_array.d_llong_array_len = len;
        //break;

    case DATA_A_FLOAT:
        //db->data_u.DataUnion_u.d_float_array.d_float_array_val = (float*)dp;
        //db->data_u.DataUnion_u.d_float_array.d_float_array_len = len;
        //break;

    case DATA_A_DOUBLE:
        //db->data_u.DataUnion_u.d_double_array.d_double_array_val = (double*)dp;
        //db->data_u.DataUnion_u.d_double_array.d_double_array_len = len;
        //break;

    case DATA_STRING:
    case DATA_TEXT:
        //db->data_u.DataUnion_u.d_char.d_char_val = dp;
        //db->data_u.DataUnion_u.d_char.d_char_len = strlen(dp);
        //break;

    case DATA_A_USTR:
        //db->data_u.DataUnion_u.d_ustr_array.d_ustr_array_val = (USTR*)dp;
        //db->data_u.DataUnion_u.d_ustr_array.d_ustr_array_len = len;
        //usp = (USTR*)dp;
        //usp->str_data.str_data_val = dp + sizeof(USTR);
        //break;

    case DATA_IMAGE: {
        //char* ptr = dp;
        //
        //
        //memcpy(&db->data_u.DataUnion_u.d_image.hdr, dp, sizeof(db->data_u.DataUnion_u.d_image.hdr));
        //
        //dp += sizeof(db->data_u.DataUnion_u.d_image.hdr);
        //db->data_u.DataUnion_u.d_image.sec = *(time_t*)dp;
        //
        //dp += sizeof(time_t);
        //db->data_u.DataUnion_u.d_image.usec = *(time_t*)dp;
        //
        //dp += sizeof(time_t);
        //db->data_u.DataUnion_u.d_image.status = *(int*)dp;
        //
        //dp += sizeof(int);
        //db->data_u.DataUnion_u.d_image.comment.comment_val = dp;
        //db->data_u.DataUnion_u.d_image.comment.comment_len = strlen(dp) + 1;
        //
        //dp += strlen(dp) + 1;
        //db->data_u.DataUnion_u.d_image.val.val_val = (u_char*)dp;
        //db->data_u.DataUnion_u.d_image.val.val_len = sz - (dp - ptr);
    } /*break*/;

    case DATA_A_BYTE: {
        //int* hdp = (int*)dp;
        //
        //db->data_u.DataUnion_u.d_byte_struct.x_dim = hdp[0];
        //db->data_u.DataUnion_u.d_byte_struct.y_dim = hdp[1];
        //db->data_u.DataUnion_u.d_byte_struct.x_offset = hdp[2];
        //db->data_u.DataUnion_u.d_byte_struct.y_offset = hdp[3];
        //db->data_u.DataUnion_u.d_byte_struct.option = hdp[4];
        //
        //dp += sizeof(int) * 5;
        //
        //db->data_u.DataUnion_u.d_byte_struct.d_byte_array.d_byte_array_len = hdp[0];
        //db->data_u.DataUnion_u.d_byte_struct.d_byte_array.d_byte_array_val = (u_char*)dp;
    } /*break*/;

    default:
        return false;
    }

    if(m_pBufferForHeader2){
        free(m_pBufferForHeader2);
        m_pBufferForHeader2 = nullptr;
    }
    if(m_expectedReadHeader2){
        m_pBufferForHeader2 = static_cast<char*>(malloc(m_expectedReadHeader2));
    }

#if 0


    if(m_pBuffer2){
        free(m_pBuffer2);
        m_pBuffer2 = nullptr;
    }
    if(m_expectedRead2){
        m_pBuffer2 = static_cast<char*>(malloc(m_expectedRead2));
    }
#endif

    return true;
}
