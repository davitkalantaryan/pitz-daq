/*
 *  Copyright (C) DESY
 *
 *  file:            main_event_based_collector_test.cpp
 *  created on:      2019 Mar 22
 *  created by:      D. Kalantaryan <davit.kalantaryan@desy.de>
 *
 */

/**
 *
 * @file       test/main_event_based_collector_test.cpp
 * @copyright  DESY
 * @brief      sourse for testing event based
 * @author     D. Kalantaryan <davit.kalantaryan@desy.de>
 * @date       2019 Mar 22
 * @details
 *             File demonstrates how event based DAQ collector works
 *
 */

//#define USE_ZMQ_MESSAGES

#include <eq_dmsg.h>
#include <zmq/zmq.h>
#include <thread>
#include <chrono>
#include <doocs/eq_client.h>
#include <time.h>
#include <string.h>
#include <string>
#include <iostream>
#include <list>
#include <vector>
#include <event_based_common_header.h>
#include <stdint.h>
#include <time.h>
#include <cpp11+/shared_mutex_cpp14.hpp>

#ifdef BUILDING_32_BIT
#define UNEXPECTED_READ_SIZE    0xffffffff
#else
#define UNEXPECTED_READ_SIZE    0xffffffffffffffff
#endif

#ifndef HANDLE_LOW_MEMORY
#define HANDLE_LOW_MEMORY(_memory) do{if(!(_memory)){exit(1);}}while(0)
#endif

#ifndef PITZ_DAQ_UNSPECIFIED_NUMBER_OF_SAMPLES
#define PITZ_DAQ_UNSPECIFIED_NUMBER_OF_SAMPLES -1
#endif

#ifndef PITZ_DAQ_UNSPECIFIED_DATA_TYPE
#define PITZ_DAQ_UNSPECIFIED_DATA_TYPE -1
#endif

//#define CENTRAL_TIMING_DETAILS  "tcp://mtcapitzcpu4:5566"

static void TineThreadFunction(void);
static void AnyThreadFunction(void);

static volatile int s_nWork2 = 1;
static void* s_pContext = nullptr;

static const char* s_vcpcDoocsAddress2[] = {
    "PITZ.RF/SIS8300DMA/RF5_DMA.ADC0/CH00.ZMQ"
    //,"PITZ.CHECK/PICUS2/TEST_ZMQ_PUB/FNUM"
    //,"PITZ.CHECK/PICUS2/TEST_ZMQ_PUB/INUM"
    //,"PITZ.CHECK/PICUS2/TEST_ZMQ_PUB/SPECT"
};

static const int s_cnNumberOfDoocsEntries2 = sizeof(s_vcpcDoocsAddress2) / sizeof(const char*) ;

class DaqCollectorDZ;

class SingleEntryZmqDoocs
{
public:
    friend class DaqCollectorDZ;
    //using SingleEntryDoocs::SingleEntryDoocs;
    SingleEntryZmqDoocs();
    ~SingleEntryZmqDoocs();

    int zmqPort()const{return m_nPort;}
    const ::std::string& host()const{return m_hostName;}
    const ::std::string& doocsAddress()const;
    void* socket()const;

    void SetDoocsName(const char* doocsProperty);
    bool LoadOrValidateData();
    void Disconnect();
    int ReadData();
    bool GetExpectedSizesAndCreateBuffers();

private:
    ::std::string   m_doocsAddress;
    ::std::string   m_hostName;
    void*           m_pSocket;
    size_t          m_secondHeaderLength;
    size_t          m_expectedDataLength;
    size_t          m_bufferForSecondHeaderSize;
    size_t          m_bufferForDataSize;
    int             m_nCount;
    int             m_nKnownDataType;
    int             m_nKnownCount;
    int             m_nPort;
    uint64_t        m_isValid : 1;
    uint64_t        m_isDataLoaded : 1;
    uint64_t        m_reserved : 62;
    char            *m_pBufferForSecondHeader,*m_pBufferForData2;
};

