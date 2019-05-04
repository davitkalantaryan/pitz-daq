//
// file eq_pitznoadc0.cc
//
// pitznoadc0 Eq function class

#include "pitz_daq_singleentrydoocs.hpp"

#define DOOCS_URI_MAX_LEN   256
#define SPECIAL_KEY_DOOCS "doocs"
#define SPECIAL_KEY_DATA_TYPE "type"
#define SPECIAL_KEY_DATA_SAMPLES "samples"
#define DATA_TYPE_TAKE_FR_DOOCS -1

/*////////////////////////////////////////////////////*/

pitz::daq::SingleEntryDoocs::SingleEntryDoocs(entryCreationType::Type a_creationType,const char* a_entryLine)
        :
        SingleEntry(a_creationType, a_entryLine),
        m_doocsUrl(NULL),
        m_rootFormatStr(NULL)
{

    size_t unStrLen ;

    switch(a_creationType)
    {
    case entryCreationType::fromOldFile:
        {
            int from, step;
            char doocs_url[DOOCS_URI_MAX_LEN],daqName[256];

            sscanf(a_entryLine,"%s %s %d %d %d %d",daqName,doocs_url,&from,&m_nSamples,&step,&m_dataType);

            unStrLen = strlen(doocs_url);
            m_doocsUrl = (char*)malloc(unStrLen + 1);
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
            unStrLen = tmpStr - pcNext;
            m_doocsUrl = (char*)malloc(unStrLen+1);
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
            else{unStrLen = tmpStr - pcNext;}
            m_doocsUrl = (char*)malloc(unStrLen+1);
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
        break;
    }

    if(!m_doocsUrl){throw errorsFromConstructor::lowMemory;}

}


pitz::daq::SingleEntryDoocs::~SingleEntryDoocs()
{
    free(m_rootFormatStr);
    free(m_doocsUrl);
}


void pitz::daq::SingleEntryDoocs::FromDoocsToMemory(data::memory::ForServerBase* a_pMemory,const EqData* a_dcsData)
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
        break;
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



const char* pitz::daq::SingleEntryDoocs::rootFormatString()const
{
    return m_rootFormatStr;
}


void pitz::daq::SingleEntryDoocs::ValueStringByKeyInherited(bool a_bReadAll, const char* a_request, char* a_buffer, int a_bufferLength)const
{
    //#define SPECIAL_KEY_DOOCS "doocs"
    //#define SPECIAL_KEY_DATA_TYPE "type"
    //#define SPECIAL_KEY_DATA_SAMPLES "samples"

    char* pcBufToWrite(a_buffer);
    int nWritten,nBufLen(a_bufferLength);

    if(a_bReadAll ||strstr(SPECIAL_KEY_DOOCS,a_request)){
        nWritten = snprintf(pcBufToWrite,nBufLen,SPECIAL_KEY_DOOCS "=%s; ",m_doocsUrl);
        pcBufToWrite += nWritten;
        nBufLen -= nWritten;
        if(nBufLen<=0){return;}
    }

    if(a_bReadAll ||strstr(SPECIAL_KEY_DATA_TYPE,a_request)){
        nWritten = snprintf(pcBufToWrite,nBufLen,SPECIAL_KEY_DATA_TYPE "=%d; ",m_dataType);
        pcBufToWrite += nWritten;
        nBufLen -= nWritten;
        if(nBufLen<=0){return;}
    }

    if(a_bReadAll ||strstr(SPECIAL_KEY_DATA_SAMPLES,a_request)){
        nWritten = snprintf(pcBufToWrite,nBufLen,SPECIAL_KEY_DATA_SAMPLES "=%d; ",m_nSamples);
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
            m_dataType,
            m_nSamples);
}


pitz::daq::data::memory::ForServerBase* pitz::daq::SingleEntryDoocs::CreateMemoryInherit()
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
