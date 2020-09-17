//include "pitz_daq_singleentry.h"
// pitz_daq_singleentry.cpp
// 2017 Sep 15


#include <cstdlib>
#include "pitz_daq_singleentry.hpp"
#include <string.h>
#include <stdlib.h>
#include "pitz_daq_collectorproperties.hpp"
#include "pitz_daq_eqfctcollector.hpp"
#include <signal.h>
#include "pitz_daq_singleentry.cpp.hpp"
#include <eq_data.h>
#include <eq_client.h>
#include <pitz_daq_data_daqdev_common.h>
#include <pitz_daq_data_collector_getter_common.h>
#include <algorithm>
#include <iostream>
#include <pitz_daq_collector_global.h>

#define DATA_SIZE_TO_SAVE   50000  // 40 kB
#define MIN_NUMBER_OF_FILLS 20


namespace pitz{ namespace daq{

static size_t EPOCH_TO_STRING(const time_t& a_epoch, char* a_buffer, size_t a_bufferLength);
static time_t STRING_TO_EPOCH(const char* a_string);

typedef char* TypeCharPtr;

static ::std::string GetPropertyName(const char* a_entryLine, TypeConstCharPtr* a_pcLineStart, TypeCharPtr* a_pDaqName)
{
    char vcBuffer[512];
    size_t daqNameLen;

    *a_pcLineStart = strpbrk(a_entryLine,POSIIBLE_TERM_SYMBOLS);

    if(!(*a_pcLineStart)){throw errorsFromConstructor::syntax;}
    daqNameLen = static_cast<size_t>((*a_pcLineStart)-a_entryLine);

    *a_pDaqName = static_cast<char*>(malloc(daqNameLen+1));
    if(!(*a_pDaqName)){throw errorsFromConstructor::lowMemory;}
    memcpy(*a_pDaqName,a_entryLine,daqNameLen);
    (*a_pDaqName)[daqNameLen] = 0;
    snprintf(vcBuffer,511,"_ENTRY.%s",(*a_pDaqName));
    return vcBuffer;
}

}}  // namespace pitz{ namespace daq{


/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/



/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

pitz::daq::SingleEntry::SingleEntry(EqFctCollector* a_pParent,/*DEC_OUT_PD(BOOL2) a_bDubRootString,*/ entryCreationType::Type a_creationType,const char* a_entryLine, TypeConstCharPtr* a_pHelper)
        :
        D_BASE_FOR_STR(GetPropertyName(a_entryLine,a_pHelper,&m_daqName),NEWNULLPTR2),
		//SingleEntryBase(),
        m_numberInCurrentFile("entriesInCurFile"),
        m_entriesNumberInAllFiles("entriesInAllFiles"),
        m_numberOfAllFiles("allFilesNumber"),
        m_expirationTime("expiration"),
        m_creationTime("creation"),
        m_collectionMask("maskCollection"),
        m_errorMask("maskErrors"),
        m_errorWithString("error"),
		m_pAdditionalData(NEWNULLPTR2),
        m_dataType("type"),
		m_samples("samples")
{
	struct PrepareDaqEntryInputs in;
	struct PrepareDaqEntryOutputs out;
    bool bCallIniter = false, bIsAddedByUser = false;
    time_t tmCurrentTime;
    TypeConstCharPtr& pLine = *a_pHelper;
    size_t strStart;
    int nError(errorsFromConstructor::noError);
	
	m_pParent = a_pParent;

    if(!pLine){throw errorsFromConstructor::syntax;}
    m_bitwise64Reserved = 0;
	m_isLoadedFromLine = 0;
	m_isPresentInLastFile = 0;
	m_bitwise64Reserved = 0;

	m_initialEntryLine = a_entryLine;

    AddNewParameterToEnd(m_dataType.thisPtr(),false,true);
	AddNewParameterToEnd(&m_samples,false,false);
    AddNewParameterToEnd(&m_numberInCurrentFile,false,false);
    AddNewParameterToEnd(&m_entriesNumberInAllFiles,false,true);
    AddNewParameterToEnd(&m_numberOfAllFiles,false,true);
    AddNewParameterToEnd(&m_expirationTime,true,true);
    AddNewParameterToEnd(&m_creationTime,false,true);
    AddNewParameterToEnd(&m_collectionMask,true,true);
    AddNewParameterToEnd(&m_errorMask,true,true);
    AddNewParameterToEnd(m_errorWithString.thisPtr(),false,false);
	//AddNewParameterToEnd(&m_additionalData,false,true);

    m_errorMask.SetParentAndClbk(this,[](EntryParams::Base* a_pErrMask, void* a_pThis){
        EntryParams::Mask* pErrorMask = static_cast<EntryParams::Mask*>(a_pErrMask);
        if(pErrorMask->isMasked()){
            static_cast<SingleEntry*>(a_pThis)->ResetAllErrors();
        }
    });

    // the story is following
    // everihhing, that is not nullable is set before the member m_nReserved
    memset(&m_firstHeader,0,static_cast<size_t>(reinterpret_cast<char*>(&m_nReserved2)-reinterpret_cast<char*>(&m_firstHeader)));
    //m_branchInfo = {PITZ_DAQ_UNSPECIFIED_DATA_TYPE,PITZ_DAQ_UNSPECIFIED_NUMBER_OF_SAMPLES};

    m_expirationTime.setDateSeconds(NON_EXPIRE_TIME);

    strStart = strspn (pLine,POSIIBLE_TERM_SYMBOLS);
    if(pLine[strStart]==0){nError = errorsFromConstructor::syntax;goto reurnPoint;}
    pLine += strStart;

    m_creationTime.setDateSeconds(time(&tmCurrentTime));
    m_numberInCurrentFile=(0);

    switch (a_creationType)
    {
    case entryCreationType::fromOldFile:
        break;
    case entryCreationType::fromConfigFile:
        bCallIniter = true;
        break;
    case entryCreationType::fromUser:
        bCallIniter = true;
        bIsAddedByUser = true;
        break;
    default:
        break;
    }

    if(bCallIniter){
        LoadFromLine(pLine,true,bIsAddedByUser);
    }

	memset(&in,0,sizeof(in));
	memset(&out,0,sizeof(out));

	in.dataType = m_dataType.value();
	PrepareDaqEntryBasedOnType(&in,&out);
	m_nSingleItemSize = static_cast<int>(out.oneItemSize);

    return;

reurnPoint:

    if(nError != errorsFromConstructor::noError){
        // some common stuff
        if(a_creationType == entryCreationType::fromUser){
            free(m_daqName);
            throw nError;
        }
    }

}


pitz::daq::SingleEntry::~SingleEntry()
{
	delete m_pAdditionalData;
	m_pAdditionalData = nullptr;
    free(this->m_daqName);
}


void pitz::daq::SingleEntry::LoadFromLine(const char* a_entryLine, bool a_isIniting, bool a_isInitingByUserSet)
{
    if(a_isIniting){
        for( auto pParam : m_allParams){
            pParam->FindAndGetFromLine(a_entryLine);
        }
        if(a_isInitingByUserSet){
            time_t tmCurrentTime;
            m_creationTime.setDateSeconds(time(&tmCurrentTime));
            m_numberInCurrentFile=(0);
            m_entriesNumberInAllFiles=(0);
            m_numberOfAllFiles = (0);
        }
    }
    else{
        for( auto pParam : m_userSetableParams){
            pParam->FindAndGetFromLine(a_entryLine);
        }
    }

}


void pitz::daq::SingleEntry::AddNewParameterToEnd(EntryParams::Base *a_newParam, bool a_isUserSetable, bool a_isPermanent)
{
    m_allParams.push_back(a_newParam);
    if(a_isUserSetable){
        m_userSetableParams.push_back(a_newParam);
    }
    if(a_isPermanent){
        m_permanentParams.push_back(a_newParam);
    }
}


void pitz::daq::SingleEntry::AddNewParameterToBeg(EntryParams::Base* a_newParam, bool a_isUserSetable, bool a_isPermanent)
{
    m_allParams.push_front(a_newParam);
    if(a_isUserSetable){
        m_userSetableParams.push_front(a_newParam);
    }
    if(a_isPermanent){
        m_permanentParams.push_front(a_newParam);
    }
}


