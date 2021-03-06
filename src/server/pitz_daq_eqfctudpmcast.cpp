/*
 * Remaining questions
 *  1) pitzbpmadc_rpc_server.cc, Lns: 1025-1042 (imast@?)
 */

#include "pitz_daq_eqfctudpmcast.hpp"
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
#include "eq_fct_errors.h"
#include "mclistener.hpp"
#include "udpmcastdaq_commonheader.h"
#include "pitz_daq_globalfunctionsiniter.hpp"
#include "common_daq_definations.h"
#include <pitz_daq_data_daqdev_common.h>
//#include "alog.h"
#include <TPluginManager.h> // https://root.cern.ch/phpBB3/viewtopic.php?t=9816
#include <iostream>
#ifdef _TEST_GUI_
#include <simple_plotter_c.h>
#endif

#define SPECIAL_KEY_CHANNEL "channel"
// char* pcPointerToFree = reinterpret_cast<char*>(a_usedMemory) - sizeof(int);
#define UDP_DATA_FROM_HEADER(_header)	reinterpret_cast<struct DATA_struct*>( reinterpret_cast<char*>(_header) - sizeof(int) )

static int SwapDataIfNecessary(DEC_OUT_PD(Header) *a_pMemory);

namespace pitz{namespace daq{

class SingleEntryUdp : public SingleEntry
{
public:
    SingleEntryUdp(EqFctCollector* a_pParent,entryCreationType::Type creationType,const char* entryLine, TypeConstCharPtr* a_pHelper);
    ~SingleEntryUdp() OVERRIDE2;

    const char* rootFormatString()const OVERRIDE2;

    void SetFctParent(EqFctUdpMcast*  a_pFctParent);
    int channelNumber()const{return m_nChannelNumber;}

    int LastEventNumberHandled()const;
    void SetLastEventNumberHandled(int);

	DEC_OUT_PD(Header)* GetAndRemovePendingBufferIfAny(int indexOrEventNumber);
	void SetPendingBuffer(int a_indexOrEventNumber, DEC_OUT_PD(Header)* a_pBuffer);

