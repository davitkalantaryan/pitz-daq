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
#include "pitz_daq_singleentrydoocs.hpp"

#include "eq_errors.h"
#include "eq_sts_codes.h"
#include "eq_fct_errors.h"

#define DATA_TYPE_TAKE_FR_DOOCS -1
#define LEN_OF_SPECIAL_MIN1   511
#define SPECIAL_KEY_DOOCS "doocs"
#define SPECIAL_KEY_DATA_TYPE "type"
#define SPECIAL_KEY_DATA_SAMPLES "samples"

namespace pitz{namespace daq{

class SingleEntryRR : public SingleEntryDoocs
{
public:
    SingleEntryRR(entryCreationType::Type creationType,const char* entryLine,TypeConstCharPtr* a_pHelper);
    ~SingleEntryRR();

    //const char* specialStringForDoocsProperty()const;
    void ValueStringByKeyInherited(bool bReadAll, const char* request, char* buffer, int bufferLength)const;
    const char* rootFormatString()const;
    void PermanentDataIntoFile(FILE* fpFile)const;
    //bool ValueStringByKeyInherited(const std::string& a_key, char* a_buffer, int a_bufferLength);

    const char* doocsUrl()const {return m_doocsUrl;}
    int dataType()const{return m_dataType;}
    void setDataType(int a_dataType){m_dataType = a_dataType;}

private:
    int m_dataType;
    int m_nSamples;
};
}}

#define DOOCS_URI_MAX_LEN   256


static bool GetEventAndTime(int* a_event, int* a_time);

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
        SingleEntryDoocs(a_creationType, a_entryLine,a_pHelper)
{

    size_t unStrLen ;

    switch(a_creationType)
    {
    case entryCreationType::fromOldFile:
        {
            int from, step;
            char doocs_url[DOOCS_URI_MAX_LEN],daqName[256];

            sscanf(a_entryLine,"%s %s %d %d %d %d",
                   daqName,doocs_url,&from,&m_nSamples,&step,&m_dataType);

            unStrLen = strlen(doocs_url);
            m_doocsUrl = static_cast<char*>(malloc(unStrLen + 1));
            if(!m_doocsUrl){throw errorsFromConstructor::lowMemory;}
            memcpy(m_doocsUrl,doocs_url,unStrLen);
            m_doocsUrl[unStrLen] = 0;
        }
        break;

    case entryCreationType::fromConfigFile:
        {
            const char *tmpStr,*pcNext = strstr(a_entryLine,SPECIAL_KEY_DOOCS "=");
            if(!pcNext){throw errorsFromConstructor::syntax;}
            pcNext += strlen( SPECIAL_KEY_DOOCS "=" );
            //strspn();// later ignore empty
            tmpStr = strpbrk(pcNext,POSIIBLE_TERM_SYMBOLS);
            if(!tmpStr){throw errorsFromConstructor::syntax;}
            unStrLen = static_cast<size_t>(tmpStr - pcNext);
            m_doocsUrl = static_cast<char*>(malloc(unStrLen+1));
            if(!m_doocsUrl){throw errorsFromConstructor::lowMemory;}
            memcpy(m_doocsUrl,pcNext,unStrLen);
            m_doocsUrl[unStrLen] = 0;

            // Find data type
            pcNext = strstr(a_entryLine,SPECIAL_KEY_DATA_TYPE "=");
            if(!pcNext){throw errorsFromConstructor::syntax;}
            pcNext += strlen( SPECIAL_KEY_DATA_TYPE "=" );
            m_dataType = atoi(pcNext);

            // Find number of samples
            m_nSamples = 1;
            pcNext = strstr(a_entryLine,SPECIAL_KEY_DATA_SAMPLES "=");
            if(!pcNext){pcNext += strlen( SPECIAL_KEY_DATA_SAMPLES "=" );m_nSamples = atoi(pcNext);}

        }
        break;

    case entryCreationType::fromUser:
        {
            const char *tmpStr,*pcNext = strstr(a_entryLine,SPECIAL_KEY_DOOCS "=");
            if(!pcNext){throw errorsFromConstructor::syntax;}
            pcNext += strlen(SPECIAL_KEY_DOOCS "=");
            tmpStr = strpbrk(pcNext,POSIIBLE_TERM_SYMBOLS);
            //strspn();// later ignore empty
            if(!tmpStr){unStrLen = strlen(pcNext);}
            else{unStrLen = static_cast<size_t>(tmpStr - pcNext);}
            m_doocsUrl = static_cast<char*>(malloc(unStrLen+1));
            if(!m_doocsUrl){throw errorsFromConstructor::lowMemory;}
            memcpy(m_doocsUrl,pcNext,unStrLen);
            m_doocsUrl[unStrLen]=0;

            // Find data type
            m_dataType = DATA_TYPE_TAKE_FR_DOOCS;
            pcNext = strstr(a_entryLine,SPECIAL_KEY_DATA_TYPE "=");
            if(pcNext){pcNext += strlen( SPECIAL_KEY_DATA_TYPE "=" ); m_dataType = atoi(pcNext);}

            // Find number of samples
            m_nSamples = 1;
            pcNext = strstr(a_entryLine,SPECIAL_KEY_DATA_SAMPLES "=");
            if(pcNext){pcNext += strlen( SPECIAL_KEY_DATA_SAMPLES "=" );m_nSamples = atoi(pcNext);}
        }
        break;

    default:
        throw errorsFromConstructor::type;
        //break;
    }

    if(!m_doocsUrl){throw errorsFromConstructor::lowMemory;}

}