#define VALUE_FOR_DELETE            1llu
#define ACTUAL_DELETER              static_cast<uint64_t>(1<<4)
#define VALUE_FOR_ADD_TO_ROOT       static_cast<uint64_t>(1<<8)
#define VALUE_FOR_ADD_TO_NETW       static_cast<uint64_t>(1<<12)
#define VALUE_FOR_ADD_TO_FILE       static_cast<uint64_t>(1<<16)
#define VALUE_FOR_UNKNOWN_STATE     static_cast<uint64_t>(1<<20)
#define NON_DELETABLE_BITS          (VALUE_FOR_ADD_TO_ROOT | VALUE_FOR_ADD_TO_NETW | VALUE_FOR_ADD_TO_FILE)
#define DELETER_ALL                 (VALUE_FOR_DELETE | ACTUAL_DELETER)

bool pitz::daq::SingleEntry::markEntryForDeleteAndReturnIfPossibleNow()
{
    uint64_t nReturn = __atomic_fetch_or (&m_willBeDeletedOrIsUsedAtomic64,DELETER_ALL,__ATOMIC_RELAXED);

    if((!nReturn)||(nReturn==VALUE_FOR_DELETE)){
        return true;
    }

    // let's remove actual deleter flag
    __atomic_fetch_and (&m_willBeDeletedOrIsUsedAtomic64,~ACTUAL_DELETER,__ATOMIC_RELAXED);
    return false;
}


bool pitz::daq::SingleEntry::lockEntryForRoot()
{
    uint64_t nReturn = __atomic_fetch_or (&m_willBeDeletedOrIsUsedAtomic64,VALUE_FOR_ADD_TO_ROOT,__ATOMIC_RELAXED);

    if(!(nReturn&VALUE_FOR_DELETE)){
        return true;
    }

    __atomic_fetch_and (&m_willBeDeletedOrIsUsedAtomic64,~VALUE_FOR_ADD_TO_ROOT,__ATOMIC_RELAXED);
    return false;
}


bool pitz::daq::SingleEntry::lockEntryForNetwork()
{
    m_errorMask.isMasked();
    if(!m_collectionMask.isMasked()){
        uint64_t nReturn = __atomic_fetch_or (&m_willBeDeletedOrIsUsedAtomic64,VALUE_FOR_ADD_TO_NETW,__ATOMIC_RELAXED);

        if(!(nReturn&VALUE_FOR_DELETE)){
            return true;
        }

        __atomic_fetch_and (&m_willBeDeletedOrIsUsedAtomic64,~VALUE_FOR_ADD_TO_NETW,__ATOMIC_RELAXED);
    }

    return false;
}


bool pitz::daq::SingleEntry::lockEntryForRootFile()
{
    uint64_t nReturn = __atomic_fetch_or (&m_willBeDeletedOrIsUsedAtomic64,VALUE_FOR_ADD_TO_FILE,__ATOMIC_RELAXED);

    if(!(nReturn&VALUE_FOR_DELETE)){
        return true;
    }

    __atomic_fetch_and (&m_willBeDeletedOrIsUsedAtomic64,~VALUE_FOR_ADD_TO_FILE,__ATOMIC_RELAXED);
    return false;
}


bool pitz::daq::SingleEntry::resetRootLockAndReturnIfDeletable()
{
    uint64_t nReturn = __atomic_and_fetch (&m_willBeDeletedOrIsUsedAtomic64,~VALUE_FOR_ADD_TO_ROOT,__ATOMIC_RELAXED);

    if(nReturn == VALUE_FOR_DELETE){ return true; }

    return false;
}


bool pitz::daq::SingleEntry::resetNetworkLockAndReturnIfDeletable()
{
    uint64_t nReturn = __atomic_and_fetch (&m_willBeDeletedOrIsUsedAtomic64,~VALUE_FOR_ADD_TO_NETW,__ATOMIC_RELAXED);
    return nReturn == VALUE_FOR_DELETE ? true : false;
}


bool pitz::daq::SingleEntry::resetRooFileLockAndReturnIfDeletable()
{
    uint64_t nReturn = __atomic_and_fetch (&m_willBeDeletedOrIsUsedAtomic64,~VALUE_FOR_ADD_TO_FILE,__ATOMIC_RELAXED);
    return nReturn == VALUE_FOR_DELETE ? true : false;
}


bool pitz::daq::SingleEntry::isLockedForAnyAction()const
{
    uint64_t nReturn = __atomic_load_n (&m_willBeDeletedOrIsUsedAtomic64,__ATOMIC_RELAXED);

    if(nReturn&NON_DELETABLE_BITS){ return true; }

    return false;
}


pitz::daq::SNetworkStruct* pitz::daq::SingleEntry::networkParent()const
{
    return m_pNetworkParent;
}


uint64_t pitz::daq::SingleEntry::isDataLoaded() const
{
	return m_isDataLoaded ;
}


uint64_t pitz::daq::SingleEntry::isLoadedFromLine()
{
	if(!m_isLoadedFromLine) {
		LoadEntryFromLine();
		if(m_isLoadedFromLine) {
			DecrementError(UNABLE_TO_INITIALIZE);
		}
	}
	return m_isLoadedFromLine;
}


void pitz::daq::SingleEntry::set(EqAdr* a_dcsAddr, EqData* a_dataFromUser, EqData* a_dataToUser,EqFct* a_loc)
{
    char vcBuffer[1024];

    a_dataFromUser->get_string(vcBuffer,1023);

    for( auto pParam : m_userSetableParams){
        pParam->FindAndGetFromLine(vcBuffer);
    }

    D_BASE_FOR_STR::set(a_dcsAddr, a_dataFromUser, a_dataToUser,a_loc);
}


void pitz::daq::SingleEntry::write ( ::std::fstream &)
{
#ifdef DEBUG_APP
    ::std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! fnc:"<< __FUNCTION__ << ::std::endl;
#endif
}

#include "pitz_daq_collectorproperties.hpp"

bool pitz::daq::SingleEntry::CheckBranchExistanceAndCreateIfNecessary()
{
	if(!m_pTreeOnRoot){

		char vcBufferData[4096];
		snprintf(vcBufferData,4095,
				 DATA_HEADER_START DATA_HEADER_TYPE "%d" DATA_HEADER_FIRST_MAX_BUFF_SIZE "%d",
				 m_dataType.value(),static_cast<int>(m_samples*m_nSingleItemSize));

		m_pTreeOnRoot = new TreeForSingleEntry(this);

		m_pHeaderAndDataBranch=m_pTreeOnRoot->Branch(vcBufferData,static_cast<void*>(nullptr),this->rootFormatString());
		if(!m_pHeaderAndDataBranch){delete m_pTreeOnRoot;m_pTreeOnRoot = nullptr;IncrementError(ROOT_ERROR,"Unable to create root branch");return false;}
		if(m_pAdditionalData){m_pAdditionalData->SetRootBranch("",m_pTreeOnRoot);}
	}

	return true;
}


int64_t pitz::daq::SingleEntry::Fill( DEC_OUT_PD(Header)* a_pNewMemory/*, int a_second, int a_eventNumber*/)
{
	int64_t genEventToReturn = FillRaw(a_pNewMemory);
    FreeUsedMemory(a_pNewMemory);
	
	return genEventToReturn;
}