class DaqCollectorDZ
{
public:
    void AddNewEntryThreadSafe(const char* doocsAddress, int type, int numberOfSamples);

public:
    void init(void);
    void cancel(void);

private:
    void AddNewEntryPrivate(const char* doocsAddress, int type, int numberOfSamples);
    void ThreadFunction();

private:
    ::STDN::shared_mutex    m_rw_lock;
    ::std::thread   m_threadDoocsZmq;
    volatile int    m_nWork;
    ::std::list< SingleEntryZmqDoocs* > m_listAll;
};

int main()
{
    DaqCollectorDZ aCollector;
    ::std::thread threadTine(TineThreadFunction);
    ::std::thread threadAny(AnyThreadFunction);

    //getchar();
    aCollector.init();

    while(s_nWork2){
        ::std::this_thread::sleep_for( ::std::chrono::hours(1) );
    }

    aCollector.cancel();
    threadAny.join();
    threadTine.join();

    return 0;
}


SingleEntryZmqDoocs::SingleEntryZmqDoocs()
{
    m_doocsAddress = "";
    m_hostName = "";
    m_pSocket = nullptr;
    m_secondHeaderLength = 0;
    m_expectedDataLength = 0;
    m_bufferForSecondHeaderSize=m_bufferForDataSize=0;
    m_nCount = PITZ_DAQ_UNSPECIFIED_NUMBER_OF_SAMPLES;
    m_nKnownDataType = PITZ_DAQ_UNSPECIFIED_DATA_TYPE;
    m_nKnownCount = PITZ_DAQ_UNSPECIFIED_NUMBER_OF_SAMPLES;
    m_nPort = PITZ_DAQ_UNKNOWN_ZMQ_PORT;
    m_isValid = 0;
    m_isDataLoaded = 0;
    m_reserved = 0;
    m_pBufferForData2 = m_pBufferForSecondHeader = nullptr;
}


SingleEntryZmqDoocs::~SingleEntryZmqDoocs()
{
    //
}


void* SingleEntryZmqDoocs::socket()const
{
    return m_pSocket;
}


void SingleEntryZmqDoocs::SetDoocsName(const char* a_doocsProperty)
{
    m_doocsAddress = a_doocsProperty;
}

bool SingleEntryZmqDoocs::LoadOrValidateData()
{
    int nReturn;
    int nSamplesFromServer;
    int nDataTypeFromServer;
    EqCall eqCall;
    EqData dataIn, dataOut;
    EqAdr eqAddr;

    eqAddr.adr(m_doocsAddress);
    nReturn = eqCall.get(&eqAddr,&dataIn,&dataOut);

    if(nReturn){
        return false;
    }

    nDataTypeFromServer = dataOut.type();

    if((nDataTypeFromServer!=m_nKnownDataType)&&(m_nKnownDataType!=PITZ_DAQ_UNSPECIFIED_DATA_TYPE)){
        // todo: error reporting
        return false;
    }
    m_nKnownDataType = nDataTypeFromServer;

    nSamplesFromServer = dataOut.length();
    if((nSamplesFromServer!=m_nKnownCount)&&(m_nKnownCount!=PITZ_DAQ_UNSPECIFIED_NUMBER_OF_SAMPLES)){
        // todo: error reporting
        return false;
    }
    m_nKnownCount = nSamplesFromServer;
    if(m_nCount==PITZ_DAQ_UNSPECIFIED_NUMBER_OF_SAMPLES){
        m_nCount = nSamplesFromServer;
    }
    else if(m_nCount>m_nKnownCount){
        // todo: only warn user
        m_nCount = m_nKnownCount;
    }

    if(m_isDataLoaded){
        // todo: make check
        return true;
    }

    int nType;
    ::std::string propToSubscribe;
    ::std::string zmqEndpoint;

    m_isValid = 0;

    eqAddr.adr(m_doocsAddress);
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
        this->m_nKnownDataType = static_cast<int>(f1);
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
    this->m_pSocket = zmq_socket(s_pContext,ZMQ_SUB);
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

    m_isValid = 1;
    m_isDataLoaded = 1;

    return true;
}


