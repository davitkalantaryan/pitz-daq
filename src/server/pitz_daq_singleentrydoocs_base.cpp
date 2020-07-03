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
	m_nSingleItemSize = -1;

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
		EntryParams::Base* pBase;
        //LoadFromLine(a_entryLine,true,bIsAddedByUser);
        m_doocsUrl.FindAndGetFromLine(a_entryLine);
		in.dataType = m_dataType.value();
		//in.dataType = static_cast<int>(m_dataType) ;
		out.inOutSamples = (m_samples);
		pBase = EntryParams::Base::FindAndCreateEntryParamFromLine("additionalData",a_entryLine);
		if(pBase){
			EntryParams::Vector* pAddData = dynamic_cast<EntryParams::Vector*>(pBase);
			if(pAddData){
				m_pAdditionalData = new EntryParams::AdditionalDataDoocs(pAddData,m_doocsUrl.value());
				if(m_pAdditionalData){
					AddNewParameterToEnd(m_pAdditionalData,false,true);
				}
			}
			delete pBase;
		}
    }

	if(!InitializeDoocsEntryFromServer()){
		throw ::std::bad_alloc();
	}
}


pitz::daq::SingleEntryDoocsBase::~SingleEntryDoocsBase()
{
    free(m_rootFormatStr);
}


bool pitz::daq::SingleEntryDoocsBase::InitializeDoocsEntryFromServer()
{
	char* pcReturnFromPrepare;
	int nSingleItemSize;
	EqData dataOut;
	struct PrepareDaqEntryInputs in;
	struct PrepareDaqEntryOutputs out;

	memset(&in,0,sizeof(in));
	memset(&out,0,sizeof(out));

	if(!GetEntryInfoFromDoocsServer(&dataOut,m_doocsUrl.value(),&in.dataType,&out.inOutSamples)){
		return false;
	}

	in.shouldDupString = 0;
	pcReturnFromPrepare = PrepareDaqEntryBasedOnType(&in,&out);
	if(!pcReturnFromPrepare){return false;}
	nSingleItemSize = static_cast<int>(out.oneItemSize);

	if((m_nSingleItemSize>=0)&&(m_nSingleItemSize!=nSingleItemSize)){
		printftostderr(__FUNCTION__,"Data item size mismatch (%d=>%d)",m_nSingleItemSize,nSingleItemSize);
		return false;
	}

	if(out.inOutSamples!=(m_samples)){
		in.shouldDupString = 1;
		free(m_rootFormatStr);
		m_rootFormatStr = PrepareDaqEntryBasedOnType(&in,&out);
		if(!m_rootFormatStr){
			return false;
		}
		m_samples = (out.inOutSamples);
	}

	return true;
}


void pitz::daq::SingleEntryDoocsBase::InitializeRootTree()
{
	InitializeDoocsEntryFromServer();
}


const ::std::string& pitz::daq::SingleEntryDoocsBase::doocsUrl()const
{
    return m_doocsUrl.value();
}


const char* pitz::daq::SingleEntryDoocsBase::rootFormatString()const
{
    return m_rootFormatStr;
}