int64_t pitz::daq::SingleEntry::FillRaw( DEC_OUT_PD(Header)* a_pNewMemory/*, int a_second, int a_eventNumber*/)
{
	int64_t genEventToReturn(-1);
	if(!lockEntryForRoot()){return -1;}
	if(!CheckBranchExistanceAndCreateIfNecessary()){return -1;}

	// handle possible gen event errors
	// int32_t nGenEventNormalized = a_pNewMemory->gen_event%s_H_count;
	//if((a_pNewMemory->header.gen_event<0)||(a_pNewMemory->header.gen_event<m_lastHeader.gen_event)){
	//	// we have gen event error handle it
	//}
	//if(g_shareptr[nGenEventNormalized].gen_event == a_pNewMemory->gen_event){
	//	a_pNewMemory->seconds = g_shareptr[nGenEventNormalized].seconds;
	//}
	
	genEventToReturn = a_pNewMemory->gen_event;
	if(genEventToReturn<1) {
		genEventToReturn = GetEventNumberFromTime(a_pNewMemory->seconds);
		a_pNewMemory->gen_event = static_cast<decltype (a_pNewMemory->gen_event)>(genEventToReturn);
	}

	a_pNewMemory->samples = static_cast<decltype (a_pNewMemory->samples)>(m_samples);
	a_pNewMemory->branch_num_in_rcv_and_next_max_buffer_size_on_root = static_cast<decltype (a_pNewMemory->branch_num_in_rcv_and_next_max_buffer_size_on_root)>(m_nMaxBufferForNextIter);

	//if(g_shareptr[a_pNewMemory->eventNumber].gen_event)

	m_pHeaderAndDataBranch->SetAddress( a_pNewMemory );
	if(m_pAdditionalData){m_pAdditionalData->Fill(a_pNewMemory);}
	m_pTreeOnRoot->Fill();

	if(!m_isPresentInLastFile){
		m_isPresentInLastFile = 1;
		m_firstHeader = *a_pNewMemory;
		m_numberInCurrentFile = (0);
		++m_numberOfAllFiles;
	}

	++m_numberInCurrentFile;
	++m_entriesNumberInAllFiles;

	DecrementError(ROOT_ERROR);

	m_lastHeader = *a_pNewMemory;
	
	return genEventToReturn;
}


void pitz::daq::SingleEntry::writeContentToTheFile(FILE* a_fpFile)const
{
    if(!(m_willBeDeletedOrIsUsedAtomic64&DELETER_ALL)){
        //char vcBufForCrt[32],vcBufForExp[32],vcBufForMask[32];

		if(m_isLoadedFromLine){
			char vcBuffer[4096];
			char* pcBufToWrite(vcBuffer);
			size_t nBufLen(4093);
			size_t nWritten;

			for( auto pParam : m_permanentParams){
				nWritten=pParam->writeToLineBuffer(pcBufToWrite,nBufLen,';');
				if(nBufLen<=(nWritten+1)){return;}
				nBufLen -= nWritten;
				pcBufToWrite += nWritten;
			}
			pcBufToWrite[0]='\n';
			pcBufToWrite[1]=0;
			pcBufToWrite[2]=0;

			fprintf(a_fpFile,"%s %s",m_daqName,vcBuffer);
		}
		else{
			//fprintf(a_fpFile,"%s\n",m_initialEntryine.c_str());
			fwrite(m_initialEntryLine.c_str(),1,m_initialEntryLine.length(),a_fpFile);
			if(m_initialEntryLine.back()!='\n'){
				putc('\n',a_fpFile);
			}
		}
    }
}


// '\t' '\n' ' ' ',' ';'
static inline char FindDelimeterFromString(char a_cDelimeterInitial, const char* a_cpcDelim)
{
    char cDelimeter=a_cDelimeterInitial;

    size_t unDelimStrLen = strlen(a_cpcDelim);
    if(unDelimStrLen>0){
        switch(a_cpcDelim[0]){
        case '\t': case '\n': case ',': case ';' : case ' ':
            cDelimeter = a_cpcDelim[0];
            break;
        case '\\':
            if(unDelimStrLen>1){
                switch(a_cpcDelim[1]){
                case 'n':
                    cDelimeter='\n';
                    break;
                case 't':
                    cDelimeter='\t';
                    break;
                case 's':
                    cDelimeter=' ';
                    break;
                default:
                    break;
                }  // switch(cpcDelim[1]){
            }  // if(unDelimStrLen>1){
            break;
        default:
            break;
        }  // switch(cpcDelim[0]){
    }  // if(unDelimStrLen>0){

    return cDelimeter;
}


void pitz::daq::SingleEntry::get(EqAdr* /*a_dcsAddr*/, EqData* a_dataFromUser, EqData* a_dataToUser,EqFct* /*a_loc*/)
{
#define INITIAL_BUF_SIZE    4095
    char vcBuffer[INITIAL_BUF_SIZE+1];
    char vcFromUser[1024];
    char* pcBufToWrite(vcBuffer);
    size_t nWritten,nBufLen(INITIAL_BUF_SIZE);
    bool bReadAll;
    char cDelimeter='\n';
    EntryParams::String aDelimeter("delimeter");

    a_dataFromUser->get_string(vcFromUser,1023);
    if(vcFromUser[0]==0){bReadAll=true;}
    else{ bReadAll=true; }

    // '\t' '\n' ' ' ',' ';'
    if(aDelimeter.FindAndGetFromLine(vcFromUser)){
		cDelimeter = FindDelimeterFromString('\n',aDelimeter.value());
    }
    else{
        //EqFctCollector* pClc = static_cast<EqFctCollector*>(get_eqfct());
        EqFctCollector* pClc = m_pParent;
        const char* cpcDelim = pClc->m_entriesReturnDelimeter.value();
        cDelimeter = FindDelimeterFromString('\n',cpcDelim);
    }

    nWritten=0;
    if(bReadAll){
        for( auto pParam : m_allParams){
            nWritten=pParam->writeToLineBuffer(pcBufToWrite,nBufLen,cDelimeter);
            if(nBufLen<=nWritten){goto returnPoint;}
            nBufLen -= nWritten;
            pcBufToWrite += nWritten;
        }
    }
    else{
        for( auto pParam : m_allParams){
            if(strstr(vcFromUser,pParam->paramName())){
                nWritten=pParam->writeToLineBuffer(pcBufToWrite,nBufLen,cDelimeter);
                if(nBufLen<=nWritten){goto returnPoint;}
                nBufLen -= nWritten;
                pcBufToWrite += nWritten;
            }
        }
    }


    if(nBufLen>0){*pcBufToWrite=0;--nBufLen;}
    if(nBufLen>0){*pcBufToWrite=0;--nBufLen;}  // this isif we swith to unicode

returnPoint:
    a_dataToUser->set(vcBuffer);
    return;
}


void pitz::daq::SingleEntry::IncrementError(uint8_t a_errorMask, const ::std::string& a_errorString)
{
    //EqFctCollector* pClc = static_cast<EqFctCollector*>(get_eqfct());
    EqFctCollector* pClc = m_pParent;

    if(m_errorMask.isMasked()&&m_errorWithString.value()){
        if(pClc){pClc->DecrementErrors(m_daqName);}
        m_errorWithString.ResetAllErrors();
        return;
    }
    else if(!m_errorWithString.value()){
        if(pClc){pClc->IncrementErrors(m_daqName);}
    }

    m_errorWithString.AddError(a_errorMask,a_errorString);

}


void pitz::daq::SingleEntry::DecrementError(uint8_t a_errorMask)
{
    m_errorWithString.ResetError(a_errorMask);
    if(!m_errorWithString.value()){
        //EqFctCollector* pClc = static_cast<EqFctCollector*>(get_eqfct());
        EqFctCollector* pClc = m_pParent;
        if(pClc){pClc->DecrementErrors(m_daqName);}
    }
}


void pitz::daq::SingleEntry::ResetAllErrors()
{
    //EqFctCollector* pClc = static_cast<EqFctCollector*>(get_eqfct());
    EqFctCollector* pClc = m_pParent;

    if(m_errorWithString.value()){
        if(pClc){pClc->DecrementErrors(m_daqName);}
        m_errorWithString.ResetAllErrors();
        return;
    }
}


pitz::daq::NewTFile* pitz::daq::SingleEntry::GetCurrentFile() const
{
	return m_pParent->m_pRootFile;
}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

pitz::daq::SNetworkStruct::SNetworkStruct(EqFctCollector* a_parent)
{
    m_shouldRun = 1;
    m_bitwise64Reserved =0;

    m_thread = STDN::thread(&EqFctCollector::DataGetterThread,a_parent,this);
}


pitz::daq::SNetworkStruct::~SNetworkStruct()
{
    StopThreadThenDeleteAndClearEntries();
}


