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
    EqData dataOut;
	struct PrepareDaqEntryInputs in;
	struct PrepareDaqEntryOutputs out;

	memset(&in,0,sizeof(in));
	memset(&out,0,sizeof(out));

	in.dataType = PITZ_DAQ_UNSPECIFIED_DATA_TYPE;

    AddNewParameterToBeg(&m_doocsUrl,false,true);

    switch(a_creationType)
    {
    case entryCreationType::fromOldFile:
        {
            int from, step;
            char doocs_url[DOOCS_URI_MAX_LEN],daqName[256];

            doocs_url[0]=0;
			sscanf(a_entryLine,"%s %s %d %d %d %d",daqName,doocs_url,&from,&out.inOutSamples,&step,&in.dataType);

			if(out.inOutSamples<1){out.inOutSamples=1;}

            m_doocsUrl.setValue(doocs_url);
			m_dataType.set(in.dataType);
			m_samples = (out.inOutSamples);
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
		in.dataType = m_dataType.value();
		out.inOutSamples = (m_samples);
    }

	if((in.dataType == PITZ_DAQ_UNSPECIFIED_DATA_TYPE)||(out.inOutSamples<1)){
		if(!GetEntryInfoFromDoocsServer(&dataOut,m_doocsUrl.value(),&in.dataType,&out.inOutSamples)){
            throw ::std::bad_alloc();
        }
    }

	m_rootFormatStr = PrepareDaqEntryBasedOnType(&in,&out);
	m_nSingleItemSize = static_cast<int>(out.oneItemSize);

    if(!m_rootFormatStr){
        throw ::std::bad_alloc();
    }

	m_samples = (out.inOutSamples);
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
