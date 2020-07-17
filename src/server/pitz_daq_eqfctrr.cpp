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


namespace pitz{namespace daq{

class SingleEntryRR final : public SingleEntryDoocsBase
{
public:
    SingleEntryRR(entryCreationType::Type creationType,const char* entryLine,TypeConstCharPtr* a_pHelper);
	~SingleEntryRR() override;

private:
	void  FreeUsedMemory(DEC_OUT_PD(Header)* usedMemory) override;

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


void pitz::daq::SingleEntryRR::FreeUsedMemory(DEC_OUT_PD(Header)* a_usedMemory)
{
	//
}