void pitz::daq::SNetworkStruct::StopThreadThenDeleteAndClearEntries()
{
    if(m_shouldRun){
        pthread_t handleToThread = static_cast<pthread_t>(m_thread.native_handle());

        m_shouldRun = 0;
        pthread_kill(handleToThread,SIGNAL_FOR_CANCELATION);
        m_thread.join();

        for(auto pEntry : m_daqEntries){
            delete pEntry;
        }
        m_daqEntries.clear();
    }
}


::std::list< pitz::daq::SingleEntry* >& pitz::daq::SNetworkStruct::daqEntries()/*const*/
{
    return m_daqEntries;
}


bool pitz::daq::SNetworkStruct::AddNewEntry(SingleEntry *a_newEntry)
{
    if(!a_newEntry || (a_newEntry->m_pNetworkParent)){return false;}
    m_daqEntries.push_back(a_newEntry);
    a_newEntry->m_thisIter = --m_daqEntries.end();
    a_newEntry->m_pNetworkParent = this;
    //m_pParent->add_property(a_newEntry);

    return true;
}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#include "pitz_daq_eqfctcollector.cpp.hpp"

pitz::daq::TreeForSingleEntry::TreeForSingleEntry( pitz::daq::SingleEntry* a_pParentEntry )
    :
      ::TTree(a_pParentEntry->m_daqName, "DATA"),
      m_pParentEntry(a_pParentEntry)
{
	NewTFile* pFile = a_pParentEntry->GetCurrentFile();
    m_pParentEntry->m_isPresentInLastFile = 0;
	if(pFile){
		pFile->AddNewTree(this);
	}
	else{
		fprintf(stderr,"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! No root file\n");
	}
}


pitz::daq::TreeForSingleEntry::~TreeForSingleEntry()
{
	m_pParentEntry->m_pHeaderAndDataBranch = NEWNULLPTR2;
	if(m_pParentEntry->m_pAdditionalData){m_pParentEntry->m_pAdditionalData->InitRoot();}
    m_pParentEntry->m_pTreeOnRoot = NEWNULLPTR2;
    if(m_pParentEntry->resetRootLockAndReturnIfDeletable()){
        SingleEntry* pParentEntry( m_pParentEntry );
        m_pParentEntry=NEWNULLPTR2;
        delete pParentEntry;
    }
}


void pitz::daq::TreeForSingleEntry::Finalize()
{
	m_pParentEntry->FinalizeRootTree();
}


/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

pitz::daq::RootBase::RootBase()
{
	m_pBranch = nullptr;
}


pitz::daq::RootBase::~RootBase()
{
	//
}

/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

static void DefaultClbk(pitz::daq::EntryParams::Base*,void*){}

pitz::daq::EntryParams::Base::Base( const char* a_entryParamName)
    :
	  m_paramName(strdup(a_entryParamName)),
      m_fpClbk(&DefaultClbk),
      m_pParent(nullptr)
{
}


pitz::daq::EntryParams::Base::~Base()
{
	free(m_paramName);
}


const char* pitz::daq::EntryParams::Base::paramName()const
{
    return m_paramName;
}


size_t pitz::daq::EntryParams::Base::writeToLineBuffer(char* a_entryLineBuffer, size_t a_unBufferSize, char a_cDelimeter)const
{
    size_t unStrLen;

    if(ShouldSkipProviding()){return 0;}

    unStrLen = strlen(m_paramName);

    if(LIKELY2(a_unBufferSize>(unStrLen+1))){
        size_t unDataStrLen;
        memcpy(a_entryLineBuffer,m_paramName,unStrLen);
        a_entryLineBuffer[unStrLen++]='=';
        a_unBufferSize -= unStrLen;
        a_entryLineBuffer += unStrLen;
        unDataStrLen=this->writeDataToLineBuffer(a_entryLineBuffer,a_unBufferSize);
        if(a_unBufferSize>unDataStrLen){
            a_entryLineBuffer[unDataStrLen++]=a_cDelimeter;
        }
        unStrLen += unDataStrLen;
    }
    else{
        unStrLen = a_unBufferSize;
        memcpy(a_entryLineBuffer,m_paramName,unStrLen);
    }

    return unStrLen;
}


pitz::daq::EntryParams::Base* pitz::daq::EntryParams::Base::FindAndCreateEntryParamFromLine(const char* a_paramName, const char* a_entryLine)
{
	const char* pcNext = strstr(a_entryLine,a_paramName);
	if(pcNext){
		// todo: check other cases, than only vector
		const char* cpcListStart = strchr(pcNext,'(');
		if(cpcListStart){
			const char* cpcListEnd = strchr(++cpcListStart,')');
			if(cpcListEnd){
				//::std::string strEntryLine = ::std::string(cpcListStart,static_cast<size_t>(cpcListEnd-cpcListStart));
				Vector* pReturn = new Vector(a_paramName);
				pcNext += strlen(a_paramName);
				pReturn->GetItemsFromLine(pcNext);
				return pReturn;
			}
		}
	}

	return nullptr;
}


bool pitz::daq::EntryParams::Base::FindAndGetFromLine(const char* a_entryLine)
{
    bool bReturn(false);
    const char* pcNext = strstr(a_entryLine,m_paramName);
    if(pcNext){
        pcNext = strchr(pcNext+1,'=');
        if(pcNext){
            bReturn=this->GetDataFromLine(++pcNext);
            if(bReturn){
                (*m_fpClbk)(this,m_pParent);
            }
        }
    }
    return bReturn;
}


void pitz::daq::EntryParams::Base::SetParentAndClbk(void* a_pParent, TypeClbk a_fpClbk)
{
    m_pParent = a_pParent;
    m_fpClbk = a_fpClbk;
}


void pitz::daq::EntryParams::Base::SetRootBranch(const char* a_cpcParentBranchName, TTree* a_pTreeOnRoot)
{
	const char* cpcRootFormatString = this->rootFormatString();
	if(cpcRootFormatString && cpcRootFormatString[0]){
		::std::string rootBranchName = a_cpcParentBranchName + ::std::string("_") + paramName() +
				DATA_HEADER_TYPE + ::std::to_string( this->dataType() ) + DATA_HEADER_FIRST_MAX_BUFF_SIZE + ::std::to_string( this->nextMaxMemorySize() );
		m_pBranch =a_pTreeOnRoot->Branch(rootBranchName.c_str(),static_cast<void*>(nullptr),this->rootFormatString());

		//TBranch* pParentBranch = a_pTreeOnRoot->Branch(rootBranchName.c_str(),nullptr,static_cast<char*>(nullptr)); // crash

		//TBranch* pParentBranch = a_pTreeOnRoot->Branch("addData",nullptr,"rate/I");
		//m_pBranch = new TBranch(pParentBranch,rootBranchName.c_str(),nullptr,this->rootFormatString());
	}
}


void pitz::daq::EntryParams::Base::InitRoot()
{
	m_pBranch = nullptr;
}


/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

pitz::daq::EntryParams::Error::Error(const char* a_entryParamName)
    :
	  SomeInts<uint32_t>(a_entryParamName)
{
}


void pitz::daq::EntryParams::Error::AddError(uint8_t a_errorMask, const ::std::string& a_errorString)
{
    if(a_errorMask>=m_snMaxNumber){return;}

    uint32_t unMask = 1<<a_errorMask;
    uint32_t unCurrentValue = value();

    if(!(unMask&unCurrentValue)){
        m_errorStrings[a_errorMask]=a_errorString;
        unCurrentValue |= unMask;
        m_value = unCurrentValue;
    }
}


void pitz::daq::EntryParams::Error::ResetError(uint8_t a_errorMask)
{
    if(a_errorMask>=m_snMaxNumber){return;}

    uint32_t unMask = 1<<a_errorMask;
    uint32_t unCurrentValue = value();

    if(unMask&unCurrentValue){
        uint32_t unAntiMask = ~unMask;
        unCurrentValue &= unAntiMask;
        m_value = unCurrentValue;
    }
}


void pitz::daq::EntryParams::Error::ResetAllErrors()
{
    m_value = 0;
}