int SingleEntryZmqDoocs::ReadData()
{
    int nReturn;
    int nDataType;
    dmsg_hdr_t aDcsHeader;
    struct dmsg_header_v1* pHeaderV1;

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
        return PITZ_DAQ_EV_BASED_DCS_ZMQ_SYNTAX_ERROR;
    }

    if(nReturn != aDcsHeader.size){
        // this is not header give chance for header
        return PITZ_DAQ_EV_BASED_DCS_ZMQ_UNKNOWN_HEADER;
    }

    switch(aDcsHeader.vers){
    case 1:
        pHeaderV1 = reinterpret_cast<struct dmsg_header_v1*>(&aDcsHeader);
        nDataType = pHeaderV1->type;
        break;
    default:
        return PITZ_DAQ_EV_BASED_DCS_ZMQ_UNKNOWN_HEADER;
    }

    if(nDataType != m_nKnownDataType){
        return PITZ_DAQ_DATA_TYPE_MISMATCH;
    }

    if(m_secondHeaderLength){
        more_size = sizeof(more);
        nReturn = zmq_getsockopt (m_pSocket, ZMQ_RCVMORE, &more, &more_size);

        if(!more){
            return -1;
        }

        nReturn=zmq_recv(this->m_pSocket,m_pBufferForData2,m_secondHeaderLength,0);
        if(nReturn!=static_cast<int>(m_secondHeaderLength)){
            return -1;
        }
    }

    if(m_expectedDataLength){
        more_size = sizeof(more);
        nReturn = zmq_getsockopt (m_pSocket, ZMQ_RCVMORE, &more, &more_size);

        if(!more){
            return -1;
        }

        nReturn=zmq_recv(this->m_pSocket,m_pBufferForData2,m_expectedDataLength,0);
        if(nReturn!=static_cast<int>(m_expectedDataLength)){
            return -1;
        }
    }

    m_isValid = 1;

    return 0;
}


/*/////////////////////////////////////////////////////////////////////////////////////////////////////////*/

