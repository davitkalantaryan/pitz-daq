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
#include <pitz_daq_data_handling_types.h>
#include <pitz_daq_data_handling_daqdev.h>
#include <pitz_daq_data_collector_getter_common.h>
#include <algorithm>
#include <iostream>

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


pitz::daq::SingleEntry::SingleEntry(/*DEC_OUT_PD(BOOL2) a_bDubRootString,*/ entryCreationType::Type a_creationType,const char* a_entryLine, TypeConstCharPtr* a_pHelper)
        :
        D_BASE_FOR_STR(GetPropertyName(a_entryLine,a_pHelper,&m_daqName),NEWNULLPTR2),
        m_numberInCurrentFile("entriesInCurFile"),
        m_entriesNumberInAllFiles("entriesInAllFiles"),
        m_numberOfAllFiles("allFilesNumber"),
        m_expirationTime("expiration"),
        m_creationTime("creation"),
        m_collectionMask("maskCollection"),
        m_errorMask("maskErrors"),
        m_errorWithString("error"),
        m_additionalData("additionalData"),
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

    if(!pLine){throw errorsFromConstructor::syntax;}
    m_bitwise64Reserved = 0;

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
    AddNewParameterToEnd(m_additionalData.thisPtr(),false,true);

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


pitz::daq::SNetworkStruct* pitz::daq::SingleEntry::networkParent()
{
    return m_pNetworkParent;
}