::std::string pitz::daq::EntryParams::Error::additionalString() const
{
    ::std::string strToReturn;
    uint32_t unCurrentValue = value();

    if(!unCurrentValue){
        strToReturn = "ok";
    }
    else{
        for(uint8_t i(0);i<m_snMaxNumber;++i){
            if((1<<i)&unCurrentValue){
                strToReturn += m_errorStrings[i];
                strToReturn += ";";
            }
        }
    }

    return strToReturn;
}


/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

// "additionalData"

pitz::daq::EntryParams::Vector::Vector(const char* a_entryParamName)
    :
	  Base(a_entryParamName)
{
}


pitz::daq::EntryParams::Vector::Vector(Vector* a_pContentToMove)
	:
	  Base(a_pContentToMove->paramName()),
	  m_vectorOfEntries( ::std::move(a_pContentToMove->m_vectorOfEntries) )
{
}


pitz::daq::EntryParams::Vector::~Vector()
{
	for(auto pBase : m_vectorOfEntries){
		delete pBase;
	}
}


bool pitz::daq::EntryParams::Vector::ShouldSkipProviding() const
{
	//return m_value ? false : true;
	// todo: implement better way
	return m_vectorOfEntries.size()<1;
}


//void pitz::daq::EntryParams::Vector::push_back(Base* a_newEntry)
//{
//	m_vectorOfEntries.push_back(a_newEntry);
//}


void pitz::daq::EntryParams::Vector::SetRootBranch(const char* , TTree *a_pTreeOnRoot)
{
	const char* cpcParamName = paramName();
	const size_t cunAdditionalDataNumber ( m_vectorOfEntries.size() );
	size_t unIndex ;

	for(unIndex=0;unIndex<cunAdditionalDataNumber;++unIndex){
		m_vectorOfEntries[unIndex]->SetRootBranch(cpcParamName,a_pTreeOnRoot);
	}
}


//time_t time;		/* Seconds since epoch, as from `time'.  */
//unsigned short int millitm;	/* Additional milliseconds.  */

#define FTIMES_TO_MS_DIFF(_timeInit,_timeFinal) \
    ( static_cast<int64_t>(((_timeFinal).time-(_timeInit).time)*1000)+static_cast<int64_t>((_timeFinal).millitm-(_timeInit).millitm)  )


void pitz::daq::EntryParams::Vector::Fill(DEC_OUT_PD(Header)* a_pNewMemory)
{
	if(this->timeToRefresh()){
		for( auto pParam : m_vectorOfEntries){
			pParam->Refresh();
			pParam->Fill(a_pNewMemory);
		}
	}
	else{
		for( auto pParam : m_vectorOfEntries){
			pParam->Fill(a_pNewMemory);
		}
	}
}


void pitz::daq::EntryParams::Vector::InitRoot()
{
	for( auto pParam : m_vectorOfEntries){
		pParam->InitRoot();
	}
}


size_t pitz::daq::EntryParams::Vector::writeDataToLineBuffer(char* a_entryLineBuffer, size_t a_unBufferSize) const
{
	size_t unWritten, unReturn=0;
	const size_t cunArraySize(m_vectorOfEntries.size());

	if(a_unBufferSize<2){return 0;}

	a_entryLineBuffer[0]='(';
	++a_entryLineBuffer;
	--a_unBufferSize;
	unReturn = 1;

	for(size_t i=0; (a_unBufferSize>1)&&(i<cunArraySize);++i){
		unWritten = m_vectorOfEntries[i]->writeToLineBuffer(a_entryLineBuffer,(a_unBufferSize-1),';');
		a_unBufferSize -= unWritten;
		a_entryLineBuffer += unWritten;
		unReturn += unWritten;
	}

	a_entryLineBuffer[0]=')';

	return ++unReturn;

}


//void pitz::daq::EntryParams::AdditionalData::setParentDoocsUrl( const ::std::string& a_parentDoocsUrl )
//{
//    if(m_pCore){
//        m_pCore->parentAndFinalDoocsUrl = a_parentDoocsUrl;
//        InitDataStuff();
//    }
//}


//bool pitz::daq::EntryParams::AdditionalData::InitDataStuff()
//{
//	struct PrepareDaqEntryInputs in;
//	struct PrepareDaqEntryOutputs out;
//
//    if(!m_pCore){return false;}
//    if(m_pCore->isInited){return true;}
//
//	memset(&in,0,sizeof(in));
//	memset(&out,0,sizeof(out));
//
//    ptrdiff_t nCount = ::std::count(m_pCore->doocsUrl2.begin(),m_pCore->doocsUrl2.end(),'/');
//
//    if(nCount>3){return false;}
//
//    ::std::string fullAddr;
//
//    if(nCount<3){
//        ptrdiff_t nParentCount = ::std::count(m_pCore->parentAndFinalDoocsUrl.begin(),m_pCore->parentAndFinalDoocsUrl.end(),'/');
//        const ptrdiff_t cnNumberToRecover = (3-nCount);
//        if(nParentCount<cnNumberToRecover){return false;}
//        size_t unIndex=0;
//        for(ptrdiff_t i(0);i<cnNumberToRecover;++i){
//            unIndex = m_pCore->parentAndFinalDoocsUrl.find("/",unIndex+1);
//        }
//
//        fullAddr = ::std::string(m_pCore->parentAndFinalDoocsUrl.c_str(),unIndex+1)+m_pCore->doocsUrl2;
//
//    }
//    else{
//        fullAddr = m_pCore->doocsUrl2;
//    }
//
//	if( !GetEntryInfoFromDoocsServer(&m_pCore->doocsData,fullAddr,&m_pCore->dataType,&m_pCore->samples) ){return false;}
//    m_pCore->parentAndFinalDoocsUrl = fullAddr;
//
//	in.dataType = m_pCore->dataType;
//	in.shouldDupString = 1;
//	out.inOutSamples = m_pCore->samples;
//	m_pCore->rootFormatString = PrepareDaqEntryBasedOnType(&in,&out);
//    if(!(m_pCore->rootFormatString)){
//        return false;
//    }
//
//    if(m_value<100){m_value=100;}
//
//    m_pCore->isInited = 1;
//    return true;
//}


//::std::string pitz::daq::EntryParams::AdditionalData::additionalString()const
//{
//    if((m_value<1)||(!m_pCore)){return "no";}
//
//    return m_pCore->doocsUrl2;
//}


bool pitz::daq::EntryParams::Vector::GetComponentsLine( const char* a_entryLine, ::std::string* a_pComponentsLine)
{
	const char* cpcBegin = strchr(a_entryLine,'(');
	const char* cpcEnd;
	::std::string& innerLine = *a_pComponentsLine;

	if(!cpcBegin){return false;}
	cpcEnd = strchr(++cpcBegin,')');
	if(!cpcEnd){return false;}

	innerLine = ::std::string(cpcBegin,static_cast<size_t>(cpcEnd-cpcBegin));
	return innerLine.size()>0;
}

static inline void ltrim(std::string* a_pStr)
{
	a_pStr->erase(a_pStr->begin(), std::find_if(a_pStr->begin(), a_pStr->end(), [](int ch) {
		return !std::isspace(ch);
	}));
}


void pitz::daq::EntryParams::Vector::GetItemsFromLine(const char* a_entryLine)
{
	::std::string innerLine;
	::std::string propNameStr;
	const char *cpcLineToScan, *cpcTmp;
	bool bContinue=true;
	Base* pNext;
	int nNumber;
	if(!GetComponentsLine(a_entryLine,&innerLine)){
		return ;
	}

	ltrim(&innerLine);

	cpcLineToScan = innerLine.c_str();
	cpcTmp = strchr(cpcLineToScan,'=');

	while(cpcTmp && bContinue){
		propNameStr = ::std::string(cpcLineToScan,static_cast<size_t>(cpcTmp-cpcLineToScan));

		if((!(*(++cpcTmp)))/*||(!(*(++cpcTmp)))*/){
			break;
		}

		cpcLineToScan=strchr(cpcTmp,';');
		if(!cpcLineToScan){bContinue=false;}

		pNext = nullptr;
		if(strncmp(cpcTmp,"doocs:",6)==0){
			pNext = new Doocs(propNameStr.c_str());
			if(bContinue){static_cast<String*>(pNext)->setValue( ::std::string(cpcTmp+6,static_cast<size_t>(cpcLineToScan-cpcTmp-6)) );}
			else{static_cast<String*>(pNext)->setValue( cpcTmp+6 );}
		}
		else{
			if(isdigit(*cpcTmp)){
				nNumber = atoi(cpcTmp);
				pNext = new SomeInts<int>(propNameStr.c_str());
				*static_cast<IntParam<int>*>(pNext) = (nNumber);
			}
			else{
				pNext = new String(propNameStr.c_str());
				if(bContinue){static_cast<String*>(pNext)->setValue( ::std::string(cpcTmp,static_cast<size_t>(cpcLineToScan-cpcTmp)) );}
				else{static_cast<String*>(pNext)->setValue( cpcTmp );}
			}
		}

		if(pNext){
			m_vectorOfEntries.push_back(pNext);
		}

		if(cpcLineToScan){cpcTmp = strchr(++cpcLineToScan,'=');}

	}  // while(cpcTmp && bContinue){

}