bool SingleEntryZmqDoocs::GetExpectedSizesAndCreateBuffers()
{
    //m_expectedRead1=0;
    //m_expectedRead2=0;
    //size_t expectedRead1(0), expectedRead2(0);
    size_t bufferForSecondHeaderSize=0,bufferForDataSize=0;

    //EqDataBlock* db;
    //char* dp;

    switch (m_nKnownDataType) {

    case DATA_INT:
        bufferForDataSize = static_cast<size_t>(m_nCount) * sizeof(int);
        break;

    case DATA_FLOAT:
        bufferForDataSize = static_cast<size_t>(m_nCount) * sizeof(float);
        break;

    case DATA_DOUBLE:
        //memcpy(&db->data_u.DataUnion_u.d_double, dp, sizeof(db->data_u.DataUnion_u.d_double));
        break;

    case DATA_IIII:
        //memcpy(&db->data_u.DataUnion_u.d_iiii, dp, sizeof(db->data_u.DataUnion_u.d_iiii));
        break;

    case DATA_IFFF:
        //memcpy(&db->data_u.DataUnion_u.d_ifff, dp, sizeof(db->data_u.DataUnion_u.d_ifff));
        break;

    case DATA_TTII:
        //memcpy(&db->data_u.DataUnion_u.d_ttii, dp, sizeof(db->data_u.DataUnion_u.d_ttii));
        break;

    case DATA_XYZS:
        //memcpy(&db->data_u.DataUnion_u.d_xyzs, dp, sizeof(db->data_u.DataUnion_u.d_xyzs));
        //db->data_u.DataUnion_u.d_xyzs.loc.loc_val = dp + sizeof(XYZS);
        break;

    case DATA_SPECTRUM:
        //memcpy(&db->data_u.DataUnion_u.d_spectrum, dp, sizeof(db->data_u.DataUnion_u.d_spectrum));
        //db->data_u.DataUnion_u.d_spectrum.comment.comment_val = dp + sizeof(SPECTRUM);
        //db->data_u.DataUnion_u.d_spectrum.d_spect_array.d_spect_array_val = (float*)(dp + sizeof(SPECTRUM) + STRING_LENGTH);
        //break;

        bufferForSecondHeaderSize = sizeof(struct SPECTRUM) + STRING_LENGTH ;
        bufferForDataSize = static_cast<size_t>(m_nKnownCount) * sizeof(float);  // in order to keep socket clean
        break;

    case DATA_GSPECTRUM:
        //memcpy(&db->data_u.DataUnion_u.d_gspectrum, dp, sizeof(db->data_u.DataUnion_u.d_gspectrum));
        //db->data_u.DataUnion_u.d_gspectrum.comment.comment_val = dp + sizeof(GSPECTRUM);
        //db->data_u.DataUnion_u.d_gspectrum.d_gspect_array.d_gspect_array_val = (float*)(dp + sizeof(GSPECTRUM) + STRING_LENGTH);
        break;

    case DATA_A_SHORT:
        //db->data_u.DataUnion_u.d_short_array.d_short_array_val = (short*)dp;
        //db->data_u.DataUnion_u.d_short_array.d_short_array_len = len;
        bufferForDataSize = static_cast<size_t>(m_nCount) * sizeof(short);
        break;

    case DATA_A_INT:
        //db->data_u.DataUnion_u.d_int_array.d_int_array_val = (int*)dp;
        //db->data_u.DataUnion_u.d_int_array.d_int_array_len = len;
        break;

    case DATA_A_LONG:
        //db->data_u.DataUnion_u.d_llong_array.d_llong_array_val = (long long*)dp;
        //db->data_u.DataUnion_u.d_llong_array.d_llong_array_len = len;
        //break;

    case DATA_A_FLOAT:
        //db->data_u.DataUnion_u.d_float_array.d_float_array_val = (float*)dp;
        //db->data_u.DataUnion_u.d_float_array.d_float_array_len = len;
        break;

    case DATA_A_DOUBLE:
        //db->data_u.DataUnion_u.d_double_array.d_double_array_val = (double*)dp;
        //db->data_u.DataUnion_u.d_double_array.d_double_array_len = len;
        break;

    case DATA_STRING:
    case DATA_TEXT:
        //db->data_u.DataUnion_u.d_char.d_char_val = dp;
        //db->data_u.DataUnion_u.d_char.d_char_len = strlen(dp);
        break;

    case DATA_A_USTR:
        //db->data_u.DataUnion_u.d_ustr_array.d_ustr_array_val = (USTR*)dp;
        //db->data_u.DataUnion_u.d_ustr_array.d_ustr_array_len = len;
        //usp = (USTR*)dp;
        //usp->str_data.str_data_val = dp + sizeof(USTR);
        break;

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
    } break;

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
    } break;

    default:
        return false;
    }

    if(bufferForSecondHeaderSize>m_bufferForSecondHeaderSize){
        char* pBufferTmp = static_cast<char*>(realloc(m_pBufferForSecondHeader,bufferForSecondHeaderSize));
        if(pBufferTmp){
            m_pBufferForSecondHeader = pBufferTmp;
            m_secondHeaderLength=m_bufferForSecondHeaderSize = bufferForSecondHeaderSize;
        }
    }
    else{
        m_secondHeaderLength = bufferForSecondHeaderSize;
    }

    if(bufferForDataSize>m_bufferForDataSize){
        char* pBufferTmp = static_cast<char*>(realloc(m_pBufferForData2,bufferForDataSize));
        if(pBufferTmp){
            m_pBufferForData2 = pBufferTmp;
            m_expectedDataLength=m_bufferForDataSize = bufferForDataSize;
        }
    }
    else{
        m_expectedDataLength = bufferForDataSize;
    }

    return true;
}

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////*/


void DaqCollectorDZ::AddNewEntryPrivate(const char* a_doocsAddress, int a_type, int a_numberOfSamples)
{
    SingleEntryZmqDoocs* pEntry = new SingleEntryZmqDoocs;

    pEntry->SetDoocsName(a_doocsAddress);
    pEntry->m_nKnownDataType = a_type;
    pEntry->m_nCount = a_numberOfSamples;

    m_listAll.push_back(pEntry);
}


void DaqCollectorDZ::cancel(void)
{
    m_nWork = 0;
    m_threadDoocsZmq.join();
}


void DaqCollectorDZ::init(void)
{
    int i;

    m_rw_lock.lock();

    for(i=0;i<s_cnNumberOfDoocsEntries2;++i){
        AddNewEntryPrivate(s_vcpcDoocsAddress2[i],-1,-1);
    }

    m_rw_lock.unlock();

    m_nWork = 1;
    m_threadDoocsZmq = ::std::thread (&DaqCollectorDZ::ThreadFunction,this);

}


