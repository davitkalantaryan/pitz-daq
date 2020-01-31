//
// file eq_pitznoadc0.cc
//
// pitznoadc0 Eq function class

#include "pitz_daq_singleentrydoocs.hpp"
#include <eq_client.h>
#include <new>
#include <pitz_daq_data_handling_types.h>

#define DOOCS_URI_MAX_LEN           256
#define SPECIAL_KEY_DOOCS           "doocs"
#define SPECIAL_KEY_DATA_TYPE       "type"
#define SPECIAL_KEY_DATA_SAMPLES    "samples"

#ifndef PITZ_DAQ_UNSPECIFIED_DATA_TYPE
#define PITZ_DAQ_UNSPECIFIED_DATA_TYPE  DATA_TYPE_TAKE_FR_DOOCS
#endif  // #ifndef PITZ_DAQ_UNKNOWN_DATA_TYPE

/*////////////////////////////////////////////////////*/

pitz::daq::SingleEntryDoocs::SingleEntryDoocs(entryCreationType::Type a_creationType,const char* a_entryLine, TypeConstCharPtr* a_pHelper)
        :
        SingleEntry(a_creationType, a_entryLine, a_pHelper),
        m_doocsUrl(NEWNULLPTR2),
        m_rootFormatStr(NEWNULLPTR2)
{

    size_t unStrLen ;

    switch(a_creationType)
    {
    case entryCreationType::fromOldFile:
        {
            int from, step;
            char doocs_url[DOOCS_URI_MAX_LEN],daqName[256];

            sscanf(a_entryLine,"%s %s %d %d %d %d",daqName,doocs_url,&from,&m_branchInfo.itemsCountPerEntry,&step,&m_branchInfo.dataType);

            if(m_branchInfo.itemsCountPerEntry<1){m_branchInfo.itemsCountPerEntry=1;}

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

            GetEntryInfoFromServer(&m_branchInfo);

            // Find data type
            pcNext = strstr(a_entryLine,SPECIAL_KEY_DATA_TYPE "=");
            if(pcNext){
                pcNext += strlen( SPECIAL_KEY_DATA_TYPE "=" );
                m_branchInfo.dataType = atoi(pcNext);
            }

            // Find number of samples
            //nSamples = 1;
            pcNext = strstr(a_entryLine,SPECIAL_KEY_DATA_SAMPLES "=");
            if(pcNext){
                int nSamples;
                pcNext += strlen( SPECIAL_KEY_DATA_SAMPLES "=" );
                nSamples = atoi(pcNext);
                if(nSamples>0){m_branchInfo.itemsCountPerEntry = nSamples;}
            }

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

            if(!GetEntryInfoFromServer(&m_branchInfo)){
                ::free(m_doocsUrl);
                m_doocsUrl = NEWNULLPTR2;
                throw errorsFromConstructor::doocsUnreachable;
            }

            // Find data type
            pcNext = strstr(a_entryLine,SPECIAL_KEY_DATA_TYPE "=");
            if(pcNext){
                pcNext += strlen( SPECIAL_KEY_DATA_TYPE "=" );
                m_branchInfo.dataType = atoi(pcNext);
            }

            // Find number of samples
            pcNext = strstr(a_entryLine,SPECIAL_KEY_DATA_SAMPLES "=");
            if(pcNext){
                int nSamples;
                pcNext += strlen( SPECIAL_KEY_DATA_SAMPLES "=" );
                nSamples = atoi(pcNext);
                if(nSamples>0){m_branchInfo.itemsCountPerEntry = nSamples;}
            }

        }
        break;

    default:
        throw errorsFromConstructor::type;
    }


    if( (m_branchInfo.dataType == PITZ_DAQ_UNSPECIFIED_DATA_TYPE)||(m_branchInfo.itemsCountPerEntry<1) ){
        throw ::std::bad_alloc();
    }

    m_rootFormatStr = PrepareDaqEntryBasedOnType(1,&m_branchInfo,&m_unOnlyDataBufferSize,&m_unTotalRootBufferSize,NEWNULLPTR2,NEWNULLPTR2,NEWNULLPTR2);

    if(!m_rootFormatStr){
        throw ::std::bad_alloc();
    }

}


pitz::daq::SingleEntryDoocs::~SingleEntryDoocs()
{
    free(m_rootFormatStr);
    free(m_doocsUrl);
}