bool pitz::daq::EntryParams::Vector::GetDataFromLine(const char* a_entryLine)
{
	const char* entryLine;
	::std::string innerLine;

	if(!GetComponentsLine(a_entryLine,&innerLine)){
		return false;
	}

	entryLine = innerLine.c_str();

	for( auto pParam : m_vectorOfEntries){
		if(!pParam->FindAndGetFromLine(entryLine)){
			return false;
		}
	}

	return true;

	//if(!SomeInts::GetDataFromLine(a_entryLine)){return false;}
	//if(m_value<1){
	//    m_value = 0;
	//    delete m_pCore;
	//    m_pCore = nullptr;
	//    return false;
	//}
	//const char* cpcStringStart = strchr(a_entryLine,'(');
	//if(!cpcStringStart){
	//    m_value = 0;
	//    delete m_pCore;
	//    m_pCore = nullptr;
	//    return false;
	//}
	//
	//const char* cpcStringEnd = strchr(cpcStringStart,')');
	//if((!cpcStringEnd) || ((cpcStringEnd-cpcStringStart)<4)){
	//    m_value = 0;
	//    delete m_pCore;
	//    m_pCore = nullptr;
	//    return false;
	//}
	//
	//if(!m_pCore){
	//    m_pCore = new Core;
	//}
	//
	//m_pCore->doocsUrl2 = ::std::string(cpcStringStart+1,static_cast<size_t>((cpcStringEnd-cpcStringStart)-1));
	//
	//return InitDataStuff();
}


/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

pitz::daq::EntryParams::DataType::DataType(const char* a_entryParamName)
    :
	  SomeInts<int32_t>(a_entryParamName)
{
}


void pitz::daq::EntryParams::DataType::set(int32_t a_type)
{
    m_value = a_type;
}


::std::string pitz::daq::EntryParams::DataType::additionalString() const
{
    EqData eqData;
    //std::string strForReturn = eqData.type_string(value());
    return eqData.type_string(static_cast<int>(value()));
}


/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

pitz::daq::EntryParams::Date::Date(const char* a_entryParamName)
    :
      Base(a_entryParamName)
{
    m_epochSeconds = time(&m_epochSeconds);
}


pitz::daq::EntryParams::Date::~Date()
{
}


bool pitz::daq::EntryParams::Date::GetDataFromLine(const char* a_entryLine)
{
    m_epochSeconds = STRING_TO_EPOCH(a_entryLine);
    return true;
}

static const size_t s_cunNonExpireStrLen = strlen(NON_EXPIRE_STRING);


size_t pitz::daq::EntryParams::Date::writeDataToLineBuffer(char* a_entryLineBuffer, size_t a_unBufferSize)const
{
    size_t unReturn(0);
    switch(m_epochSeconds){
    case NOT_MASKED_TO_TIME:
        return 0;
    case NON_EXPIRE_TIME:
        if(a_unBufferSize<5){
            return 0;
        }
        memcpy(a_entryLineBuffer,NON_EXPIRE_STRING,s_cunNonExpireStrLen);
        unReturn = 5;
        break;
    default:
        if(a_unBufferSize<(16+s_cunNonExpireStrLen)){
            return 0;
        }
        unReturn = EPOCH_TO_STRING(m_epochSeconds,a_entryLineBuffer+unReturn,a_unBufferSize-1);
        break;
    }

    return unReturn;
}


time_t pitz::daq::EntryParams::Date::dateSeconds() const
{
    return m_epochSeconds;
}


void pitz::daq::EntryParams::Date::setDateSeconds(time_t a_dateSeconds)
{
    m_epochSeconds = a_dateSeconds;
}


/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

pitz::daq::EntryParams::Mask::Mask(const char* a_entryParamName)
    :
      Date(a_entryParamName)
{
    m_epochSeconds = NOT_MASKED_TO_TIME;
}


bool pitz::daq::EntryParams::Mask::GetDataFromLine(const char* a_entryLine)
{
    m_epochSeconds = NOT_MASKED_TO_TIME;

    if(strncmp(a_entryLine,"true(",5)==0){
        Date::GetDataFromLine(a_entryLine+5);
    }

    return true;
}


size_t pitz::daq::EntryParams::Mask::writeDataToLineBuffer(char* a_entryLineBuffer, size_t a_unBufferSize)const
{
    size_t unReturn(0);
    switch(m_epochSeconds){
    case NOT_MASKED_TO_TIME:
        if(a_unBufferSize<5){
            return 0;
        }
        memcpy(a_entryLineBuffer,"false",5);
        return 5;
    case NON_EXPIRE_TIME:
        if(a_unBufferSize<11){
            return 0;
        }
        memcpy(a_entryLineBuffer,"true(never)",11);
        unReturn = 11;
        break;
    default:
        if(a_unBufferSize<(20+s_cunNonExpireStrLen)){
            return 0;
        }
        memcpy(a_entryLineBuffer,"true(",5);
        a_entryLineBuffer += 5;
        a_unBufferSize -= 5;
        unReturn = 5 + Date::writeDataToLineBuffer(a_entryLineBuffer,a_unBufferSize);
        a_entryLineBuffer[unReturn++]=')';
        break;
    }

    return unReturn;
}


bool pitz::daq::EntryParams::Mask::isMasked()
{
    time_t currentTime;
    time(&currentTime);
    if((m_epochSeconds!=NON_EXPIRE_TIME)&&(currentTime>=m_epochSeconds)){m_epochSeconds=NOT_MASKED_TO_TIME;}
    if(m_epochSeconds==NOT_MASKED_TO_TIME){return false;}
    return true;
}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#define VALUE_FROM_HEADERTR(_headerPtr)		( reinterpret_cast<char*>(_headerPtr) + sizeof( DEC_OUT_PD(Header) )  )

pitz::daq::EntryParams::String::String(const char* a_entryParamName)
    :
      Base(a_entryParamName)
{
	char* pcStringPart;
	struct PrepareDaqEntryInputs in;
	struct PrepareDaqEntryOutputs out;

	m_pHeader = nullptr;
	m_pHeader=static_cast<DEC_OUT_PD(Header)*>(ResizePitzDaqBuffer(m_pHeader,1+sizeof( DEC_OUT_PD(Header) )));
	pcStringPart = VALUE_FROM_HEADERTR(m_pHeader);
	pcStringPart[0]=0;
	m_unStrLen = 0;
	m_unNextMaxMemorySize = 1;

	m_pHeader->samples = 1;
	m_pHeader->branch_num_in_rcv_and_next_max_buffer_size_on_root = 1;

	memset(&in,0,sizeof(in));
	memset(&out,0,sizeof(out));

	in.dataType = DATA_STRING;
	m_rootFormatString = PrepareDaqEntryBasedOnType(&in,&out);
}


pitz::daq::EntryParams::String::~String()
{
	FreePitzDaqBuffer(m_pHeader);
}