void DaqCollectorDZ::ThreadFunction(void)
{
    zmq_pollitem_t *pItemsTmp, *pItems = nullptr;
    ::std::list< SingleEntryZmqDoocs* > validList;
    time_t  lastUpdateTime = 0, currentTime;
    int nNumbeOfEntries=0, nAllocatedSize=0;
    int i;
    int nReturn, nPrinted, nPrintedIndex;
    int nIteration = 0;

    s_pContext = zmq_ctx_new();

    printf("Iteration: ");
    nPrinted = printf("%d",0);
    fflush(stdout);

    while(m_nWork){

        time(&currentTime);

        if(currentTime-lastUpdateTime>PITZ_DAQ_LIST_UPDATE_DEFAULT_TIME){
            validList.clear();
            m_rw_lock.lock_shared();
            for(auto pEntry : m_listAll){
                if(pEntry->LoadOrValidateData()){
                    validList.push_back(pEntry);
                }
            }
            m_rw_lock.unlock_shared();
            nNumbeOfEntries = static_cast<int>(validList.size());
            if(nNumbeOfEntries>nAllocatedSize){
                pItemsTmp = static_cast<zmq_pollitem_t*>(realloc(pItems,sizeof(zmq_pollitem_t)*static_cast<size_t>(nNumbeOfEntries)));
                HANDLE_LOW_MEMORY(pItemsTmp);
                pItems = pItemsTmp;
                nAllocatedSize = nNumbeOfEntries;
            }
            lastUpdateTime = currentTime;

        }

        if(!nNumbeOfEntries){
            ::std::this_thread::sleep_for( ::std::chrono::seconds(1) );
            continue;
        }

        i = 0;
        for(auto pEntry : validList){
            pItems[i].revents = 0;
            pItems[i].socket = pEntry->socket();
            pItems[i].events = ZMQ_POLLIN;
            ++i;
        }

        nReturn=zmq_poll(pItems,nNumbeOfEntries,-1);

        if(nReturn<=0){
            continue;
        }

        i = 0;
        for(auto pEntry : validList){
            if(pItems[i].revents & ZMQ_POLLIN){

                ++nIteration;
                for(nPrintedIndex=0;nPrintedIndex<nPrinted;++nPrintedIndex){
                    printf("\b");
                }
                nPrinted = printf("%d",nIteration);
                fflush(stdout);

                // read: pItems[0].socket
                if(pEntry->ReadData()<0){
                    fprintf(stderr,"Unable to read !!!!!\n");
                }
            }
            ++i;
        }
    }

    zmq_ctx_destroy(s_pContext);

}


// /doocs/develop/kalantar/programs/matlab/ccppfiles/mexprojects/tine_subscriber
static void TineThreadFunction(void)
{
    //
}


/**
 * @brief
 *  function that demonstrates how any socket (ZMQ included) implementation will work.
 * @details
 *  For simplicity error codes on options setting ignored
 * @cite
 *  http://api.zeromq.org/2-1:zmq-poll
 *   - All ØMQ sockets passed to the zmq_poll() function must share the same ØMQ context and must belong to the thread calling zmq_poll().
 */
#if 0
void *socket;
#if defined _WIN32
SOCKET fd;
#else
int fd;
#endif
short events;
short revents;
#endif
static void AnyThreadFunction(void)
{
    void* pContext = nullptr;
    void* pZmqSocket = nullptr;
    //zmq_pollitem_t vPollItems[1024];
    //int nitems = 0;

    pContext = zmq_ctx_new();
    if(!pContext){goto returnPoint;}

    pZmqSocket = zmq_socket (pContext, ZMQ_SUB);
    if(!pZmqSocket){goto returnPoint;}

    //vPollItems[0].socket = pZmqSocket;
    //vPollItems[0].fd = -1; // commented because socket is tried when non null


    zmq_setsockopt (pZmqSocket,ZMQ_SUBSCRIBE, nullptr,0);
    //zmq_connect (pZmqSocket, CENTRAL_TIMING_DETAILS);


returnPoint:
    if(pContext){zmq_ctx_destroy(pContext);}
}
