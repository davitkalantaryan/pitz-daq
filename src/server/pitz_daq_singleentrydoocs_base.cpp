//
// file eq_pitznoadc0.cc
//
// pitznoadc0 Eq function class

#include "pitz_daq_singleentrydoocs_base.hpp"
#include <eq_client.h>
#include <new>
#include <pitz_daq_data_handling_types.h>

#define DOOCS_URI_MAX_LEN           256
#define SPECIAL_KEY_DOOCS           "doocs"


#ifndef PITZ_DAQ_UNSPECIFIED_DATA_TYPE
#define PITZ_DAQ_UNSPECIFIED_DATA_TYPE  DATA_TYPE_TAKE_FR_DOOCS
#endif  // #ifndef PITZ_DAQ_UNKNOWN_DATA_TYPE

/*////////////////////////////////////////////////////*/

pitz::daq::SingleEntryDoocsBase::SingleEntryDoocsBase(entryCreationType::Type a_creationType,const char* a_entryLine, TypeConstCharPtr* a_pHelper)
        :
        SingleEntry(a_creationType, a_entryLine, a_pHelper),
        m_doocsUrl(SPECIAL_KEY_DOOCS),
        m_rootFormatStr(NEWNULLPTR2)
{
    //bool bCallIniter = false, bIsAddedByUser = false;
    bool bCallIniter = false;
    uint32_t singleEntrySize;
    DEC_OUT_PD(TypeAndCount)      branchInfo={-1,-1};
    EqData dataOut;

    AddNewParameterToBeg(&m_doocsUrl,false,true);

    switch(a_creationType)
    {
    case entryCreationType::fromOldFile:
        {
            int from, step;
            char doocs_url[DOOCS_URI_MAX_LEN],daqName[256];

            doocs_url[0]=0;
            sscanf(a_entryLine,"%s %s %d %d %d %d",daqName,doocs_url,&from,&branchInfo.itemsCountPerEntry,&step,&branchInfo.type);

            if(branchInfo.itemsCountPerEntry<1){branchInfo.itemsCountPerEntry=1;}

            m_doocsUrl.setValue(doocs_url);
            m_dataType.set(branchInfo.type);
            m_itemsCountPerEntry=branchInfo.itemsCountPerEntry;
        }
        break;
    case entryCreationType::fromConfigFile:
        bCallIniter = true;
        break;
    case entryCreationType::fromUser:
        bCallIniter = true;
        //bIsAddedByUser = true;
        break;
    default:
        throw errorsFromConstructor::type;
    }

    if(bCallIniter){
        //LoadFromLine(a_entryLine,true,bIsAddedByUser);
        m_doocsUrl.FindAndGetFromLine(a_entryLine);
        m_additionalData.setParentDoocsUrl(m_doocsUrl.value());
        branchInfo.type = m_dataType.value();
        branchInfo.itemsCountPerEntry = (m_itemsCountPerEntry);
    }

    if((branchInfo.type == PITZ_DAQ_UNSPECIFIED_DATA_TYPE)||(branchInfo.itemsCountPerEntry<1)){
        if(!GetEntryInfoFromDoocsServer(&dataOut,m_doocsUrl.value(),&branchInfo)){
            throw ::std::bad_alloc();
        }
    }

    m_rootFormatStr = PrepareDaqEntryBasedOnType2(1,branchInfo.type,NEWNULLPTR2,&branchInfo,&singleEntrySize,NEWNULLPTR2,NEWNULLPTR2,NEWNULLPTR2);

    if(!m_rootFormatStr){
        throw ::std::bad_alloc();
    }

    m_itemsCountPerEntry = (branchInfo.itemsCountPerEntry);

}


pitz::daq::SingleEntryDoocsBase::~SingleEntryDoocsBase()
{
    free(m_rootFormatStr);
}


const ::std::string& pitz::daq::SingleEntryDoocsBase::doocsUrl()const
{
    return m_doocsUrl.value();
}


const char* pitz::daq::SingleEntryDoocsBase::rootFormatString()const
{
    return m_rootFormatStr;
}