bool pitz::daq::EntryParams::String::GetDataFromLine(const char* a_entryLine)
{
	char* pcStringPart;
    size_t unStrLen = strcspn(a_entryLine," \n\t;");

	m_pHeader=static_cast<DEC_OUT_PD(Header)*>(ResizePitzDaqBuffer(m_pHeader,unStrLen+1+sizeof( DEC_OUT_PD(Header) )));
	pcStringPart = VALUE_FROM_HEADERTR(m_pHeader);
	memcpy(pcStringPart,a_entryLine,unStrLen);
	pcStringPart[unStrLen]=0;
	m_unStrLen = unStrLen;
	m_unNextMaxMemorySize = unStrLen+1;

    return true;
}


size_t pitz::daq::EntryParams::String::writeDataToLineBuffer(char* a_entryLineBuffer, size_t a_unBufferSize)const
{
	if(m_unStrLen){
		size_t unStrLen = m_unStrLen;
		unStrLen = (unStrLen>a_unBufferSize)?a_unBufferSize:unStrLen;
		memcpy(a_entryLineBuffer,VALUE_FROM_HEADERTR(m_pHeader),unStrLen);
		return unStrLen;
	}
	return 0;
}


const char* pitz::daq::EntryParams::String::value()const
{
	return VALUE_FROM_HEADERTR(m_pHeader);
}


void pitz::daq::EntryParams::String::setValue(const ::std::string& a_newValue)
{
	const char* entryLine = a_newValue.c_str();
	char* pcStringPart;
	size_t unStrLen = a_newValue.length();

	m_pHeader=static_cast<DEC_OUT_PD(Header)*>(ResizePitzDaqBuffer(m_pHeader,unStrLen+1+sizeof( DEC_OUT_PD(Header) )));
	pcStringPart = VALUE_FROM_HEADERTR(m_pHeader);
	memcpy(pcStringPart,entryLine,unStrLen);
	pcStringPart[unStrLen]=0;
	m_unStrLen = unStrLen;
	m_unNextMaxMemorySize = unStrLen+1;
}


void pitz::daq::EntryParams::String::Fill(DEC_OUT_PD(Header)* a_pHeader )
{
	m_pHeader->seconds = a_pHeader->seconds;
	m_pHeader->gen_event = a_pHeader->gen_event;
	m_pHeader->branch_num_in_rcv_and_next_max_buffer_size_on_root = static_cast<int32_t>(m_unNextMaxMemorySize);
	m_pBranch->SetAddress(m_pHeader);
}


const char* pitz::daq::EntryParams::String::rootFormatString()const
{
	//return "data/C";
	return m_rootFormatString;
}


int	pitz::daq::EntryParams::String::dataType() const
{
	return DATA_STRING;
}


int	pitz::daq::EntryParams::String::nextMaxMemorySize() const
{
	return static_cast<int>(m_unNextMaxMemorySize);
}


/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////*/


pitz::daq::EntryParams::Doocs::Doocs(const char* a_entryParamName)
	:
	  String(a_entryParamName)
{
	m_rootFormatStr = nullptr;
	m_dataType = DATA_NULL;
	m_samples = 0;
	//DEC_OUT_PD(Header)*		m_pFillData;
	//uint64_t				m_deleteFillData : 1;
	//uint64_t				m_reserved64bit01 : 63;
	m_pFillData = nullptr;
	m_deleteFillData = 0;
	m_reserved64bit01 =0;
	m_expectedDataLength = 0;
	m_reserved1 = 0;
}


pitz::daq::EntryParams::Doocs::~Doocs()
{
	if(m_deleteFillData){
		FreePitzDaqBuffer(m_pFillData);
	}
	free(m_rootFormatStr);
}


void pitz::daq::EntryParams::Doocs::Initialize()
{
	if((m_unStrLen>1)&&(m_parentDoocsAddress.size()>0)){
		::std::string aString = VALUE_FROM_HEADERTR(m_pHeader);
		const char* cpcRootFormatString;
		size_t unIndex=0;
		::std::string strLastPart;
		ptrdiff_t nCount;
		ptrdiff_t nParentCount = ::std::count(m_parentDoocsAddress.begin(),m_parentDoocsAddress.end(),'/');
		ptrdiff_t i, nNumberToFindInParent;
		struct PrepareDaqEntryInputs in;
		struct PrepareDaqEntryOutputs out;

		memset(&in,0,sizeof(in));
		memset(&out,0,sizeof(out));

		if(aString.at(0)!='/'){strLastPart=aString;}
		else{strLastPart=aString.c_str()+1;}
		nCount = ::std::count(strLastPart.begin(),strLastPart.end(),'/');

		if((nCount>=3)||((nCount+nParentCount)<2)){
			return;
		}

		nNumberToFindInParent = 3-nCount;
		for(i=0;i<nNumberToFindInParent;++i){
			unIndex = m_parentDoocsAddress.find("/",unIndex+1);
		}

		m_doocsAddress = ::std::string(m_parentDoocsAddress.c_str(),unIndex);
		m_doocsAddress.push_back('/');
		m_doocsAddress += strLastPart;

		m_data.init();
		GetEntryInfoFromDoocsServer(&m_data,m_doocsAddress,&in.dataType,&out.inOutSamples,nullptr);
		m_dataType = in.dataType;

		cpcRootFormatString = PrepareDaqEntryBasedOnType(&in,&out);
		if(cpcRootFormatString){
			m_nSingleItemSize = static_cast< decltype (m_nSingleItemSize) >(out.oneItemSize);
			m_samples = out.inOutSamples;
			m_nMaxBufferForNextIter = m_expectedDataLength = static_cast< decltype (m_expectedDataLength) >(m_nSingleItemSize*m_samples);
			if(!m_rootFormatStr){
				m_rootFormatStr = strdup(cpcRootFormatString);
			}
		}
	}
}


const char* pitz::daq::EntryParams::Doocs::rootFormatString()const
{
	return m_rootFormatStr ? m_rootFormatStr : "";
}


void pitz::daq::EntryParams::Doocs::Fill(DEC_OUT_PD(Header)* a_pNewMemory)
{
	if(m_pFillData){
		*m_pFillData = *a_pNewMemory;
		m_pFillData->samples = m_samples;
		m_pFillData->branch_num_in_rcv_and_next_max_buffer_size_on_root = m_nMaxBufferForNextIter;
		m_pBranch->SetAddress( m_pFillData );
		//m_pBranch->Fill(); // troot fill will will all
	}
}


int	pitz::daq::EntryParams::Doocs::dataType() const
{
	return m_dataType;
}


int	pitz::daq::EntryParams::Doocs::nextMaxMemorySize() const
{
	return m_nMaxBufferForNextIter;
}


bool pitz::daq::EntryParams::Doocs::GetDataFromLine(const char* a_entryLine)
{
	size_t unStrLen = strcspn(a_entryLine," \n\t;");
	::std::string stringInit = ::std::string(a_entryLine,unStrLen);

	if(stringInit.find("doocs:",0)==0){
		String::setValue(stringInit.c_str()+6);
		Initialize();
		return true;
	}

	return false;
}


size_t pitz::daq::EntryParams::Doocs::writeDataToLineBuffer(char* a_entryLineBuffer, size_t a_unBufferSize)const
{
	if(a_unBufferSize>6){
		memcpy(a_entryLineBuffer,"doocs:",6);
		a_entryLineBuffer += 6;
		a_unBufferSize -= 6;
		size_t unStrLen = m_unStrLen;
		unStrLen = (unStrLen>a_unBufferSize)?a_unBufferSize:unStrLen;
		memcpy(a_entryLineBuffer,VALUE_FROM_HEADERTR(m_pHeader),unStrLen);
		return unStrLen+6;
	}

	return 0;
}


void pitz::daq::EntryParams::Doocs::SetParentDoocsAddres(const ::std::string& a_parent)
{
	m_parentDoocsAddress = a_parent;
	Initialize();
}