uint64_t pitz::daq::SingleEntry::isDataLoaded() const
{
	return m_isDataLoaded ;
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


void pitz::daq::SingleEntry::write (fstream &)
{
#ifdef DEBUG_APP
    ::std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! fnc:"<< __FUNCTION__ << ::std::endl;
#endif
}


void pitz::daq::SingleEntry::FreeUsedMemory(DEC_OUT_PD(SingleData2)* a_usedMemory)
{
    FreePitzDaqBuffer(a_usedMemory->data);a_usedMemory->data=NEWNULLPTR2;
    FreePitzDaqBuffer(a_usedMemory->additionalDataPtr);a_usedMemory->additionalDataPtr=NEWNULLPTR2;
    FreeDataWithOffset2(a_usedMemory,0);
}

#include "pitz_daq_collectorproperties.hpp"

void pitz::daq::SingleEntry::Fill( DEC_OUT_PD(SingleData2)* a_pNewMemory/*, int a_second, int a_eventNumber*/)
{
    int32_t nGenEventNormalized = a_pNewMemory->header.eventNumber%s_H_count;
    if(!lockEntryForRoot()){return;}
    if(!m_pTreeOnRoot){

        char vcBufferData[4096];
		snprintf(vcBufferData,4095,DATA_HEADER_START DATA_HEADER_TYPE "%d",m_dataType.value());

        m_pTreeOnRoot = new TreeForSingleEntry(this);

		m_pHeaderBranch =m_pTreeOnRoot->Branch(HEADERS_HEADER,nullptr,"seconds/I:gen_event/I:samples/I");
        if(!m_pHeaderBranch){delete m_pTreeOnRoot;m_pTreeOnRoot = nullptr;IncrementError(ROOT_ERROR,"Unable to create root branch");return ;}
        m_pDataBranch=m_pTreeOnRoot->Branch(vcBufferData,nullptr,this->rootFormatString());
        if(!m_pDataBranch){delete m_pTreeOnRoot;m_pTreeOnRoot = nullptr;IncrementError(ROOT_ERROR,"Unable to create root branch");return ;}
        m_additionalData.setRootBranchIfEnabled(m_pTreeOnRoot);
    }

    // handle possible gen event errors
    if((a_pNewMemory->header.eventNumber<0)||(a_pNewMemory->header.eventNumber<m_lastHeader.timestampSeconds)){
        // we have gen event error handle it
    }
    if(g_shareptr[nGenEventNormalized].gen_event == a_pNewMemory->header.eventNumber){
        a_pNewMemory->header.timestampSeconds = g_shareptr[nGenEventNormalized].seconds;
    }


    //if(g_shareptr[a_pNewMemory->eventNumber].gen_event)

    m_pHeaderBranch->SetAddress(&(a_pNewMemory->header));
    m_pDataBranch->SetAddress( a_pNewMemory->data);
    m_additionalData.checkIfFillTimeAndFillIfYes();
    m_pTreeOnRoot->Fill();

    if(!m_isPresentInLastFile){
        m_isPresentInLastFile = 1;
        m_firstHeader = a_pNewMemory->header;
        m_numberInCurrentFile = (0);
        ++m_numberOfAllFiles;
    }

    ++m_numberInCurrentFile;
    ++m_entriesNumberInAllFiles;

    DecrementError(ROOT_ERROR);

    m_lastHeader = a_pNewMemory->header;
    FreeUsedMemory(a_pNewMemory);
}


void pitz::daq::SingleEntry::writeContentToTheFile(FILE* a_fpFile)const
{
    if(!(m_willBeDeletedOrIsUsedAtomic64&DELETER_ALL)){
        //char vcBufForCrt[32],vcBufForExp[32],vcBufForMask[32];
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
        cDelimeter = FindDelimeterFromString('\n',aDelimeter.value().c_str());
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
    NewTFile* pFile = dynamic_cast<NewTFile*>(GetCurrentFile());
    m_pParentEntry->m_isPresentInLastFile = 0;
    pFile->AddNewTree(this);
}


pitz::daq::TreeForSingleEntry::~TreeForSingleEntry()
{
    m_pParentEntry->m_pHeaderBranch = NEWNULLPTR2;
    m_pParentEntry->m_pDataBranch = NEWNULLPTR2;
    m_pParentEntry->m_additionalData.initTimeAndRoot();
    m_pParentEntry->m_pTreeOnRoot = NEWNULLPTR2;
    if(m_pParentEntry->resetRootLockAndReturnIfDeletable()){
        SingleEntry* pParentEntry( m_pParentEntry );
        m_pParentEntry=NEWNULLPTR2;
        delete pParentEntry;
    }
}


/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

static void DefaultClbk(pitz::daq::EntryParams::Base*,void*){}

pitz::daq::EntryParams::Base::Base( const char* a_entryParamName)
    :
      m_paramName(a_entryParamName),
      m_fpClbk(&DefaultClbk),
      m_pParent(nullptr)
{
}


pitz::daq::EntryParams::Base::~Base()
{
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


/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

template <typename Int32Type>
pitz::daq::EntryParams::SomeInts<Int32Type>::SomeInts(const char* a_entryParamName)
    :
	  IntParam<Int32Type>(a_entryParamName)
{
	IntParam<Int32Type>::m_value = 0;
}


template <typename Int32Type>
pitz::daq::EntryParams::SomeInts<Int32Type>::~SomeInts()
{
    //
}


template <typename Int32Type>
Int32Type pitz::daq::EntryParams::SomeInts<Int32Type>::value()const
{
	return IntParam<Int32Type>::m_value;
}


template <typename Int32Type>
pitz::daq::EntryParams::Base* pitz::daq::EntryParams::SomeInts<Int32Type>::thisPtr()
{
    return this;
}


template <typename Int32Type>
size_t pitz::daq::EntryParams::SomeInts<Int32Type>::writeDataToLineBuffer(char* a_entryLineBuffer, size_t a_unBufferSize)const
{
    ::std::string strAdditionalString = this->additionalString();
	size_t unReturn = IntParam<Int32Type>::writeDataToLineBuffer(a_entryLineBuffer,a_unBufferSize);
    size_t unStrLen = strAdditionalString.length();

    if(unStrLen){
        if((unReturn+unStrLen+2)<=a_unBufferSize){
            a_entryLineBuffer[unReturn++]='(';
            memcpy(a_entryLineBuffer+unReturn,strAdditionalString.c_str(),unStrLen);
            unReturn += unStrLen;
            a_entryLineBuffer[unReturn++]=')';
        }
    }

    return unReturn;

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

pitz::daq::EntryParams::AdditionalData::AdditionalData(const char* a_entryParamName)
    :
	  SomeInts<uint32_t>(a_entryParamName),
      m_pCore(nullptr)
{
    m_value = 0;
}


pitz::daq::EntryParams::AdditionalData::~AdditionalData()
{
    delete m_pCore;
}


bool pitz::daq::EntryParams::AdditionalData::ShouldSkipProviding() const
{
    return m_value ? false : true;
}


void pitz::daq::EntryParams::AdditionalData::setRootBranchIfEnabled(TTree* a_pTreeOnRoot)
{
    if(!m_pCore){return ;}
    if((!m_pCore->isInited)&&(!InitDataStuff())){return;}
    char vcBufferData[4096];
	snprintf(vcBufferData,4095,ADDITIONAL_HEADER_START DATA_HEADER_TYPE "%d" ADD_DATA_HEADER_COUNT "%d",m_pCore->dataType,m_pCore->samples);
    m_pCore->rootBranch = a_pTreeOnRoot->Branch( vcBufferData,nullptr,m_pCore->rootFormatString);
    if(m_pCore->rootBranch){
        void* pAddress = GetDataPointerFromEqData(&m_pCore->doocsData,nullptr,nullptr);
        m_pCore->rootBranch->SetAddress( pAddress );
    }
}


//time_t time;		/* Seconds since epoch, as from `time'.  */
//unsigned short int millitm;	/* Additional milliseconds.  */

#define FTIMES_TO_MS_DIFF(_timeInit,_timeFinal) \
    ( static_cast<int64_t>(((_timeFinal).time-(_timeInit).time)*1000)+static_cast<int64_t>((_timeFinal).millitm-(_timeInit).millitm)  )


void pitz::daq::EntryParams::AdditionalData::checkIfFillTimeAndFillIfYes()
{
    if((!m_pCore)||(!m_pCore->isInited)||(!m_pCore->rootBranch)){return ;}

    struct timeb timeNow;
    const int64_t  maxTime = static_cast<int64_t>(m_value);

    ftime(&timeNow);

    if( FTIMES_TO_MS_DIFF(m_pCore->lastUpdateTime,timeNow)>maxTime){
        EqData dataIn, dataOut;
        EqAdr doocsAdr;
        EqCall doocsCaller;

        m_pCore->doocsData.init();
        doocsAdr.adr(m_pCore->parentAndFinalDoocsUrl);

        if(doocsCaller.get(&doocsAdr,&dataIn,&dataOut)){
            ::std::string errorString = m_pCore->doocsData.get_string();
            ::std::cerr << errorString << ::std::endl;
            return;
        }
        m_pCore->doocsData.copy_from(&dataOut);

        void* pAddress = GetDataPointerFromEqData(&m_pCore->doocsData,nullptr,nullptr);
        if(pAddress){
            m_pCore->rootBranch->SetAddress( pAddress );
            m_pCore->lastUpdateTime = timeNow;
        }
    }
}


void pitz::daq::EntryParams::AdditionalData::initTimeAndRoot()
{
    if(m_pCore){
        m_pCore->lastUpdateTime = {0,0,0,0};
        m_pCore->rootBranch = nullptr;
    }
}


void pitz::daq::EntryParams::AdditionalData::setParentDoocsUrl( const ::std::string& a_parentDoocsUrl )
{
    if(m_pCore){
        m_pCore->parentAndFinalDoocsUrl = a_parentDoocsUrl;
        InitDataStuff();
    }
}


bool pitz::daq::EntryParams::AdditionalData::InitDataStuff()
{
	struct PrepareDaqEntryInputs in;
	struct PrepareDaqEntryOutputs out;

    if(!m_pCore){return false;}
    if(m_pCore->isInited){return true;}

	memset(&in,0,sizeof(in));
	memset(&out,0,sizeof(out));

    ptrdiff_t nCount = ::std::count(m_pCore->doocsUrl2.begin(),m_pCore->doocsUrl2.end(),'/');

    if(nCount>3){return false;}

    ::std::string fullAddr;

    if(nCount<3){
        ptrdiff_t nParentCount = ::std::count(m_pCore->parentAndFinalDoocsUrl.begin(),m_pCore->parentAndFinalDoocsUrl.end(),'/');
        const ptrdiff_t cnNumberToRecover = (3-nCount);
        if(nParentCount<cnNumberToRecover){return false;}
        size_t unIndex=0;
        for(ptrdiff_t i(0);i<cnNumberToRecover;++i){
            unIndex = m_pCore->parentAndFinalDoocsUrl.find("/",unIndex+1);
        }

        fullAddr = ::std::string(m_pCore->parentAndFinalDoocsUrl.c_str(),unIndex+1)+m_pCore->doocsUrl2;

    }
    else{
        fullAddr = m_pCore->doocsUrl2;
    }

	if( !GetEntryInfoFromDoocsServer(&m_pCore->doocsData,fullAddr,&m_pCore->dataType,&m_pCore->samples) ){return false;}
    m_pCore->parentAndFinalDoocsUrl = fullAddr;

	in.dataType = m_pCore->dataType;
	in.shouldDupString = 1;
	out.inOutSamples = m_pCore->samples;
	m_pCore->rootFormatString = PrepareDaqEntryBasedOnType(&in,&out);
    if(!(m_pCore->rootFormatString)){
        return false;
    }

    if(m_value<100){m_value=100;}

    m_pCore->isInited = 1;
    return true;
}


::std::string pitz::daq::EntryParams::AdditionalData::additionalString()const
{
    if((m_value<1)||(!m_pCore)){return "no";}

    return m_pCore->doocsUrl2;
}


bool pitz::daq::EntryParams::AdditionalData::GetDataFromLine(const char* a_entryLine)
{
    if(!SomeInts::GetDataFromLine(a_entryLine)){return false;}
    if(m_value<1){
        m_value = 0;
        delete m_pCore;
        m_pCore = nullptr;
        return false;
    }
    const char* cpcStringStart = strchr(a_entryLine,'(');
    if(!cpcStringStart){
        m_value = 0;
        delete m_pCore;
        m_pCore = nullptr;
        return false;
    }

    const char* cpcStringEnd = strchr(cpcStringStart,')');
    if((!cpcStringEnd) || ((cpcStringEnd-cpcStringStart)<4)){
        m_value = 0;
        delete m_pCore;
        m_pCore = nullptr;
        return false;
    }

    if(!m_pCore){
        m_pCore = new Core;
    }

    m_pCore->doocsUrl2 = ::std::string(cpcStringStart+1,static_cast<size_t>((cpcStringEnd-cpcStringStart)-1));

    return InitDataStuff();
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

pitz::daq::EntryParams::String::String(const char* a_entryParamName)
    :
      Base(a_entryParamName)
{
    //
}


pitz::daq::EntryParams::String::~String()
{
    //
}


bool pitz::daq::EntryParams::String::GetDataFromLine(const char* a_entryLine)
{
    size_t unStrLen = strcspn(a_entryLine," \n\t;");
    m_string = ::std::string(a_entryLine,unStrLen);
    return true;
}


size_t pitz::daq::EntryParams::String::writeDataToLineBuffer(char* a_entryLineBuffer, size_t a_unBufferSize)const
{
    size_t unStrLen = m_string.length();
    unStrLen = (unStrLen>a_unBufferSize)?a_unBufferSize:unStrLen;
    memcpy(a_entryLineBuffer,m_string.data(),unStrLen);
    return unStrLen;
}


const ::std::string& pitz::daq::EntryParams::String::value()const
{
    return m_string;
}


void pitz::daq::EntryParams::String::setValue(const ::std::string& a_newValue)
{
    m_string = a_newValue;
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


bool GetEntryInfoFromDoocsServer( EqData* a_pEqDataOut, const ::std::string& a_doocsUrl, int* a_pType, int* a_pSamples )
{
    int nReturn;
    EqCall eqCall;
    EqData dataIn;
    EqAdr eqAddr;

    eqAddr.adr(a_doocsUrl);
    nReturn = eqCall.get(&eqAddr,&dataIn,a_pEqDataOut);

    if(nReturn){
        ::std::string errorString = a_pEqDataOut->get_string();
        ::std::cerr << errorString << ::std::endl;
        return false;
    }

	*a_pType = a_pEqDataOut->type();
	*a_pSamples = a_pEqDataOut->length();

    //return GetDataPointerFromEqData(a_pEqDataOut)?true:false;
    return true;
}


void* GetDataPointerFromEqData(EqData* a_pData, int64_t* a_pTimeSeconds, int64_t* a_pMacroPulse)
{
    EqDataBlock* pDataBlock = a_pData->data_block();

    if((!pDataBlock)||(pDataBlock->error)||(!pDataBlock->tm)){return nullptr;}

    int nDataLen = a_pData->length();
    if(nDataLen<1){return nullptr;}

    if(a_pTimeSeconds){*a_pTimeSeconds=static_cast<int64_t>(pDataBlock->tm);}
    if(a_pMacroPulse){
        *a_pMacroPulse = static_cast<int64_t>((static_cast<uint64_t>(pDataBlock->mp_hi)<<32) | static_cast<uint64_t>(pDataBlock->mp_lo));
    }


    if((nDataLen<2)||(a_pData->type()==DATA_IIII)||(a_pData->type()==DATA_IFFF)){return &(pDataBlock->data_u.DataUnion_u);}

    return pDataBlock->data_u.DataUnion_u.d_char.d_char_val;
}

}}

