//
// file eq_pitznoadc0.cc
//
// pitznoadc0 Eq function class


#include "pitz_daq_eqfctrr.hpp"
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <sys/shm.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <libgen.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "printtostderr.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include "pitz_daq_singleentrydoocs_base.hpp"
#include <pitz_daq_data_handling_types.h>
#include <pitz_daq_data_handling_daqdev.h>
#include "eq_errors.h"
#include "eq_sts_codes.h"
#include "eq_fct_errors.h"


namespace pitz{namespace daq{

class SingleEntryRR : public SingleEntryDoocsBase
{
public:
    SingleEntryRR(entryCreationType::Type creationType,const char* entryLine,TypeConstCharPtr* a_pHelper);
    ~SingleEntryRR();

    void GetDataAndFill();

private:
	int m_itemsCountPerEntry;
	int m_reserved;

};
}}


//
// used during startup of the server to create the locations
//
EqFct* eq_create(int a_eq_code, void* )
{
        ::EqFct* pRet = NEWNULLPTR2;
        switch (a_eq_code)
        {
        case EqFctRR_code:pRet = new pitz::daq::EqFctRR;break;
        default: break;
        }
        return pRet;
}


pitz::daq::EqFctRR::EqFctRR()
    :
      m_pollingPeriod("POLLING.PERIOD time in ms to wait between 2 data gets",this)
{
}


pitz::daq::EqFctRR::~EqFctRR()
{
}


int pitz::daq::EqFctRR::fct_code()
{
    return EqFctRR_code;
}


pitz::daq::SingleEntry* pitz::daq::EqFctRR::CreateNewEntry(entryCreationType::Type a_creationType,const char* a_entryLine)
{
    const char* cpcLine;
    return new SingleEntryRR(a_creationType,a_entryLine,&cpcLine);
}


void pitz::daq::EqFctRR::DataGetterFunctionWithWait(const SNetworkStruct* /*a_pNet*/, const ::std::vector<SingleEntry*>& a_pEntries)
{
    int nWaitMs;
    SingleEntryRR* pEntry;
    size_t unIndex;
    const size_t cunValidSize( a_pEntries.size() );

    for(unIndex=0;unIndex<cunValidSize;++unIndex){
        pEntry = static_cast< SingleEntryRR* >( a_pEntries[unIndex] );
        pEntry->GetDataAndFill();

    }

    nWaitMs = m_pollingPeriod.value();
    if(nWaitMs<10){nWaitMs=10;}
    SleepMs(nWaitMs);
}


/*////////////////////////////////////////////////////*/

pitz::daq::SingleEntryRR::SingleEntryRR(entryCreationType::Type a_creationType,const char* a_entryLine,TypeConstCharPtr* a_pHelper)
        :
        SingleEntryDoocsBase(a_creationType, a_entryLine,a_pHelper)
{
	m_itemsCountPerEntry = 0;
	m_reserved = 0;
}


pitz::daq::SingleEntryRR::~SingleEntryRR()
{
}


void pitz::daq::SingleEntryRR::GetDataAndFill()
{
    void* pDoocsData;
    size_t expectedDataLength;
    int64_t timeSeconds, genEvent;
    DEC_OUT_PD(TypeAndCount) entryInfo;
    EqData dataOut;
	int nSamples;
    DEC_OUT_PD(SingleData2)* pNewMemory;
	struct PrepareDaqEntryInputs in;
	struct PrepareDaqEntryOutputs out;

	memset(&in,0,sizeof(in));
	memset(&out,0,sizeof(out));

	if(!GetEntryInfoFromDoocsServer(&dataOut,m_doocsUrl.value(),&entryInfo.type,&nSamples)){
		IncrementError(NETWORK_READ_ERROR,"DOOCS RR error");
        return;
    }

	in.dataType = entryInfo.type;
	out.inOutSamples = nSamples;

	if(!PrepareDaqEntryBasedOnType(&in,&out)){
		IncrementError(UNABLE_TO_PREPARE_DATA,"unable to prepare data");
        return ;
    }

	if((m_dataType.value()!=entryInfo.type)||(m_itemsCountPerEntry!=out.inOutSamples)){
		IncrementError(DATA_TYPE_MISMATCH,"data type mismatch");
        return ;
    }

    pDoocsData=GetDataPointerFromEqData(&dataOut,&timeSeconds,&genEvent);
    if(!pDoocsData){
		IncrementError(UNABLE_TO_GET_DOOCS_DATA,"unable to get doocs data");
        return ;
    }

	expectedDataLength = out.oneItemSize*static_cast<uint32_t>(out.inOutSamples);
    //pNewMemory =CreateDataWithOffset2(0,expectedDataLength);
    pNewMemory =CreateDataWithOffset2(0);
    pNewMemory->data = CreatePitzDaqBuffer(expectedDataLength);

    pNewMemory->header.eventNumber = static_cast<decltype (pNewMemory->header.eventNumber)>(genEvent);
    pNewMemory->header.timestampSeconds = static_cast<decltype (pNewMemory->header.timestampSeconds)>(timeSeconds);

    memcpy(pNewMemory->data,pDoocsData,expectedDataLength);

    Fill(pNewMemory);
}
