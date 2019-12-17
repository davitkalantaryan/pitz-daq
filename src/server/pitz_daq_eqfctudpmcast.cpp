/*
 * Remaining questions
 *  1) pitzbpmadc_rpc_server.cc, Lns: 1025-1042 (imast@?)
 */

#include "pitz_daq_eqfctudpmcast.hpp"
#include "printtostderr.h"
#include <sys/stat.h>
#include <sys/shm.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <libgen.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "eq_errors.h"
#include "eq_sts_codes.h"
#include "eq_fct_errors.h"
#include "mclistener.hpp"
#include "udpmcastdaq_commonheader.h"
#include "pitz_daq_globalfunctionsiniter.hpp"
#include "common_daq_definations.h"
//#include "alog.h"
#include <TPluginManager.h> // https://root.cern.ch/phpBB3/viewtopic.php?t=9816
#ifdef _TEST_GUI_
#include <simple_plotter_c.h>
#endif

#define SPECIAL_KEY "channel"

static int SwapDataIfNecessary(DEC_OUT_PD(SingleData) *a_pMemory);

namespace pitz{namespace daq{

class SingleEntryUdp : public SingleEntry
{
public:
    SingleEntryUdp(entryCreationType::Type creationType,const char* entryLine, TypeConstCharPtr* a_pHelper);
    ~SingleEntryUdp();

    const char* rootFormatString()const;
    void ValueStringByKeyInherited(bool bReadAll, const char* request, char* buffer, int bufferLength)const;
    void PermanentDataIntoFile(FILE* fpFile)const;

    void SetFctParent(EqFctUdpMcast*  a_pFctParent);
    int channelNumber()const{return m_nChannelNumber;}

    //void ResetPending(int indexOrEventNumber);
    //unsigned int IsAnyPending()const;
    //unsigned int IsPending(int indexOrEventNumber)const;

    DEC_OUT_PD(SingleData)* GetAndRemovePendingBufferIfAny(int indexOrEventNumber);
    void SetPendingBuffer(int a_indexOrEventNumber, DEC_OUT_PD(SingleData)* a_pBuffer);

private:
    EqFctUdpMcast*  m_pFctParent;
    int             m_nChannelNumber;
    int             m_nReserved;
    //unsigned int    m_isPending;
    DEC_OUT_PD(SingleData)* m_vPendings[NUMBER_OF_PENDING_PACKS];

};
}}

using namespace pitz::daq;


EqFct* eq_create(int a_eq_code, void* /*a_arg*/)
{
    ::EqFct* pRet = NEWNULLPTR2;
    //getchar();

    switch (a_eq_code)
    {
    case CODE_UDP_MCAST_DAQ:
        pRet = new pitz::daq::EqFctUdpMcast;
        break;
    default: break;
    }
    return pRet;
}


/*////////////////////////////////////////////////////////////////////*/


pitz::daq::EqFctUdpMcast::EqFctUdpMcast() :
       m_hostName("HOST__NAME for multicasting", this)

{
    memset(m_vMapper,0,sizeof(m_vMapper));
    m_hostName.set_ro_access();
}



pitz::daq::EqFctUdpMcast::~EqFctUdpMcast()
{
}


int pitz::daq::EqFctUdpMcast::fct_code()
{
    return CODE_UDP_MCAST_DAQ;
}


pitz::daq::SingleEntry* pitz::daq::EqFctUdpMcast::CreateNewEntry(entryCreationType::Type a_creationType,const char* a_entryLine)
{
    const char* cpcLine;
    SingleEntryUdp* pEntry = new SingleEntryUdp(a_creationType,a_entryLine,&cpcLine);
    int nChannelNumber ;
    
    if(!pEntry){return NEWNULLPTR2;}
       
    nChannelNumber = pEntry->channelNumber();

    if(m_vMapper[nChannelNumber]){
        delete pEntry;
        return NEWNULLPTR2;
    }
    pEntry->SetFctParent(this);

    return pEntry;
}