void pitz::daq::SingleEntryDoocs::FromDoocsToMemory(DEC_OUT_PD(SingleData)* a_pMemory,const EqData* a_dcsData)
{
    switch ( m_branchInfo.dataType % 100 )
    {
    case  1 :
    {
        //data::memory::M01* pMem = (data::memory::M01*)a_pMemory;
        //pMem->value() = a_dcsData->get_int();
        *wrPitzDaqDataFromEntryT(int*,a_pMemory) = a_dcsData->get_int();
    }
    break;
    case  2 :
    {
        //data::memory::M02* pMem = (data::memory::M02*)a_pMemory;
        //pMem->value() = a_dcsData->get_float();
        *wrPitzDaqDataFromEntryT(float*,a_pMemory) = a_dcsData->get_float();
    }
    break;
    case  3 :
    {
        //data::memory::M03* pMem = (data::memory::M03*)a_pMemory;
        //sprintf(pMem->value(),"%s",a_dcsData->get_char_array());
        sprintf(wrPitzDaqDataFromEntryT(char*,a_pMemory),"%s",a_dcsData->get_char_array());
    }
    break;
    case  4 :
    {
        //data::memory::M01* pMem = (data::memory::M01*)a_pMemory;
        //pMem->value() = a_dcsData->get_int();
        *wrPitzDaqDataFromEntryT(int*,a_pMemory) = a_dcsData->get_int();
    }
    break;
    case  6 :
    {
        //data::memory::M02* pMem = (data::memory::M02*)a_pMemory;
        //pMem->value() = (float)a_dcsData->get_double();
        *wrPitzDaqDataFromEntryT(float*,a_pMemory) = static_cast<float>(a_dcsData->get_double());
    }
    break;
    case DATA_IIII /*14*/:
    {
        IIII*	  POLYPARA;
        //data::memory::M15* pMem = (data::memory::M15*)a_pMemory;
        POLYPARA = a_dcsData->get_iiii();
        if(POLYPARA){
            //sprintf(pMem->value(),"%d  %d  %d  %d",POLYPARA->i1_data,POLYPARA->i2_data,POLYPARA->i3_data,POLYPARA->i4_data);
            sprintf(wrPitzDaqDataFromEntryT(char*,a_pMemory),"%d  %d  %d  %d",POLYPARA->i1_data,POLYPARA->i2_data,POLYPARA->i3_data,POLYPARA->i4_data);
        }

    }
        break;
    case 15 :
    {
        IFFF*	  POLYPARA;
        //data::memory::M15* pMem = (data::memory::M15*)a_pMemory;
        POLYPARA = a_dcsData->get_ifff();
        if(POLYPARA){
            //sprintf(pMem->value(),"%d  %e  %e  %e",POLYPARA->i1_data,POLYPARA->f1_data,POLYPARA->f2_data,POLYPARA->f3_data);
            sprintf(wrPitzDaqDataFromEntryT(char*,a_pMemory),"%d  %e  %e  %e",
                    POLYPARA->i1_data,static_cast<double>(POLYPARA->f1_data),static_cast<double>(POLYPARA->f2_data),static_cast<double>(POLYPARA->f3_data));
        }

    }
    break;
    case DATA_SPECTRUM /*19*/ :
    {
        //data::memory::M19* pMem = (data::memory::M19*)a_pMemory;
        int nArrayLen = a_dcsData->array_length();
        if((nArrayLen>0)&&this->m_branchInfo.itemsCountPerEntry){
            const float* fpValue = a_dcsData->get_float_array();
            //pMem->SetElements(fpValue,nArrayLen);
            nArrayLen = (nArrayLen>this->m_branchInfo.itemsCountPerEntry) ? this->m_branchInfo.itemsCountPerEntry : nArrayLen;
            memcpy(wrPitzDaqDataFromEntry(a_pMemory),fpValue,static_cast<size_t>(nArrayLen)*sizeof(float));
        }
    }
    break;
    default :
    break;
    }

}


bool pitz::daq::SingleEntryDoocs::GetEntryInfoFromServer( DEC_OUT_PD(BranchDataRaw)* a_pEntryInfo )const
{
    int nReturn;
    EqCall eqCall;
    EqData dataIn, dataOut;
    EqAdr eqAddr;

    eqAddr.adr(m_doocsUrl);
    nReturn = eqCall.get(&eqAddr,&dataIn,&dataOut);

    if(nReturn){
        a_pEntryInfo->dataType = DATA_INT;
        a_pEntryInfo->itemsCountPerEntry = 1;
        return false;
    }

    a_pEntryInfo->dataType = dataOut.type();
    a_pEntryInfo->itemsCountPerEntry = dataOut.length();

    return true;
}


const char* pitz::daq::SingleEntryDoocs::rootFormatString()const
{
    return m_rootFormatStr;
}


DEC_OUT_PD(SingleData)* pitz::daq::SingleEntryDoocs::GetNewMemoryForNetwork2()
{
    return static_cast<DEC_OUT_PD(SingleData)*>(malloc(m_unTotalRootBufferSize+16));
}


#if 0
void pitz::daq::SingleEntryDoocs::ValueStringByKeyInherited(bool a_bReadAll, const char* a_request, char* a_buffer, int a_bufferLength)const
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
        nWritten = snprintf(pcBufToWrite,static_cast<size_t>(nBufLen),SPECIAL_KEY_DATA_TYPE "=%d; ",m_branchInfo.dataType);
        pcBufToWrite += nWritten;
        nBufLen -= nWritten;
        if(nBufLen<=0){return;}
    }

    if(a_bReadAll ||strstr(SPECIAL_KEY_DATA_SAMPLES,a_request)){
        nWritten = snprintf(pcBufToWrite,static_cast<size_t>(nBufLen),SPECIAL_KEY_DATA_SAMPLES "=%d; ",m_branchInfo.itemsCountPerEntry);
        pcBufToWrite += nWritten;
        nBufLen -= nWritten;
        if(nBufLen<=0){return;}
    }

}


void pitz::daq::SingleEntryDoocs::PermanentDataIntoFile(FILE* a_fpFile)const
{
    fprintf(a_fpFile,
            SPECIAL_KEY_DOOCS "=%s; "
            SPECIAL_KEY_DATA_TYPE "=%d; "
            SPECIAL_KEY_DATA_SAMPLES "=%d; ",

            m_doocsUrl,
            m_branchInfo.dataType,
            m_branchInfo.itemsCountPerEntry);
}
#endif