void pitz::daq::EntryParams::Doocs::Refresh()
{
	EqData dataIn, dataOut;
	EqAdr doocsAdr;
	EqCall doocsCaller;
	bool bShouldDeletePointer=false;
	struct PrepareDaqEntryInputs in;
	struct PrepareDaqEntryOutputs out;

	memset(&in,0,sizeof(in));
	memset(&out,0,sizeof(out));

	if(m_nMaxBufferForNextIter!=static_cast<int>(m_expectedDataLength)){
		m_expectedDataLength = static_cast<decltype (m_expectedDataLength)>(m_nMaxBufferForNextIter);
		m_samples = m_nMaxBufferForNextIter/m_nSingleItemSize;
	}

	doocsAdr.adr(m_doocsAddress);
	if(doocsCaller.get(&doocsAdr,&dataIn,&dataOut)){
		::std::string errorString = dataOut.get_string();
		::std::cerr << "error for DOOCS address:"<<m_doocsAddress<<".error is: "<<errorString << ::std::endl;
		return;
	}

	m_data.init();
	m_data.copy_from(&dataOut);

	if(m_deleteFillData){
		FreePitzDaqBuffer(m_pFillData);
		m_deleteFillData = 0;
	}
	m_pFillData = nullptr;

	GetEntryInfoFromDoocsServer(&m_data,m_doocsAddress,&in.dataType,&out.inOutSamples,nullptr);
	if(PrepareDaqEntryBasedOnType(&in,&out)){
		if(m_samples!=out.inOutSamples){
			m_nMaxBufferForNextIter = out.inOutSamples*static_cast<int>(out.oneItemSize);
			if(m_expectedDataLength<static_cast<decltype (m_expectedDataLength)>(m_nMaxBufferForNextIter)){
				m_expectedDataLength = static_cast<decltype (m_expectedDataLength)>(m_nMaxBufferForNextIter);
				m_samples = m_nMaxBufferForNextIter/m_nSingleItemSize;
			}
		}
	}

	m_pFillData = GetDataPointerFromEqData(m_expectedDataLength,&m_data,&bShouldDeletePointer);
	m_deleteFillData = bShouldDeletePointer?1:0;
}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

namespace pitz{ namespace daq{

static time_t STRING_TO_EPOCH(const char* a_string)
{
    char *pcNext;
    const char *pcTmp;
    struct tm aTm;

    if(strncmp(a_string,NON_EXPIRE_STRING,s_cunNonExpireStrLen)==0){return NON_EXPIRE_TIME;}

    aTm.tm_year = static_cast<decltype (aTm.tm_year)>(strtol(a_string,&pcNext,10) - 1900);
    if((pcNext++)==a_string){return -errorsFromConstructor::syntax;}

    pcTmp = pcNext;
    aTm.tm_mon = static_cast<decltype (aTm.tm_mon)>(strtol(pcTmp,&pcNext,10) - 1);
    if((pcNext++)==pcTmp){return -errorsFromConstructor::syntax;}

    pcTmp = pcNext;
    aTm.tm_mday = static_cast<decltype (aTm.tm_mday)>(strtol(pcTmp,&pcNext,10));
    if((pcNext++)==pcTmp){return -errorsFromConstructor::syntax;}

    pcTmp = pcNext;
    aTm.tm_hour = static_cast<decltype (aTm.tm_hour)>(strtol(pcTmp,&pcNext,10) );
    if((pcNext++)==pcTmp){return -errorsFromConstructor::syntax;}

    pcTmp = pcNext;
    aTm.tm_min = static_cast<decltype (aTm.tm_min)>(strtol(pcTmp,&pcNext,10));
    if((pcNext++)==pcTmp){return -errorsFromConstructor::syntax;}

    aTm.tm_sec = 0;
    aTm.tm_isdst = -1;

    return mktime(&aTm);

}


static size_t EPOCH_TO_STRING(const time_t& a_epoch, char* a_buffer, size_t a_bufferLength)
{
    struct tm aTm;
    localtime_r(&a_epoch,&aTm);
    return static_cast<size_t>(snprintf(a_buffer,a_bufferLength,"%d.%.2d.%.2d-%.2d:%.2d",aTm.tm_year+1900,aTm.tm_mon+1,aTm.tm_mday,aTm.tm_hour,aTm.tm_min));
}


bool GetEntryInfoFromDoocsServer( EqData* a_pEqDataOut, const ::std::string& a_doocsUrl, int* a_pType, int* a_pSamples, std::string* a_pErrorString )
{
    int nReturn;
    EqCall eqCall;
    EqData dataIn;
    EqAdr eqAddr;

    eqAddr.adr(a_doocsUrl);
    nReturn = eqCall.get(&eqAddr,&dataIn,a_pEqDataOut);

    if(nReturn){
		::std::string errorString = a_pEqDataOut->get_string();
		if(a_pErrorString) {
			*a_pErrorString = errorString;
		}
		else {
			::std::cerr << "doocsAdr:"<<a_doocsUrl << ",err:"<<errorString << ::std::endl;
		}
        return false;
    }

	*a_pType = a_pEqDataOut->type();
	*a_pSamples = a_pEqDataOut->length();

    //return GetDataPointerFromEqData(a_pEqDataOut)?true:false;
    return true;
}


DEC_OUT_PD(Header)* GetDataPointerFromEqData(int32_t a_nExpectedDataLen, EqData* a_pData, bool* a_pbFreeFillData, int a_dataType)
{
	DEC_OUT_PD(Header)* pReturn=nullptr;
    EqDataBlock* pDataBlock = a_pData->data_block();
	uint64_t llnMacroPulse;
	time_t secondsFromserver, secondsLocal;

    if((!pDataBlock)||(pDataBlock->error)||(!pDataBlock->tm)){return nullptr;}

    int nDataLen = a_pData->length();
    if(nDataLen<1){return nullptr;}

	llnMacroPulse = (static_cast<uint64_t>(pDataBlock->mp_hi)<<32) | static_cast<uint64_t>(pDataBlock->mp_lo);

	if((nDataLen<2)||(a_pData->type()==DATA_IIII)||(a_pData->type()==DATA_IFFF)){
		if( HAS_HEADER( &(pDataBlock->data_u.DataUnion_u) ) ){
			pReturn = PD_HEADER( &(pDataBlock->data_u.DataUnion_u) );
			*a_pbFreeFillData = false;
		}
		else{
			DEC_OUT_PD(Header)* pReturnWithHeader = CreatePitzDaqSingleDataHeader(static_cast<size_t>(a_nExpectedDataLen));
			memcpy(wrPitzDaqDataFromHeader(pReturnWithHeader),&(pDataBlock->data_u.DataUnion_u),static_cast<size_t>(a_nExpectedDataLen));
			pReturn = pReturnWithHeader;
			*a_pbFreeFillData = true;
		}
	}
	else{
		if( HAS_HEADER(pDataBlock->data_u.DataUnion_u.d_char.d_char_val) ){
			pReturn = PD_HEADER(pDataBlock->data_u.DataUnion_u.d_char.d_char_val);
			*a_pbFreeFillData = false;
		}
		else{
			DEC_OUT_PD(Header)* pReturnWithHeader = CreatePitzDaqSingleDataHeader(static_cast<size_t>(a_nExpectedDataLen));
			if(a_dataType==DATA_SPECTRUM) {
				void* ptrServer = pDataBlock->data_u.DataUnion_u.d_spectrum.d_spect_array.d_spect_array_val;
				memcpy(wrPitzDaqDataFromHeader(pReturnWithHeader),ptrServer,static_cast<size_t>(a_nExpectedDataLen));
				pReturn = pReturnWithHeader;
				*a_pbFreeFillData = true;
			}
			else{
				memcpy(wrPitzDaqDataFromHeader(pReturnWithHeader),pDataBlock->data_u.DataUnion_u.d_char.d_char_val,static_cast<size_t>(a_nExpectedDataLen));
				pReturn = pReturnWithHeader;
				*a_pbFreeFillData = true;
			}
		}
	}
	
	secondsFromserver=pDataBlock->tm;
	secondsLocal=time(&secondsLocal);
	
	if(
			((secondsLocal>secondsFromserver)&&((secondsLocal-secondsFromserver)>4))||
			((secondsFromserver>secondsLocal)&&((secondsFromserver-secondsLocal)>4))    )  
	{
		secondsFromserver = secondsLocal;
		llnMacroPulse = 0; // let's leave to next function correcting this
	}

	pReturn->seconds = static_cast< decltype (pReturn->seconds) >(secondsFromserver);
	pReturn->gen_event = static_cast< decltype (pReturn->gen_event)>(llnMacroPulse);
	return pReturn;
}

}}