pitz::daq::SingleEntryRR::~SingleEntryRR()
{
    free(m_rootFormatStr);
    free(m_doocsUrl);
}


#if 0
void pitz::daq::SingleEntryRR::FromDoocsToMemory(data::memory::ForServerBase* a_pMemory,const EqData* a_dcsData)
{
    switch ( m_dataType % 100 )
    {
    case  1 :
    {
        data::memory::M01* pMem = (data::memory::M01*)a_pMemory;
        pMem->value() = a_dcsData->get_int();
    }
    break;
    case  2 :
    {
        data::memory::M02* pMem = (data::memory::M02*)a_pMemory;
        pMem->value() = a_dcsData->get_float();
    }
    break;
    case  3 :
    {
        data::memory::M03* pMem = (data::memory::M03*)a_pMemory;
        sprintf(pMem->value(),"%s",a_dcsData->get_char_array());
    }
    break;
    case  4 :
    {
        data::memory::M01* pMem = (data::memory::M01*)a_pMemory;
        pMem->value() = a_dcsData->get_int();
    }
    break;
    case  6 :
    {
        data::memory::M02* pMem = (data::memory::M02*)a_pMemory;
        pMem->value() = (float)a_dcsData->get_double();
    }
    break;
    case DATA_IIII /*14*/:
    {
        IIII*	  POLYPARA;
        data::memory::M15* pMem = (data::memory::M15*)a_pMemory;
        POLYPARA = a_dcsData->get_iiii();
        if(POLYPARA){
            sprintf(pMem->value(),"%d  %d  %d  %d",
                POLYPARA->i1_data,POLYPARA->i2_data,POLYPARA->i3_data,POLYPARA->i4_data);
        }

    }
    case 15 :
    {
        IFFF*	  POLYPARA;
        data::memory::M15* pMem = (data::memory::M15*)a_pMemory;
        POLYPARA = a_dcsData->get_ifff();
        if(POLYPARA){
            sprintf(pMem->value(),"%d  %e  %e  %e",POLYPARA->i1_data,POLYPARA->f1_data,POLYPARA->f2_data,POLYPARA->f3_data);
        }

    }
    break;
    case DATA_SPECTRUM /*19*/ :
    {
        data::memory::M19* pMem = (data::memory::M19*)a_pMemory;
        float* fpValue = a_dcsData->get_float_array();
        int nArrayLen = a_dcsData->array_length();
        if(nArrayLen>0){pMem->SetElements(fpValue,nArrayLen);}
    }
    break;
    default :
    break;
    }

}
#endif



const char* pitz::daq::SingleEntryRR::rootFormatString()const
{
    return m_rootFormatStr;
}