void pitz::daq::EqFctUdpMcast::DataGetterThread(SNetworkStruct* /*pNet*/)
{
    SingleEntryUdp* pCurEntry;
    DEC_OUT_PD(SingleData) *pPendingData, *pMemory2 = CreateDataWithOffset(8,sizeof(DATA_struct)-8); // 8 = 4(for endian), 4(for branchNum)
    MClistener      aListener;
    //MClistener
    int nBranchNum;
    int nbytes;
    int GH;
    int nEventNumberToTryPending,nLastEventToTryPending, nError=0;
    int nEventNumber(0), nLastEventNumberHandled;
    bool bErrorNoEntrySet(false);

    if(aListener.ConnectToTheMGroup(m_hostName.value())){
        std::cerr<<"Unable to connect to host\""<<m_hostName.value()<<"\"\n";
        exit(1);
    }
    aListener.SetSocketTimeout(2000);

    while( shouldWork() ){

        nbytes = aListener.recvC(pMemory2,sizeof(DATA_struct));

        //DEBUG_("nbytes=%d\n",nbytes);

        if( nbytes != sizeof(DATA_struct) ){

#ifdef __OLD__
            p->listener->init_socket_new();
#else
            if(shouldWork()){
                if(nError==0){
                    printtostderr("ERROR","nbytes < 0");
                    //ERR_LOG("nbytes<0");
                    fprintf(stderr,"Error during reading\n");
                    nError = 1;
                }
                SleepMs(5000);
            }
            continue;
#endif
        }

        if(nError==1){
            nError = 0;
            printf("Error recovered!\n");
        }
        nBranchNum = SwapDataIfNecessary(pMemory2)%MAX_CHANNELS_NUM;

        pCurEntry = static_cast<SingleEntryUdp*>(m_vMapper[nBranchNum]);
        pCurEntry->lockEntryForRoot();

        nLastEventNumberHandled = pCurEntry->LastEventNumberHandled();
        nEventNumber = pMemory2->eventNumber;

        if(nEventNumber<=nLastEventNumberHandled){
            // report on repetition (this is just warning forgot it :) )
            continue;
        }

        GH = nEventNumber% s_H_count;
        pMemory2->timestampSeconds=::g_shareptr[GH].seconds;

        // let's remove a pending data (if exists) corresponding to this event
        pPendingData = pCurEntry->GetAndRemovePendingBufferIfAny(nEventNumber);
        if(pPendingData){
            // bed luck we have lost data
            fprintf(stderr, "!!! data loss in the channel %d. Event number before %d is not found (%d).\n",
                    pCurEntry->channelNumber(), nEventNumber,pPendingData->eventNumber);
            if( !AddJobForRootThread(pPendingData,pCurEntry) ){
                pCurEntry->SetError(-2);
                if(!bErrorNoEntrySet){fprintf(stderr, "No place in root fifo!\n");bErrorNoEntrySet=true;}
            }
            else{ bErrorNoEntrySet = false; }
            //pCurEntry->ResetPending(nEventNumber);
            //if(nEventNumberOfPending>pCurEntry->LastEventNumberHandled()){pCurEntry->SetLastEventNumberHandled(nEventNumber-NUMBER_OF_PENDING_PACKS);}
            pCurEntry->SetLastEventNumberHandled(pPendingData->eventNumber);  // checking is done inside
        }

        // let's remove all pending data (if any) after this event
        for(
            nEventNumberToTryPending=nEventNumber+1,nLastEventToTryPending=nEventNumber+NUMBER_OF_PENDING_PACKS;
            nEventNumberToTryPending<nLastEventToTryPending;++nEventNumberToTryPending)
        {
            pPendingData = pCurEntry->GetAndRemovePendingBufferIfAny(nEventNumberToTryPending);
            if(!pPendingData){break;}
            if( !AddJobForRootThread(pPendingData,pCurEntry) ){
                pCurEntry->SetError(-2);
                if(!bErrorNoEntrySet){fprintf(stderr, "No place in root fifo!\n");bErrorNoEntrySet=true;}
            }
            else{ bErrorNoEntrySet = false; }
            //pCurEntry->ResetPending(nEventNumberToTryPending);
            pCurEntry->SetLastEventNumberHandled(pPendingData->eventNumber);
        }


        // let's check whether this data should wait (become pending, untill previous package arrives) to make packages in correct order
        if( ((nEventNumber-nLastEventNumberHandled)>1)&&nLastEventNumberHandled ){
            pCurEntry->SetPendingBuffer(nEventNumber,pMemory2);
            goto prepareNextReceiveBuffer;
        }

        // ererything is ok add data to root
        if(!AddJobForRootThread(pMemory2,pCurEntry)){
            pCurEntry->SetMemoryBack(pMemory2);
            pCurEntry->SetError(-2);
            if(!bErrorNoEntrySet){fprintf(stderr, "No place in root fifo!\n");bErrorNoEntrySet=true;}
        }
        else{ bErrorNoEntrySet = false; }
        pCurEntry->SetLastEventNumberHandled(nEventNumber);

prepareNextReceiveBuffer:
        pMemory2 = pCurEntry->GetNewMemoryForNetwork();

    } // while( m_nWork )

    aListener.CloseSock();
    //delete pMemForRcv;
    //DEBUG_("!!!!!!!!!!!!!!!!!!!!! m_nWork=%d\n",m_nWork);

}



