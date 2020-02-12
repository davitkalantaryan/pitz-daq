//
// file eq_pitznoadc0.cc
//
// pitznoadc0 Eq function class

// to be deleted
#define TO_BE_UNDERSTOOD_ASOVA_THREAD   0

//#define IMPLEMENT_CONDITIONS
//#define ALARM_UNDERSTOOD

#include <cstdlib>
#define atoll       atol
#define strtoull    strtoul
#include "pitz_daq_eqfctrr.hpp"
#include <ctime>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <cerrno>

#include <sys/stat.h>
#include <sys/shm.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <libgen.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "printtostderr.h"

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


#include <TROOT.h>
#include <TString.h>
#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include <TBasket.h>
#include <TObject.h>
#include <TSystem.h>
#include <TError.h>
#include <TNetFile.h>
#include <stdio.h>
#include "pitz_daq_singleentrydoocs_base.hpp"

#include "eq_errors.h"
#include "eq_sts_codes.h"
#include "eq_fct_errors.h"

#define DATA_TYPE_TAKE_FR_DOOCS -1
#define LEN_OF_SPECIAL_MIN1   511
#define SPECIAL_KEY_DOOCS "doocs"
#define SPECIAL_KEY_DATA_TYPE "type"
#define SPECIAL_KEY_DATA_SAMPLES "samples"

namespace pitz{namespace daq{

class SingleEntryRR : public SingleEntryDoocsBase
{
public:
    SingleEntryRR(entryCreationType::Type creationType,const char* entryLine,TypeConstCharPtr* a_pHelper);
    ~SingleEntryRR();

private:
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
    EqAdr dcsAddr;
    EqData dataIn, dataOut;
    EqCall eqCall;
    const char* cpcLine;
    SingleEntryRR* pEntry = new SingleEntryRR(a_creationType,a_entryLine,&cpcLine);
    int nDcsCallResult;

    if(!pEntry){return NEWNULLPTR2;}

    dcsAddr.adr( pEntry->doocsUrl());
    dataIn.init();dataOut.init();
    nDcsCallResult = eqCall.get(&dcsAddr,&dataIn,&dataOut);

    if(nDcsCallResult != 0){
        goto returnPoint;
    }
    else if(dataOut.type() != pEntry->dataType()){

        if(pEntry->dataType() == DATA_TYPE_TAKE_FR_DOOCS){
            pEntry->setDataType(dataOut.type());
        }
        else {nDcsCallResult = -2; goto returnPoint;}
    }


returnPoint:

    if(nDcsCallResult && (a_creationType == entryCreationType::fromUser)){
        delete pEntry;return NEWNULLPTR2;
    }

    return pEntry;
}

extern int g_nInTheStack;

void pitz::daq::EqFctRR::DataGetterThread(SNetworkStruct* a_pNet)
{
    DEC_OUT_PD(SingleData)* pMemory;
    ::std::list< SingleEntry* >::const_iterator pIter, pIterEnd;
    const ::std::list< SingleEntry* >& entriesList = a_pNet->daqEntries();

    //data::memory::ForServerBase* pMemory;
    SingleEntryRR *pEntry;
    //SingleEntryRR *pEntry, *pCurEntry, *pNextOfLast;
    EqAdr  dcsAddr;
    EqData dataIn, dataOut;
    EqCall eqCall;
    int nDcsCallresult, nEventNumber;
    int nWaitMs;
    int nTime;

    while(shouldWork() && a_pNet->shouldRun()){
        //m_mutexForEntries.lock_shared();
        //pNextOfLast = (SingleEntryRR*)a_pNet->last()->next;
        GetEventAndTime(&nEventNumber,&nTime);

        pIterEnd = entriesList.end();
        for(pIter=entriesList.begin();pIter!=pIterEnd;++pIter){
            pEntry = static_cast< SingleEntryRR* >( *pIter );
            //if(pEntry->LoadOrValidateData(pNetZmq->m_pContext)){
            //    if(pEntry->lockEntryForNetwork()){
            //        validEntries.push_back(pEntry);
            //        //validEntries.AddNewElement(pEntryCheck->socket(),pEntryCheck);
            //    }
            //}

            dcsAddr.adr(pEntry->doocsUrl());
            dataIn.init();dataOut.init();
            nDcsCallresult = eqCall.get(&dcsAddr,&dataIn,&dataOut);

            if(nDcsCallresult){
                //if(pEntry->MaskErrors(nullptr)){}
                pEntry->SetError(-3);
                continue;
            }

            pMemory = pEntry->GetNewMemoryForNetwork();
            pMemory->eventNumber = nEventNumber;
            pMemory->timestampSeconds = nTime;
            pEntry->FromDoocsToMemory(pMemory,&dataOut);

            if(!AddJobForRootThread(pMemory,pEntry)){
                pEntry->SetError(-2);
                fprintf(stderr, "No place in root fifo!\n");
            }
        }

        nWaitMs = m_pollingPeriod.value();
        if(!nWaitMs){nWaitMs=1;}
        SleepMs(nWaitMs);
    }  // while(m_nWork){

}



/*////////////////////////////////////////////////////*/

pitz::daq::SingleEntryRR::SingleEntryRR(entryCreationType::Type a_creationType,const char* a_entryLine,TypeConstCharPtr* a_pHelper)
        :
        SingleEntryDoocsBase(a_creationType, a_entryLine,a_pHelper)
{
}


pitz::daq::SingleEntryRR::~SingleEntryRR()
{
}


