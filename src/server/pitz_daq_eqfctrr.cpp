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
#include <pitz_daq_data_daqdev_common.h>
#include "eq_errors.h"
#include "eq_sts_codes.h"
#include "eq_fct_errors.h"
#include <pitz_daq_collector_global.h>


namespace pitz{namespace daq{

class SingleEntryRR final : public SingleEntryDoocsBase
{
public:
    SingleEntryRR(entryCreationType::Type creationType,const char* entryLine,TypeConstCharPtr* a_pHelper);
	~SingleEntryRR() override;

	DEC_OUT_PD(Header)* GetDataAndAddForRoot();

private:
	void  FreeUsedMemory(DEC_OUT_PD(Header)* usedMemory) override;

private:
	//uint32_t m_expectedDataLength;
	//int m_reserved;

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
	DEC_OUT_PD(Header)* pNewEntry;
    const size_t cunValidSize( a_pEntries.size() );

    for(unIndex=0;unIndex<cunValidSize;++unIndex){
        pEntry = static_cast< SingleEntryRR* >( a_pEntries[unIndex] );
		pNewEntry = pEntry->GetDataAndAddForRoot();
		if(pNewEntry){
			AddJobForRootThread(pNewEntry,pEntry);
		}

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
	//m_expectedDataLength = static_cast<uint32_t>( (m_samples)*m_nSingleItemSize );
	//m_reserved = 0;
}


pitz::daq::SingleEntryRR::~SingleEntryRR()
{
}


DEC_OUT_PD(Header)* pitz::daq::SingleEntryRR::GetDataAndAddForRoot()
{
	int nReturn;
	bool bShouldDeletePointer=false;
	EqData* pDataOut = new EqData;
	EqAdr eqAddr;
	EqData dataIn;
	EqCall eqCall;
	DEC_OUT_PD(Header)* pFillData;

	struct PrepareDaqEntryInputs in;
	struct PrepareDaqEntryOutputs out;

	memset(&in,0,sizeof(in));
	memset(&out,0,sizeof(out));

	eqAddr.adr(m_doocsUrl.value());
	nReturn = eqCall.get(&eqAddr,&dataIn,pDataOut);

	if(nReturn){
		::std::string errorString = pDataOut->get_string();
		::std::cerr << "doocsAdr:"<<m_doocsUrl.value() << ",err:"<<errorString << ::std::endl;
		delete pDataOut;
		return nullptr;
	}

	in.dataType = pDataOut->type();
	in.inNeededBufferSize = m_nMaxBufferForNextIter;
	out.inOutSamples = pDataOut->length();

	if(!PrepareDaqEntryBasedOnType(&in,&out)){
		IncrementError(UNABLE_TO_GET_DOOCS_DATA,"unable to get doocs data");
		delete pDataOut;
		return nullptr;
	}


	if(out.outNeededBufferSize!=m_nMaxBufferForNextIter){
		if(m_recalculateNumberOfSamples || (out.outNeededBufferSize<m_nMaxBufferForNextIter)){
			m_samples = out.inOutSamples;
			m_recalculateNumberOfSamples = 0;
		}
		else{
			m_recalculateNumberOfSamples = 1;
		}
	}
	m_nMaxBufferForNextIter = out.outNeededBufferSize;
	if(m_nMaxBufferForNextIter<1){
		IncrementError(UNABLE_TO_GET_DOOCS_DATA,"unable to get doocs data");
		delete pDataOut;
		return nullptr;
	}

	pFillData = GetDataPointerFromEqData(in.inNeededBufferSize,pDataOut,&bShouldDeletePointer);
	if(!pFillData){
		IncrementError(UNABLE_TO_GET_DOOCS_DATA,"unable to get doocs data");
		delete pDataOut;
		return nullptr;
	}

	if(!bShouldDeletePointer){
		data::SMemoryHeader* pHeader = HEADER_FROM_HEADER(pFillData);
		pHeader->priv = pDataOut;
		MAKE_PADDIN32_ZERO(pHeader,0)
	}
	else{
		delete pDataOut;
	}

	return pFillData;
}


void pitz::daq::SingleEntryRR::FreeUsedMemory(DEC_OUT_PD(Header)* a_usedMemory)
{
	if(a_usedMemory){
		data::SMemoryHeader* pHeader = HEADER_FROM_HEADER(a_usedMemory);
		if( HAS_HEADER_RAW(pHeader) ){
			EqData* pDataOut = static_cast<EqData*>(pHeader->priv);
			delete pDataOut;
		}
		else{
			FreePitzDaqBuffer(a_usedMemory);
		}
	}
}