/*///////////////////////////////////////////////////////////////////////////////*/
pitz::daq::SingleEntryUdp::SingleEntryUdp(entryCreationType::Type a_creationType,const char* a_entryLine,TypeConstCharPtr* a_pHelper)
        :
        SingleEntry(a_creationType,a_entryLine,a_pHelper),
        m_pFctParent(NEWNULLPTR2)
{

    m_nReserved = 0;
    //m_isPending = 0;
    memset(m_vPendings,0,sizeof(m_vPendings));

    switch(a_creationType)
    {
    case entryCreationType::fromOldFile:
        {
            int from, samples, step;
            char doocs_url[256], daqName[256];

            sscanf(a_entryLine,"%s %s %d %d %d %d",
                   daqName,doocs_url,&from,&samples,&step,&m_nChannelNumber);
        }
        break;

    case entryCreationType::fromConfigFile: case entryCreationType::fromUser:
        {
            const char *pcNext = strstr(a_entryLine,SPECIAL_KEY "=");

            if(!pcNext){throw errorsFromConstructor::syntax;}
            pcNext += strlen(SPECIAL_KEY "=");
            m_nChannelNumber = atoi(pcNext);

        }
        break;

    default:
        throw errorsFromConstructor::type;
        break;
    }

    m_nChannelNumber = m_nChannelNumber<0 ? 0 : m_nChannelNumber;
    m_nChannelNumber %= MAX_CHANNELS_NUM;

}


pitz::daq::SingleEntryUdp::~SingleEntryUdp()
{
    if(m_pFctParent){m_pFctParent->m_vMapper[m_nChannelNumber] = NEWNULLPTR2;}
}


void pitz::daq::SingleEntryUdp::SetFctParent(EqFctUdpMcast*  a_pFctParent)
{
    m_pFctParent = a_pFctParent;
    m_pFctParent->m_vMapper[m_nChannelNumber] = this;
}


void pitz::daq::SingleEntryUdp::PermanentDataIntoFile(FILE* a_fpFile)const
{
    fprintf(a_fpFile,SPECIAL_KEY "=%d; ",m_nChannelNumber);
}


const char* pitz::daq::SingleEntryUdp::rootFormatString()const
{
    return "seconds/I:gen_event/I:array_value[2048]/F";
}


void pitz::daq::SingleEntryUdp::ValueStringByKeyInherited(bool a_bReadAll, const char* a_request, char* a_buffer, int a_bufferLength)const
{    
    char* pcBufToWrite(a_buffer);
    int nWritten,nBufLen(a_bufferLength);

    if(a_bReadAll ||strstr(SPECIAL_KEY,a_request)){
        nWritten = snprintf(pcBufToWrite,static_cast<size_t>(nBufLen),SPECIAL_KEY "=%d; ",m_nChannelNumber);
        pcBufToWrite += nWritten;
        nBufLen -= nWritten;
        if(nBufLen<=0){return;}
    }
}



DEC_OUT_PD(SingleData)* pitz::daq::SingleEntryUdp::GetAndRemovePendingBufferIfAny(int a_indexOrEventNumber)
{
    int nIndex = a_indexOrEventNumber%NUMBER_OF_PENDING_PACKS;
    DEC_OUT_PD(SingleData)* pReturn = m_vPendings[nIndex];
    //unsigned int unMask = ~(1<<nIndex);

    m_vPendings[nIndex]=NEWNULLPTR2;
    //m_isPending &= unMask;
    return pReturn;
}


void pitz::daq::SingleEntryUdp::SetPendingBuffer(int a_indexOrEventNumber, DEC_OUT_PD(SingleData)* a_pBuffer)
{
    int nIndex = a_indexOrEventNumber%NUMBER_OF_PENDING_PACKS;
    //unsigned int unMask = 1<<nIndex;
    //m_isPending |= unMask;
    m_vPendings[nIndex] = a_pBuffer;
}

/*/////////////////////////////////////////////////////////////////////////////////////*/


#define SWAP_4_BYTES_RAW(_pBufForSwap,_tmpVar)  \
    (_tmpVar)=(_pBufForSwap)[0];(_pBufForSwap)[0]=(_pBufForSwap)[3]; (_pBufForSwap)[3]=(_tmpVar);   \
    (_tmpVar)=(_pBufForSwap)[1];(_pBufForSwap)[1]=(_pBufForSwap)[2]; (_pBufForSwap)[2]=(_tmpVar)

#define SWAP_4_BYTES(_pBufForSwap,_tmpVar)  SWAP_4_BYTES_RAW(reinterpret_cast<char*>(_pBufForSwap),_tmpVar)


static int SwapDataIfNecessary(DEC_OUT_PD(SingleData) *a_pMemory)
{
    DATA_struct* pData = reinterpret_cast<DATA_struct*>(a_pMemory);

    if(pData->endian!=1){
        int i;
        char tmpVar;

        SWAP_4_BYTES(&pData->branch_num,tmpVar);
        SWAP_4_BYTES(&pData->seconds,tmpVar);
        SWAP_4_BYTES(&pData->gen_event,tmpVar);
        SWAP_4_BYTES(&pData->samples,tmpVar);
        for(i=0;i<2048;++i){
            SWAP_4_BYTES(&pData->f[i],tmpVar);
        }

    }

    return pData->branch_num;
}