void pitz::daq::SingleEntryRR::ValueStringByKeyInherited(bool a_bReadAll, const char* a_request, char* a_buffer, int a_bufferLength)const
{
    //#define SPECIAL_KEY_DOOCS "doocs"
    //#define SPECIAL_KEY_DATA_TYPE "type"
    //#define SPECIAL_KEY_DATA_SAMPLES "samples"

    char* pcBufToWrite(a_buffer);
    int nWritten,nBufLen(a_bufferLength);

    if(a_bReadAll ||strstr(SPECIAL_KEY_DOOCS,a_request)){
        nWritten = snprintf(pcBufToWrite,static_cast<size_t>(nBufLen),SPECIAL_KEY_DOOCS "=%s; ",m_doocsUrl);
        pcBufToWrite += nWritten;
        nBufLen -= nWritten;
        if(nBufLen<=0){return;}
    }

    if(a_bReadAll ||strstr(SPECIAL_KEY_DATA_TYPE,a_request)){
        nWritten = snprintf(pcBufToWrite,static_cast<size_t>(nBufLen),SPECIAL_KEY_DATA_TYPE "=%d; ",m_dataType);
        pcBufToWrite += nWritten;
        nBufLen -= nWritten;
        if(nBufLen<=0){return;}
    }

    if(a_bReadAll ||strstr(SPECIAL_KEY_DATA_SAMPLES,a_request)){
        nWritten = snprintf(pcBufToWrite,static_cast<size_t>(nBufLen),SPECIAL_KEY_DATA_SAMPLES "=%d; ",m_nSamples);
        pcBufToWrite += nWritten;
        nBufLen -= nWritten;
        if(nBufLen<=0){return;}
    }

}


void pitz::daq::SingleEntryRR::PermanentDataIntoFile(FILE* a_fpFile)const
{
    fprintf(a_fpFile,
            SPECIAL_KEY_DOOCS "=%s; "
            SPECIAL_KEY_DATA_TYPE "=%d; "
            SPECIAL_KEY_DATA_SAMPLES "=%d; ",

            m_doocsUrl,
            m_dataType,
            m_nSamples);
}


#if 0
pitz::daq::data::memory::ForServerBase* pitz::daq::SingleEntryRR::CreateMemoryInherit()
{
    //m_dataType %= 100;

    switch ( m_dataType )
    {
    case  1 :
        copyString(&m_rootFormatStr,"time/I:buffer/I:int_value/I");
        return new data::memory::M01(this);
    break;
    case  2 :
        copyString(&m_rootFormatStr,"time/I:buffer/I:float_value/F");
        return new data::memory::M02(this);
    break;
    case  3 :
        copyString(&m_rootFormatStr,"time/I:buffer/I:char_array[60]/C");
        return new data::memory::M03(this,60);
    break;
    case  4 :
        copyString(&m_rootFormatStr,"time/I:buffer/I:int_value/I");
        return new data::memory::M01(this);
    break;
    case  6 :
        copyString(&m_rootFormatStr,"time/I:buffer/I:float_value/F");
        return new data::memory::M02(this);
    break;
    case 14 :
        copyString(&m_rootFormatStr,"time/I:buffer/I:IIII_array[60]/C");
        return new data::memory::M15(this);
    case 15 :
        copyString(&m_rootFormatStr,"time/I:buffer/I:IFFF_array[60]/C");
        return new data::memory::M15(this);
    break;
    case 19 :
        m_rootFormatStr = (char*)malloc(1024);
        snprintf(m_rootFormatStr,1023,"time/I:buffer/I:array_value[%d]/F",m_nSamples);
        return new data::memory::M19(this,m_nSamples,3*sizeof(int));
    break;
    case 119:
        m_rootFormatStr = (char*)malloc(1024);
        snprintf(m_rootFormatStr,1023,"seconds/I:gen_event/I:array_value[%d]/F",m_nSamples);
        return new data::memory::M19(this,m_nSamples,3*sizeof(int));
    case 219:
        copyString(&m_rootFormatStr,"seconds/I:gen_event/I:array_value[2048]/F");
        return new data::memory::M19(this,m_nSamples,3*sizeof(int));
    default :
    break;
    }

    return NULL;
}
#endif


/*////////////////////////////////////////////////////////////*/
static bool GetEventAndTime(int* a_event, int* a_time)
{
    if(!g_shareptr){return false;}
    int nIndexLast(0);

    for(int i(1); i<s_H_count;++i){
        if( (g_shareptr[i].seconds>g_shareptr[nIndexLast].seconds)||
           ((g_shareptr[i].seconds==g_shareptr[nIndexLast].seconds)&&(g_shareptr[i].gen_event>g_shareptr[nIndexLast].gen_event))  )
        {
            nIndexLast = i;
        }
    } // for(int i(1); i<s_H_count;++i){

    *a_event = g_shareptr[nIndexLast].gen_event;
    *a_time = g_shareptr[nIndexLast].seconds;

    return true;
}
