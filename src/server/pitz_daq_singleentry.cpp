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
        m_numberInCurrentFile(NUM_IN_CUR_FL_KEY_STR),
        m_numberInAllFiles(NUM_OF_FILES_IN_KEY_STR),
        m_expirationTime(EXPIRATION_STR),
        m_creationTime(CREATION_STR),
        m_collectionMask(MASK_COLLECTION_KEY_STR),
        m_errorMask(MASK_ERRORS_KEY_STR),
        m_errorWithString(ERROR_KEY_STR),
        m_dataType(SPECIAL_KEY_DATA_TYPE),
        m_itemsCountPerEntry(SPECIAL_KEY_DATA_SAMPLES)
{
    bool bCallIniter = false, bIsAddedByUser = false;
    time_t tmCurrentTime;
    TypeConstCharPtr& pLine = *a_pHelper;
    size_t strStart;
    int nError(errorsFromConstructor::noError);

    if(!pLine){throw errorsFromConstructor::syntax;}
    m_bitwise64Reserved = 0;

    AddNewParameterToEnd(m_dataType.thisPtr(),false,true);
    AddNewParameterToEnd(&m_itemsCountPerEntry,false,true);
    AddNewParameterToEnd(&m_numberInCurrentFile,false,false);
    AddNewParameterToEnd(&m_numberInAllFiles,false,true);
    AddNewParameterToEnd(&m_expirationTime,true,true);
    AddNewParameterToEnd(&m_creationTime,false,true);
    AddNewParameterToEnd(&m_collectionMask,true,true);
    AddNewParameterToEnd(&m_errorMask,true,true);
    AddNewParameterToEnd(m_errorWithString.thisPtr(),false,false);

    m_errorMask.SetParentAndClbk(this,[](EntryParams::Base* a_pErrMask, void* a_pThis){
        EntryParams::Mask* pErrorMask = static_cast<EntryParams::Mask*>(a_pErrMask);
        if(pErrorMask->isMasked()){
            static_cast<SingleEntry*>(a_pThis)->m_errorWithString.setError(0,"No error");
        }
    });

    // the story is following
    // everihhing, that is not nullable is set before the member m_nReserved
    memset(&m_firstEventNumber,0,static_cast<size_t>(reinterpret_cast<char*>(&m_nReserved2)-reinterpret_cast<char*>(&m_firstEventNumber)));
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
            m_numberInAllFiles=(0);
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

    if(!nReturn){
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
    uint64_t nReturn = __atomic_fetch_or (&m_willBeDeletedOrIsUsedAtomic64,VALUE_FOR_ADD_TO_NETW,__ATOMIC_RELAXED);

    if(!(nReturn&VALUE_FOR_DELETE)){
        return true;
    }

    __atomic_fetch_and (&m_willBeDeletedOrIsUsedAtomic64,~VALUE_FOR_ADD_TO_NETW,__ATOMIC_RELAXED);
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


uint64_t pitz::daq::SingleEntry::isValid()const
{
    return m_isValid ;
}


void pitz::daq::SingleEntry::SetValid()
{
    m_isValid = 1;
}


void pitz::daq::SingleEntry::SetInvalid()
{
    m_isValid = 0;
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


void pitz::daq::SingleEntry::FreeUsedMemory(DEC_OUT_PD(SingleData)* a_usedMemory)
{
    FreeDataWithOffset(a_usedMemory,0);
}


void pitz::daq::SingleEntry::Fill( DEC_OUT_PD(SingleData)* a_pNewMemory, int a_second, int a_eventNumber)
{
    static Int_t snSize=0;
    if(!m_pTreeOnRoot2){
        m_pTreeOnRoot2 = new TreeForSingleEntry(this);
        m_pBranchOnTree = m_pTreeOnRoot2->Branch(m_daqName,nullptr,this->rootFormatString());

        if(!m_pBranchOnTree){
            delete m_pTreeOnRoot2;
            m_pTreeOnRoot2 = nullptr;
            SetError(ROOT_ERROR,"Unable to create root branch");
            return ;
        }
    }

    m_pBranchOnTree->SetAddress(a_pNewMemory);
    snSize += m_pTreeOnRoot2->Fill();
    ::std::cout<< "rootFormatString="<<this->rootFormatString()<<"snSize = "<<snSize<<::std::endl;;

    if(!m_isPresentInCurrentFile){
        m_firstSecond = a_second;
        m_firstEventNumber = a_eventNumber;
        m_isPresentInCurrentFile = 1;
    }

    ++m_numberInCurrentFile;
    ++m_numberInAllFiles;

    // todo: set only root part to null
    SetError(0, "No error");

    m_lastSecond = a_second;
    m_lastEventNumber = a_eventNumber;
    FreeUsedMemory(a_pNewMemory);
}


void pitz::daq::SingleEntry::WriteContentToTheFile(FILE* a_fpFile)const
{
    //char vcBufForCrt[32],vcBufForExp[32],vcBufForMask[32];
    char vcBuffer[4096];
    char* pcBufToWrite(vcBuffer);
    size_t nBufLen(4093);
    size_t nWritten;

    for( auto pParam : m_permanentParams){
        nWritten=pParam->WriteToLineBuffer(pcBufToWrite,nBufLen);
        if(nBufLen<=(nWritten+1)){return;}
        nBufLen -= nWritten;
        pcBufToWrite += nWritten;
    }
    pcBufToWrite[0]='\n';
    pcBufToWrite[1]=0;
    pcBufToWrite[2]=0;

    fprintf(a_fpFile,"%s %s",m_daqName,vcBuffer);
}


void pitz::daq::SingleEntry::get(EqAdr* /*a_dcsAddr*/, EqData* a_dataFromUser, EqData* a_dataToUser,EqFct* /*a_loc*/)
{
    char vcBuffer[4096];
    char vcFromUser[1024];
    char* pcBufToWrite(vcBuffer);
    size_t nWritten,nBufLen(4096);
    bool bReadAll;

    a_dataFromUser->get_string(vcFromUser,1023);
    if(vcFromUser[0]==0){bReadAll=true;}
    else{ bReadAll=true; }

    if(bReadAll){
        for( auto pParam : m_allParams){
            nWritten=pParam->WriteToLineBuffer(pcBufToWrite,nBufLen);
            if(nBufLen<=nWritten){goto returnPoint;}
            nBufLen -= nWritten;
            pcBufToWrite += nWritten;
        }
    }
    else{
        for( auto pParam : m_allParams){
            if(strstr(vcFromUser,pParam->paramName())){
                nWritten=pParam->WriteToLineBuffer(pcBufToWrite,nBufLen);
                if(nBufLen<=nWritten){goto returnPoint;}
                nBufLen -= nWritten;
                pcBufToWrite += nWritten;
            }
        }
    }

returnPoint:
    a_dataToUser->set(vcBuffer);
    return;
}


void pitz::daq::SingleEntry::SetError(int a_error, const ::std::string& a_errorString)
{
    EqFctCollector* pClc = m_pNetworkParent?m_pNetworkParent->m_pParent:NEWNULLPTR2;

    if(a_error&&(!m_errorWithString.value())){
        if(pClc){pClc->IncrementErrors(m_daqName);}
    }
    else if((a_error==0)&&(m_errorWithString.value())){
        if(pClc){pClc->DecrementErrors(m_daqName);}
    }

    m_errorWithString.setError(a_error,a_errorString);
}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

pitz::daq::SNetworkStruct::SNetworkStruct(EqFctCollector* a_parent)
        :
        m_pParent(a_parent)
{
    m_shouldRun = 1;
    m_bitwise64Reserved =0;

    m_thread = STDN::thread(&EqFctCollector::DataGetterThread2,m_pParent,this);
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


const ::std::list< pitz::daq::SingleEntry* >& pitz::daq::SNetworkStruct::daqEntries()/*const*/
{
    return m_daqEntries;
}


bool pitz::daq::SNetworkStruct::AddNewEntry(SingleEntry *a_newEntry)
{
    if(!a_newEntry || (a_newEntry->m_pNetworkParent)){return false;}
    m_daqEntries.push_front(a_newEntry);
    a_newEntry->m_thisIter = m_daqEntries.begin();
    a_newEntry->m_pNetworkParent = this;
    m_pParent->add_property(a_newEntry);
    //a_newEntry->SetNetworkParent(this);

    return true;
}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

pitz::daq::TreeForSingleEntry::TreeForSingleEntry( pitz::daq::SingleEntry* a_pParentEntry )
    :
      ::TTree(a_pParentEntry->m_daqName, "DATA"),
      m_pParentEntry(a_pParentEntry)
{
}


pitz::daq::TreeForSingleEntry::~TreeForSingleEntry()
{
    if(m_pParentEntry ){
        m_pParentEntry->m_pBranchOnTree = NEWNULLPTR2;
        m_pParentEntry->m_pTreeOnRoot2 = NEWNULLPTR2;
        m_pParentEntry->m_isPresentInCurrentFile = 0;
        m_pParentEntry->m_numberInCurrentFile = 0;
        if(m_pParentEntry->resetRootLockAndReturnIfDeletable()){
            SingleEntry* pParentEntry( m_pParentEntry );
            m_pParentEntry=NEWNULLPTR2;
            delete pParentEntry;
        }
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


size_t pitz::daq::EntryParams::Base::WriteToLineBuffer(char* a_entryLineBuffer, size_t a_unBufferSize)const
{
    size_t unStrLen = strlen(m_paramName);

    if(LIKELY2(a_unBufferSize>(unStrLen+1))){
        size_t unDataStrLen;
        memcpy(a_entryLineBuffer,m_paramName,unStrLen);
        a_entryLineBuffer[unStrLen++]='=';
        a_unBufferSize -= unStrLen;
        a_entryLineBuffer += unStrLen;
        unDataStrLen=this->WriteDataToLineBuffer(a_entryLineBuffer,a_unBufferSize);
        if(a_unBufferSize>unDataStrLen){
            a_entryLineBuffer[unDataStrLen++]=';';
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

pitz::daq::EntryParams::SomeInts::SomeInts(const char* a_entryParamName)
    :
      IntParam<int>(a_entryParamName)
{
    m_value = 0;
}


pitz::daq::EntryParams::SomeInts::~SomeInts()
{
    //
}


int pitz::daq::EntryParams::SomeInts::value()const
{
    return m_value;
}


pitz::daq::EntryParams::Base* pitz::daq::EntryParams::SomeInts::thisPtr()
{
    return this;
}


size_t pitz::daq::EntryParams::SomeInts::WriteDataToLineBuffer(char* a_entryLineBuffer, size_t a_unBufferSize)const
{
    ::std::string strAdditionalString = this->additionalString();
    size_t unReturn = IntParam<int32_t>::WriteDataToLineBuffer(a_entryLineBuffer,a_unBufferSize);
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
      SomeInts(a_entryParamName)
{
}


pitz::daq::EntryParams::Error::~Error()
{
    //
}


void pitz::daq::EntryParams::Error::setError(int a_error, const ::std::string& a_errorString)
{
    switch(a_error){
    case 0:
        m_errorString = "";
        break;
    default:
        if(!(a_error & value())){
            m_errorString += ";";
            m_errorString += a_errorString;
        }
        break;
    }
}


::std::string pitz::daq::EntryParams::Error::additionalString() const
{
    return m_errorString;
}


/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

pitz::daq::EntryParams::DataType::DataType(const char* a_entryParamName)
    :
      SomeInts(a_entryParamName)
{
}


pitz::daq::EntryParams::DataType::~DataType()
{
    //
}


void pitz::daq::EntryParams::DataType::set(int32_t a_type)
{
    m_value = a_type;
}


::std::string pitz::daq::EntryParams::DataType::additionalString() const
{
    EqData eqData;
    //std::string strForReturn = eqData.type_string(value());
    return eqData.type_string(value());
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
    m_epochSeconds = STRING_TO_EPOCH(a_entryLine+1);
    return true;
}

static const size_t s_cunNonExpireStrLen = strlen(NON_EXPIRE_STRING);


size_t pitz::daq::EntryParams::Date::WriteDataToLineBuffer(char* a_entryLineBuffer, size_t a_unBufferSize)const
{
    size_t unReturn(0);
    switch(m_epochSeconds){
    case NOT_MASKED_TO_TIME:
        return 0;
    case NON_EXPIRE_TIME:
        if(a_unBufferSize<5){
            return 0;
        }
        memcpy(a_entryLineBuffer,"never",5);
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


pitz::daq::EntryParams::Mask::~Mask()
{
}


bool pitz::daq::EntryParams::Mask::GetDataFromLine(const char* a_entryLine)
{
    m_epochSeconds = NOT_MASKED_TO_TIME;

    if(strncmp(a_entryLine,"true(",5)==0){
        Date::GetDataFromLine(a_entryLine+5);
    }

    return true;
}


size_t pitz::daq::EntryParams::Mask::WriteDataToLineBuffer(char* a_entryLineBuffer, size_t a_unBufferSize)const
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
        unReturn = 5 + Date::WriteDataToLineBuffer(a_entryLineBuffer,a_unBufferSize);
        a_entryLineBuffer[unReturn++]=')';
        break;
    }

    return unReturn;
}


bool pitz::daq::EntryParams::Mask::isMasked()const
{
    return m_epochSeconds != NOT_MASKED_TO_TIME;
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


size_t pitz::daq::EntryParams::String::WriteDataToLineBuffer(char* a_entryLineBuffer, size_t a_unBufferSize)const
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

}}