/*//////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
pitz::daq::EntryParams::AdditionalDataDoocs::AdditionalDataDoocs( Vector* a_pContentToMove, const std::string& a_parentDoocsUrl)
	:
	  Vector ( a_pContentToMove ),
	  m_parentDoocsUrl(a_parentDoocsUrl)
{
	const size_t cunVectorSize( m_vectorOfEntries.size() );
	//size_t unIndexToDelete = cunVectorSize+1;
	IntParam<int>* pRateParam=nullptr;
	Doocs* pDoocsParam;

	m_rateForFill = 0;
	m_lastFillTime = 0;
	for(size_t unIndex(0); unIndex<cunVectorSize;++unIndex){
		pDoocsParam = dynamic_cast<Doocs*>(m_vectorOfEntries[unIndex]);
		if(pDoocsParam){
			pDoocsParam->SetParentDoocsAddres(a_parentDoocsUrl);
		}
		else if( strcmp(m_vectorOfEntries[unIndex]->paramName(),"rate")==0){
			pRateParam = dynamic_cast< IntParam<int>* >(m_vectorOfEntries[unIndex]);
			if(pRateParam){
				//unIndexToDelete = unIndex;
				m_rateForFill = static_cast< decltype (m_rateForFill) >( *pRateParam );
			}
		}  // if( strcmp(m_vectorOfEntries[unIndex]->paramName(),"rate")==0){

	}  // for(size_t unIndex(0); unIndex<cunVectorSize;++unIndex){

	if(m_rateForFill<3){
		m_rateForFill = 10;
	}

	//if(unIndexToDelete<cunVectorSize){
	//	m_vectorOfEntries.erase(m_vectorOfEntries.begin() + static_cast<ptrdiff_t>(unIndexToDelete));
	//	delete pRateParam;
	//}
}


pitz::daq::EntryParams::AdditionalDataDoocs::~AdditionalDataDoocs( )
{
}


//void pitz::daq::EntryParams::AdditionalDataDoocs::Fill()
//{
//	//
//}

bool pitz::daq::EntryParams::AdditionalDataDoocs::timeToRefresh()const
{
	bool bRefresh = false;
	time_t timeNow;

	timeNow = time(&timeNow);

	if((timeNow-m_lastFillTime)>m_rateForFill){
		bRefresh = true;
		m_lastFillTime = timeNow;
	}

	return bRefresh;
}


//void pitz::daq::EntryParams::AdditionalDataDoocs::push_back(Base* a_newEntry)
//{
//	AddDataItem<DoocsEntryPlatform,DoocsAddressString>* pDoocsAddressItem = dynamic_cast<AddDataItem<DoocsEntryPlatform,DoocsAddressString>*>(a_newEntry);
//	if(pDoocsAddressItem){
//		pDoocsAddressItem->SetParentDoocsAddres(m_parentDoocsUrl);
//
//			//std::string doocsAddrValue = pDoocsAddressItem->value();
//			//
//			//
//			//struct PrepareDaqEntryInputs in;
//			//struct PrepareDaqEntryOutputs out;
//			//
//			//if(!m_pCore){return false;}
//			//if(m_pCore->isInited){return true;}
//			//
//			//memset(&in,0,sizeof(in));
//			//memset(&out,0,sizeof(out));
//			//
//			//ptrdiff_t nCount = ::std::count(m_pCore->doocsUrl2.begin(),m_pCore->doocsUrl2.end(),'/');
//			//
//			//if(nCount>3){return false;}
//			//
//			//::std::string fullAddr;
//			//
//			//if(nCount<3){
//			//	ptrdiff_t nParentCount = ::std::count(m_pCore->parentAndFinalDoocsUrl.begin(),m_pCore->parentAndFinalDoocsUrl.end(),'/');
//			//	const ptrdiff_t cnNumberToRecover = (3-nCount);
//			//	if(nParentCount<cnNumberToRecover){return false;}
//			//	size_t unIndex=0;
//			//	for(ptrdiff_t i(0);i<cnNumberToRecover;++i){
//			//		unIndex = m_pCore->parentAndFinalDoocsUrl.find("/",unIndex+1);
//			//	}
//			//
//			//	fullAddr = ::std::string(m_pCore->parentAndFinalDoocsUrl.c_str(),unIndex+1)+m_pCore->doocsUrl2;
//			//
//			//}
//			//else{
//			//	fullAddr = m_pCore->doocsUrl2;
//			//}
//			//
//			//if( !GetEntryInfoFromDoocsServer(&m_pCore->doocsData,fullAddr,&m_pCore->dataType,&m_pCore->samples) ){return false;}
//			//m_pCore->parentAndFinalDoocsUrl = fullAddr;
//			//
//			//in.dataType = m_pCore->dataType;
//			//in.shouldDupString = 1;
//			//out.inOutSamples = m_pCore->samples;
//			//m_pCore->rootFormatString = PrepareDaqEntryBasedOnType(&in,&out);
//			//if(!(m_pCore->rootFormatString)){
//			//	return false;
//			//}
//			//
//			//if(m_value<100){m_value=100;}
//			//
//			//m_pCore->isInited = 1;
//			//return true;
//
//
//	}
//	else{
//	}
//
//	Vector::push_back(a_newEntry);
//
//	//// attention: be sure what is below
//	//DoocsEntryPlatform* pPlatform = reinterpret_cast<DoocsEntryPlatform*>(a_newEntry);
//	//if(pPlatform->m_bIsDoocsAdress){
//	//}
//}