	void  FreeUsedMemory(DEC_OUT_PD(Header)* usedMemory) OVERRIDE2;

private:
    EqFctUdpMcast*                      m_pFctParent;
    EntryParams::IntParam<int>          m_nChannelNumber;
    int                                 m_nLastEventNumberHandled;
    int                                 m_nReservedUdp1;
	DEC_OUT_PD(Header)*					m_vPendings[NUMBER_OF_PENDING_PACKS];
	const char*							m_rootFormatString;

};


class SNetworkStructUdp : public SNetworkStruct
{
public:
    SNetworkStructUdp( EqFctCollector* pParentCollector, const char* a_cpcHostName );
    ~SNetworkStructUdp() OVERRIDE2 ;

public:
    MClistener         m_socket;
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
       m_hostName("HOST__NAME multicaster host name ", this)

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
    SingleEntryUdp* pEntry = new SingleEntryUdp(this,a_creationType,a_entryLine,&cpcLine);
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


pitz::daq::SNetworkStruct* pitz::daq::EqFctUdpMcast::CreateNewNetworkStruct()
{
    return new SNetworkStructUdp(this,m_hostName.value());
}


void pitz::daq::EqFctUdpMcast::DataGetterFunctionWithWait(const SNetworkStruct* a_pNet, const ::std::vector<SingleEntry*>& /*pEntries*/)
{
    SingleEntryUdp* pCurEntry;
    //DEC_OUT_PD(SingleData2) *pPendingData, *pMemory = CreateDataWithOffset2(8,sizeof(DATA_struct)-8); // 8 = 4(for endian), 4(for branchNum)
	struct DATA_struct* pWholeData = static_cast<struct DATA_struct*>(CreatePitzDaqBuffer(sizeof(struct DATA_struct)));
	DEC_OUT_PD(Header) *pPendingData, *pMemory = reinterpret_cast< DEC_OUT_PD(Header)* >( &(pWholeData->branch_num_in_rcv_and_next_max_buffer_size_on_root) ); // 8 = 4(for endian), 4(for branchNum)
	int nBranchNum, nBranchNumFirst=-1;
    int nbytes;
    int nEventNumberToTryPending,nLastEventToTryPending;
    int nEventNumber(0), nLastEventNumberHandled;
    const SNetworkStructUdp* pNet = static_cast<const SNetworkStructUdp*>(a_pNet);

receivePoint:
	nbytes = pNet->m_socket.recvC(pWholeData,sizeof(DATA_struct));

    if( nbytes != sizeof(DATA_struct) ){

		printtostderr("ERROR","nbytes != sizeof(DATA_struct)");
        //ERR_LOG("nbytes<0");
		fprintf(stderr,"Error during reading nBytes=%d\n",nbytes);
        return;
    }
	nBranchNum = SwapDataIfNecessary(pMemory)%MAX_CHANNELS_NUM;

    pCurEntry = static_cast<SingleEntryUdp*>(m_vMapper[nBranchNum]);

	if(!pCurEntry){
		// we do not handle this channel, try next
		if(nBranchNumFirst<0){nBranchNumFirst=nBranchNum;}
		else if(nBranchNum==nBranchNumFirst){
			// we did one iteration no needed data, return here
			return;
		}
		goto receivePoint;
	}

    nLastEventNumberHandled = pCurEntry->LastEventNumberHandled();
	nEventNumber = pMemory->gen_event;

    if(nEventNumber<=nLastEventNumberHandled){
        // report on repetition (this is just warning forgot it :) )
        return;
    }

    // let's remove a pending data (if exists) corresponding to this event
    pPendingData = pCurEntry->GetAndRemovePendingBufferIfAny(nEventNumber);
    if(pPendingData){
        // bad luck we have lost data
        fprintf(stderr, "!!! data loss in the channel %d. Event number before %d is not found (%d).\n",
				pCurEntry->channelNumber(), nEventNumber,pPendingData->gen_event);
        AddJobForRootThread(pPendingData,pCurEntry);
		pCurEntry->SetLastEventNumberHandled(pPendingData->gen_event);  // checking is done inside
    }

    // let's remove all pending data (if any) after this event
    for(
        nEventNumberToTryPending=nEventNumber+1,nLastEventToTryPending=nEventNumber+NUMBER_OF_PENDING_PACKS;
        nEventNumberToTryPending<nLastEventToTryPending;++nEventNumberToTryPending)
    {
        pPendingData = pCurEntry->GetAndRemovePendingBufferIfAny(nEventNumberToTryPending);
        if(!pPendingData){break;}
        AddJobForRootThread(pPendingData,pCurEntry);
		pCurEntry->SetLastEventNumberHandled(pPendingData->gen_event);
    }

    // let's check whether this data should wait (become pending, untill previous package arrives) to make packages in correct order
    if( ((nEventNumber-nLastEventNumberHandled)>1)&&nLastEventNumberHandled ){
        pCurEntry->SetPendingBuffer(nEventNumber,pMemory);
        return;
    }

    // ererything is ok add data to root
    AddJobForRootThread(pMemory,pCurEntry);
    pCurEntry->SetLastEventNumberHandled(nEventNumber);
}



/*///////////////////////////////////////////////////////////////////////////////*/
pitz::daq::SingleEntryUdp::SingleEntryUdp(EqFctCollector* a_pParent,entryCreationType::Type a_creationType,const char* a_entryLine,TypeConstCharPtr* a_pHelper)
        :
        SingleEntry(a_pParent,a_creationType,a_entryLine,a_pHelper),
        m_pFctParent(NEWNULLPTR2),
        m_nChannelNumber(SPECIAL_KEY_CHANNEL)
{
    int nChannelNumber=0;
    bool bCallIniter = false;
	struct PrepareDaqEntryInputs in;
	struct PrepareDaqEntryOutputs out;

	memset(&in,0,sizeof(in));
	memset(&out,0,sizeof(out));

	in.dataType = DATA_FLOAT;
	out.inOutSamples = 2048;
	m_rootFormatString = PrepareDaqEntryBasedOnType(&in,&out);

	m_nSingleItemSize = 4;
	m_samples = (2048);
	m_dataType.set(DATA_FLOAT);

    m_nReservedUdp1 = 0;
    m_nLastEventNumberHandled = 0;
    memset(m_vPendings,0,sizeof(m_vPendings));

    AddNewParameterToEnd(&m_nChannelNumber,false,true);

    switch(a_creationType)
    {
    case entryCreationType::fromOldFile:
        {
            int from, samples, step;
            char doocs_url[256], daqName[256];

            sscanf(a_entryLine,"%s %s %d %d %d %d",
                   daqName,doocs_url,&from,&samples,&step,&nChannelNumber);
            //m_nChannelNumber = (nChannelNumber);
        }
        break;

    case entryCreationType::fromConfigFile: case entryCreationType::fromUser:
        bCallIniter = true;
        break;

    default:
        throw errorsFromConstructor::type;
    }

    if(bCallIniter){
        m_nChannelNumber.FindAndGetFromLine(a_entryLine);
        nChannelNumber = (m_nChannelNumber);
    }

    nChannelNumber = nChannelNumber<0 ? 0 : nChannelNumber;
    nChannelNumber %= MAX_CHANNELS_NUM;
    m_nChannelNumber = (nChannelNumber);
	m_nMaxBufferForNextIter = m_nSingleItemSize*(m_samples);
	m_isLoadedFromLine = 1;
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


const char* pitz::daq::SingleEntryUdp::rootFormatString()const
{
    //return "seconds/I:gen_event/I:array_value[2048]/F";
	return m_rootFormatString; // "data[2048]/F";
}


void pitz::daq::SingleEntryUdp::SetLastEventNumberHandled(int a_nLastEventNumber)
{
    m_nLastEventNumberHandled = a_nLastEventNumber;
}


int pitz::daq::SingleEntryUdp::LastEventNumberHandled() const
{
    return m_nLastEventNumberHandled ;
}


DEC_OUT_PD(Header)* pitz::daq::SingleEntryUdp::GetAndRemovePendingBufferIfAny(int a_indexOrEventNumber)
{
    int nIndex = a_indexOrEventNumber%NUMBER_OF_PENDING_PACKS;
	DEC_OUT_PD(Header)* pReturn = m_vPendings[nIndex];
    //unsigned int unMask = ~(1<<nIndex);

    m_vPendings[nIndex]=NEWNULLPTR2;
    //m_isPending &= unMask;
    return pReturn;
}


void pitz::daq::SingleEntryUdp::SetPendingBuffer(int a_indexOrEventNumber, DEC_OUT_PD(Header)* a_pBuffer)
{
    int nIndex = a_indexOrEventNumber%NUMBER_OF_PENDING_PACKS;
    //unsigned int unMask = 1<<nIndex;
    //m_isPending |= unMask;
    m_vPendings[nIndex] = a_pBuffer;
}


void pitz::daq::SingleEntryUdp::FreeUsedMemory(DEC_OUT_PD(Header)* a_usedMemory)
{
	struct DATA_struct* pcPointerToFree = UDP_DATA_FROM_HEADER(a_usedMemory);
	FreePitzDaqBuffer(pcPointerToFree);
}


/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
pitz::daq::SNetworkStructUdp::SNetworkStructUdp( EqFctCollector* a_pParentCollector, const char* a_cpcHostName )
    :
      SNetworkStruct(a_pParentCollector)
{
    if(m_socket.ConnectToTheMGroup(a_cpcHostName)){
        std::cerr<<"Unable to connect to host\""<<a_cpcHostName<<"\"\n";
        exit(1);
    }
    m_socket.SetSocketTimeout(2000);
}


pitz::daq::SNetworkStructUdp::~SNetworkStructUdp()
{
    StopThreadThenDeleteAndClearEntries();
    m_socket.CloseSock();
}

/*/////////////////////////////////////////////////////////////////////////////////////*/


#define SWAP_4_BYTES_RAW(_pBufForSwap,_tmpVar)  \
    (_tmpVar)=(_pBufForSwap)[0];(_pBufForSwap)[0]=(_pBufForSwap)[3]; (_pBufForSwap)[3]=(_tmpVar);   \
    (_tmpVar)=(_pBufForSwap)[1];(_pBufForSwap)[1]=(_pBufForSwap)[2]; (_pBufForSwap)[2]=(_tmpVar)

#define SWAP_4_BYTES(_pBufForSwap,_tmpVar)  SWAP_4_BYTES_RAW(reinterpret_cast<char*>(_pBufForSwap),_tmpVar)


static int SwapDataIfNecessary(DEC_OUT_PD(Header) *a_pMemory)
{
	DATA_struct* pData = UDP_DATA_FROM_HEADER(a_pMemory);

    if(pData->endian!=1){
        int i;
        char tmpVar;

		SWAP_4_BYTES(&pData->branch_num_in_rcv_and_next_max_buffer_size_on_root,tmpVar);
        SWAP_4_BYTES(&pData->seconds,tmpVar);
        SWAP_4_BYTES(&pData->gen_event,tmpVar);
        SWAP_4_BYTES(&pData->samples,tmpVar);
        for(i=0;i<2048;++i){
            SWAP_4_BYTES(&pData->f[i],tmpVar);
        }

    }

	return pData->branch_num_in_rcv_and_next_max_buffer_size_on_root;
}

