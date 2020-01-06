//include "pitz_daq_singleentry.h"
// pitz_daq_singleentry.cpp
// 2017 Sep 15


#include <cstdlib>
#define atoll       atol
#define strtoull    strtoul
#include "pitz_daq_singleentry.hpp"
#include <string.h>
#include <stdlib.h>
#include "pitz_daq_collectorproperties.hpp"
#include "pitz_daq_eqfctcollector.hpp"
#include <signal.h>
#include <pitz_daq_data_handling_types.h>
#include "pitz_daq_singleentry.cpp.hpp"

#define DATA_SIZE_TO_SAVE   50000  // 40 kB
#define MIN_NUMBER_OF_FILLS 20

static void SignalHandler(int){}


namespace pitz{ namespace daq{

static size_t EPOCH_TO_STRING3(const time_t& a_epoch, char* a_buffer, size_t a_bufferLength);
static time_t STRING_TO_EPOCH2(const char* a_string);

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
        m_numberInCurrentFile(NUM_IN_CUR_FL_KEY_STR,&m_allParams),
        m_numberInAllFiles(NUM_OF_FILES_IN_KEY_STR,&m_allParams),
        m_expirationTime(EXPIRATION_STR,&m_allParams),
        m_creationTime(CREATION_STR,&m_allParams),
        m_collectionMask(MASK_COLLECTION_KEY_STR,&m_allParams),
        m_errorMask(MASK_ERRORS_KEY_STR,&m_allParams),
        m_error(ERROR_KEY_STR,&m_allParams)
{
    time_t tmCurrentTime;
    TypeConstCharPtr& pLine = *a_pHelper;
    size_t strStart;
    int nError(errorsFromConstructor::noError);

    if(!pLine){throw errorsFromConstructor::syntax;}
    m_bitwise64Reserved = 0;

    m_permanentParams.push_back(&m_numberInAllFiles);
    m_permanentParams.push_back(&m_expirationTime);
    m_permanentParams.push_back(&m_creationTime);
    m_permanentParams.push_back(&m_collectionMask);
    m_permanentParams.push_back(&m_errorMask);
    //
    m_userSetableParams.push_back(&m_expirationTime);
    m_userSetableParams.push_back(&m_collectionMask);
    m_userSetableParams.push_back(&m_errorMask);

    m_errorMask.SetParentAndClbk(this,[](EntryParams::Base* a_pErrMask, void* a_pThis){
        EntryParams::Mask* pErrorMask = static_cast<EntryParams::Mask*>(a_pErrMask);
        if(pErrorMask->isMasked()){
            static_cast<SingleEntry*>(a_pThis)->m_error = (0);
        }
    });

    // the story is following
    // everihhing, that is not nullable is set before the member m_nReserved
    memset(&m_nReserved,0,sizeof(SingleEntry)-static_cast<size_t>(reinterpret_cast<char*>(&m_nReserved)-reinterpret_cast<char*>(this)));
    m_branchInfo = {PITZ_DAQ_UNSPECIFIED_DATA_TYPE,PITZ_DAQ_UNSPECIFIED_NUMBER_OF_SAMPLES};

    strStart = strspn (pLine,POSIIBLE_TERM_SYMBOLS);
    if(pLine[strStart]==0){nError = errorsFromConstructor::syntax;goto reurnPoint;}
    pLine += strStart;

    m_creationTime=time(&tmCurrentTime);
    m_numberInCurrentFile=(0);

    switch (a_creationType)
    {
    case entryCreationType::fromOldFile:
        break;
    case entryCreationType::fromConfigFile:

        for( auto pParam : m_permanentParams){
            pParam->FindAndGetFromLine(pLine);
        }

        break;

    case entryCreationType::fromUser:

        for( auto pParam : m_allParams){
            pParam->FindAndGetFromLine(pLine);
        }
        m_numberInCurrentFile=(0);
        m_numberInAllFiles=(0);
        m_creationTime=(tmCurrentTime);

        break;

    default:
        break;
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

#define VALUE_FOR_DELETE            1u
#define VALUE_FOR_ADD_TO_ROOT       static_cast<uint32_t>(1<<4)
#define VALUE_FOR_ADD_TO_NETW       static_cast<uint32_t>(1<<8)
#define VALUE_FOR_UNKNOWN_STATE     static_cast<uint32_t>(1<<12)
#define NON_DELETABLE_BITS          (VALUE_FOR_ADD_TO_ROOT | VALUE_FOR_ADD_TO_NETW)

bool pitz::daq::SingleEntry::markEntryForDeleteAndReturnIfPossibleNow()
{
    uint32_t nReturn = __atomic_fetch_or (&m_willBeDeletedOrAddedToRootAtomic,VALUE_FOR_DELETE,__ATOMIC_RELAXED);
    return nReturn ? false : true;
}


bool pitz::daq::SingleEntry::lockEntryForRoot()
{
    uint32_t nReturn = __atomic_fetch_or (&m_willBeDeletedOrAddedToRootAtomic,VALUE_FOR_ADD_TO_ROOT,__ATOMIC_RELAXED);

    if(!(nReturn&VALUE_FOR_DELETE)){
        return true;
    }

    __atomic_fetch_and (&m_willBeDeletedOrAddedToRootAtomic,~VALUE_FOR_ADD_TO_ROOT,__ATOMIC_RELAXED);
    return false;
}


bool pitz::daq::SingleEntry::lockEntryForNetwork()
{
    uint32_t nReturn = __atomic_fetch_or (&m_willBeDeletedOrAddedToRootAtomic,VALUE_FOR_ADD_TO_NETW,__ATOMIC_RELAXED);

    if(!(nReturn&VALUE_FOR_DELETE)){
        return true;
    }

    __atomic_fetch_and (&m_willBeDeletedOrAddedToRootAtomic,~VALUE_FOR_ADD_TO_NETW,__ATOMIC_RELAXED);
    return false;
}


bool pitz::daq::SingleEntry::resetRootLockAndReturnIfDeletable()
{
    uint32_t nReturn = __atomic_and_fetch (&m_willBeDeletedOrAddedToRootAtomic,~VALUE_FOR_ADD_TO_ROOT,__ATOMIC_RELAXED);

    if(nReturn == VALUE_FOR_DELETE){ return true; }

    return false;
}


bool pitz::daq::SingleEntry::resetNetworkLockAndReturnIfDeletable()
{
    uint32_t nReturn = __atomic_and_fetch (&m_willBeDeletedOrAddedToRootAtomic,~VALUE_FOR_ADD_TO_NETW,__ATOMIC_RELAXED);
    return nReturn == VALUE_FOR_DELETE ? true : false;
}


bool pitz::daq::SingleEntry::isLockedByRootOrNetwork()const
{
    uint32_t nReturn = __atomic_load_n (&m_willBeDeletedOrAddedToRootAtomic,__ATOMIC_RELAXED);

    if(nReturn&NON_DELETABLE_BITS){ return true; }

    return false;
}



char* pitz::daq::SingleEntry::ApplyEntryInfo( DEC_OUT_PD(BOOL2) a_bDubRootString )
{
    char* pcRootFormatString;
    uint32_t unOnlyDataBufferSize,unTotalRootBufferSize;

    if( (m_branchInfo.dataType == PITZ_DAQ_UNSPECIFIED_DATA_TYPE)||(m_branchInfo.itemsCountPerEntry<1) ){
        return NEWNULLPTR2;
    }

    pcRootFormatString = PrepareDaqEntryBasedOnType(a_bDubRootString,&m_branchInfo,&unOnlyDataBufferSize,&unTotalRootBufferSize,NEWNULLPTR2,NEWNULLPTR2,NEWNULLPTR2);
    if(!pcRootFormatString){return NEWNULLPTR2;}

    m_totalRootBufferSize = unTotalRootBufferSize;
    m_onlyNetDataBufferSize = unOnlyDataBufferSize;

    return pcRootFormatString;
}


//void pitz::daq::SingleEntry::RemoveDoocsProperty()
//{
//    if( m_pNetworkParent && m_pNetworkParent->parent()){
//        m_pNetworkParent->parent()->rem_property(this);
//    }
//}


void pitz::daq::SingleEntry::SetNetworkParent(SNetworkStruct* a_pNetworkParent)
{

    if(a_pNetworkParent == m_pNetworkParent){return;}

    m_isCleanEntryInheritableCalled = 0;

    m_pNetworkParent = a_pNetworkParent;

    if(a_pNetworkParent && a_pNetworkParent->parent()){
        a_pNetworkParent->parent()->add_property(this);
    }
}



//pitz::daq::SNetworkStruct* pitz::daq::SingleEntry::networkParent()
//{
//    return m_pNetworkParent;
//}


//bool pitz::daq::SingleEntry::KeepEntry2()const
//{
//    if(m_pp.expirationTime>0){
//        time_t currentTime;
//        currentTime = time(&currentTime);
//        if(currentTime>=m_pp.expirationTime){
//            return false;
//        }
//    }
//
//    return true;
//}


//void pitz::daq::SingleEntry::MaskErrors(const char* a_maskResetTime)
//{
//    m_pp.errorMaskParam.expirationTime = STRING_TO_EPOCH(a_maskResetTime,MASK_NO_EXPIRE_STRING);
//    if(m_pp.errorMaskParam.expirationTime<0){m_pp.errorMaskParam.expirationTime=NON_EXPIRE_TIME;}
//    m_pp.errorsMasked = true;
//
//    SetError(0);
//}
//
//
//void pitz::daq::SingleEntry::UnmaskErrors()
//{
//    m_pp.errorsMasked = false;
//    m_pp.errorMaskParam.expirationTime=NON_EXPIRE_TIME;
//}
//

void pitz::daq::SingleEntry::set(EqAdr* a_dcsAddr, EqData* a_dataFromUser, EqData* a_dataToUser,EqFct* a_loc)
//void pitz::daq::SingleEntry::SetProperty(const char* a_propertyAndAttributes)
{
    char vcBuffer[1024];

    a_dataFromUser->get_string(vcBuffer,1023);

    for( auto pParam : m_userSetableParams){
        pParam->FindAndGetFromLine(vcBuffer);
    }

    D_BASE_FOR_STR::set(a_dcsAddr, a_dataFromUser, a_dataToUser,a_loc);
}


//void pitz::daq::SingleEntry::SetRootTreeAndBranchAddress(TTree* a_tree)
//{
//    m_pTreeOnRoot2 = a_tree;
//    if(!a_tree){m_pBranchOnTree=nullptr; return;}
//    ++m_pp.numOfFilesIn;
//    m_nNumberInCurrentFile = 0;
//    m_isPresentInCurrentFile = 0;
//
//    //CalculateAndSetString();
//
//    m_pBranchOnTree = m_pTreeOnRoot2->Branch(this->daqName(),nullptr,this->rootFormatString());
//
//}


//DEC_OUT_PD(SingleData)* pitz::daq::SingleEntry::GetNewMemoryForNetwork()
//{
//    return CreateDataWithOffset(m_unOffset,m_totalRootBufferSize);
//}


void pitz::daq::SingleEntry::FreeUsedMemory(DEC_OUT_PD(SingleData)* a_usedMemory)
{
    free(a_usedMemory);
}


//pitz::daq::SNetworkStruct* pitz::daq::SingleEntry::CleanEntryNoFree()
//{
//    SNetworkStruct* pNetworkParent = m_pNetworkParent;
//
//    this->CleanEntryNoFreeInheritable();
//    if(!m_isCleanEntryInheritableCalled){
//        SingleEntry::CleanEntryNoFreeInheritable();
//    }
//    this->CleanEntryNoFreeInheritable();
//    return pNetworkParent;
//}


//void pitz::daq::SingleEntry::CleanEntryNoFreeInheritable()
//{
//    if(m_pNetworkParent){
//        m_pNetworkParent->m_daqEntries.erase(this->m_thisIter);
//        if(m_pNetworkParent->parent()){
//            m_pNetworkParent->parent()->rem_property(this);
//        }
//        m_pNetworkParent = NEWNULLPTR2;
//    }
//
//    m_isCleanEntryInheritableCalled=1;
//}


//void pitz::daq::SingleEntry::SetMemoryBack( DEC_OUT_PD(SingleData)* a_pMemory )
//{
//    FreeDataWithOffset(a_pMemory,m_unOffset);
//}


void pitz::daq::SingleEntry::Fill( DEC_OUT_PD(SingleData)* a_pNewMemory, int a_second, int a_eventNumber)
{
    if(!m_pTreeOnRoot2){
        m_pTreeOnRoot2 = new TreeForSingleEntry(this);
        m_pBranchOnTree = m_pTreeOnRoot2->Branch(m_daqName,nullptr,this->rootFormatString());

        if(!m_pBranchOnTree){
            delete m_pTreeOnRoot2;
            m_pTreeOnRoot2 = nullptr;
            SetError(1);
            return ;
        }
    }

    m_pBranchOnTree->SetAddress(a_pNewMemory);
    m_pTreeOnRoot2->Fill();

    if(!m_isPresentInCurrentFile){
        m_firstSecond = a_second;
        m_firstEventNumber = a_eventNumber;
        m_isPresentInCurrentFile = 1;
    }

    ++m_numberInCurrentFile;
    ++m_numberInAllFiles;

    // todo: set only root part to null
    SetError(0);

    m_lastSecond = a_second;
    m_lastEventNumber = a_eventNumber;
    FreeUsedMemory(a_pNewMemory);
}


//void pitz::daq::SingleEntry::AddExistanceInRootFile(int a_second, int a_eventNumber)
//{
//    if(!m_isPresentInCurrentFile){
//        m_firstSecond = a_second;
//        m_firstEventNumber = a_eventNumber;
//        m_isPresentInCurrentFile = 1;
//    }
//
//    ++m_nNumberInCurrentFile;
//    ++m_pp.numberInAllFiles;
//    SetError(0);
//
//    m_lastSecond = a_second;
//    m_lastEventNumber = a_eventNumber;
//}


void pitz::daq::SingleEntry::WriteContentToTheFile(FILE* a_fpFile)const
{
    //char vcBufForCrt[32],vcBufForExp[32],vcBufForMask[32];
    char vcBuffer[4096];
    char* pcBufToWrite(vcBuffer);
    size_t nBufLen(4096);
    size_t nWritten;

    for( auto pParam : m_userSetableParams){
        nWritten=pParam->WriteToLineBuffer(pcBufToWrite,nBufLen);
        if(nBufLen<=(nWritten+1)){return;}
        nBufLen -= nWritten;
        pcBufToWrite += nWritten;
    }
    pcBufToWrite[0]=0;

    fprintf(a_fpFile,"%s %s",m_daqName,vcBuffer);
}


//CREATION_STR    "creation"
//#define EXPIRATION_STR    "expiration"
//#define ERROR_KEY_STR   "error"
//#define NUM_IN_CUR_FL_KEY_STR "numberInCurrentFile"
//#define NUM_OF_FILES_IN_KEY_STR "numOfFiles"
//#define NUM_OF_ALL_ENTRIES_KEY_STR "numOfAllEntries"
//#define NUM_OF_ERRORS_KEY_STR   "numOfErrors"

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


void pitz::daq::SingleEntry::SetError(int a_error)
{
    EqFctCollector* pClc = m_pNetworkParent?m_pNetworkParent->parent():NEWNULLPTR2;

    if(a_error&&(!m_error)){
        if(pClc){pClc->IncrementErrors(m_daqName);}
    }
    else if((a_error==0)&&(m_error)){
        if(pClc){pClc->DecrementErrors(m_daqName);}
    }

    m_error=(a_error);
}


/*//////////////////////////////////////////////////*/

pitz::daq::SNetworkStruct::SNetworkStruct(EqFctCollector* a_parent)
        :
        m_pParent(a_parent)
{
    m_shouldRun = 1;
    m_bitwise64Reserved =0;

    m_pThread = new STDN::thread(&SNetworkStruct::DataGetterThread,this);
}


pitz::daq::SNetworkStruct::~SNetworkStruct()
{
    StopThreadAndClear();
}


void pitz::daq::SNetworkStruct::StopThreadAndClear()
{
    if(m_pThread){
        pthread_t handleToThread = static_cast<pthread_t>(m_pThread->native_handle());

        m_shouldRun = 0;
        pthread_kill(handleToThread,SIGNAL_FOR_CANCELATION);
        m_pThread->join();
        delete m_pThread;
        m_pThread = NEWNULLPTR2;

        for(auto pEntry : m_daqEntries){
            delete pEntry;
        }
        m_daqEntries.clear();
    }
}


void pitz::daq::SNetworkStruct::DataGetterThread()
{
    struct sigaction sigAction;

    //sigAction.sa_handler = SignalHandler;
#ifdef sa_handler
#pragma push_macro("sa_handler")
#undef sa_handler
    sigAction.__sigaction_handler.sa_handler = SignalHandler;
#pragma pop_macro("sa_handler")
#else
    sigAction.sa_handler = SignalHandler;
#endif
    sigfillset(&sigAction.sa_mask);
    sigdelset(&sigAction.sa_mask,SIGNAL_FOR_CANCELATION);
    sigAction.sa_flags = 0;
    sigAction.sa_restorer = NEWNULLPTR2; // not used

    // we have to init sig handle, because in some cases, we will stop by interrupt
    sigaction(SIGNAL_FOR_CANCELATION,&sigAction,NEWNULLPTR2);


    while(m_shouldRun && m_pParent->shouldWork()){
        //if((m_daqEntries.size()>0)&&(!m_pParent2->m_nDataStopCount)){
        if((m_daqEntries.size()>0)){
            m_pParent->DataGetterThread(this);
        }
        else{
            SleepMs(2);
        }
    }

}


pitz::daq::EqFctCollector* pitz::daq::SNetworkStruct::parent()
{
    return m_pParent;
}


const ::std::list< pitz::daq::SingleEntry* >& pitz::daq::SNetworkStruct::daqEntries()const
{
    return m_daqEntries;
}


uint64_t pitz::daq::SNetworkStruct::shouldRun()const
{
    return m_shouldRun;
}


//void pitz::daq::SNetworkStruct::RemoveEntryNoDeletePrivate(SingleEntry *a_newEntry)
//{
//    m_daqEntries.erase(a_newEntry->m_thisIter);
//}


bool pitz::daq::SNetworkStruct::AddNewEntry(SingleEntry *a_newEntry)
{
    if(!a_newEntry){return false;}
    m_daqEntries.push_front(a_newEntry);
    a_newEntry->m_thisIter = m_daqEntries.begin();
    a_newEntry->SetNetworkParent(this);

    return true;
}


/*////////////////////////////////////*/
//pitz::daq::D_stringForEntry::D_stringForEntry(const char* a_pn, SingleEntry* a_parent)
//        :
//        D_BASE_FOR_STR(a_pn,NEWNULLPTR2),
//        m_pParent(a_parent)
//{
//}
//
//
//pitz::daq::D_stringForEntry::~D_stringForEntry()
//{
//}
//
//
//void pitz::daq::D_stringForEntry::get(EqAdr* /*a_dcsAddr*/, EqData* a_dataFromUser, EqData* a_dataToUser,EqFct* /*a_loc*/)
//{
//    char vcBuffer[4096];
//    char vcFromUser[512];
//    //bool bFound = m_pParent->ValueStringByKey(dataFromUser,vcBuffer,511);
//
//    a_dataFromUser->get_string(vcFromUser,511);
//
//    m_pParent->ValueStringByKey2(vcFromUser,vcBuffer,4095);
//
//    a_dataToUser->set(vcBuffer);
//}
//
//
//void pitz::daq::D_stringForEntry::set(EqAdr* a_dcsAddr, EqData* a_dataFromUser, EqData* a_dataToUser,EqFct* a_loc)
//{
//
//    char vcBuffer[1024];
//    a_dataFromUser->get_string(vcBuffer,1023);
//    m_pParent->SetProperty(vcBuffer);
//    D_BASE_FOR_STR::set(a_dcsAddr, a_dataFromUser, a_dataToUser,a_loc);
//
//
//}

/*//////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
pitz::daq::TreeForSingleEntry::TreeForSingleEntry( pitz::daq::SingleEntry* a_pParentEntry )
    :
      ::TTree(a_pParentEntry->m_daqName, "DATA"),
      m_pParentEntry(a_pParentEntry)
{
    //m_pParentEntry->SetT
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

/*////////////////////////////////////////////////////////////////////////////////////////*/

static void DefaultClbk(pitz::daq::EntryParams::Base*,void*){}

pitz::daq::EntryParams::Base::Base( const char* a_entryParamName, ::std::list<EntryParams::Base*>* a_pContainer )
    :
      m_paramName(a_entryParamName),
      m_fpClbk(&DefaultClbk),
      m_pParent(nullptr)
{
    a_pContainer->push_back(this);
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
        a_entryLineBuffer[unStrLen]='=';
        a_unBufferSize -= (unStrLen+1);
        a_entryLineBuffer += (unStrLen+1);
        unDataStrLen=this->WriteDataToLineBuffer(a_entryLineBuffer,a_unBufferSize);
        if(a_unBufferSize>unDataStrLen){
            a_entryLineBuffer[unDataStrLen]=';';
        }
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

/*////////////////////////////////////////////////////////////////////////////////////////*/

pitz::daq::EntryParams::Mask::Mask(const char* a_entryParamName, ::std::list<EntryParams::Base*>* a_pContainer)
    :
      Base(a_entryParamName,a_pContainer)
{
    m_expirationTime = NOT_MASKED_TO_TIME;
}


pitz::daq::EntryParams::Mask::~Mask()
{
}


bool pitz::daq::EntryParams::Mask::GetDataFromLine(const char* a_entryLine)
{
    m_expirationTime = NOT_MASKED_TO_TIME;

    if(strncmp(a_entryLine,"true",4)==0){
        const char* cpcDataLine = a_entryLine+4;
        m_expirationTime = NON_EXPIRE_TIME;
        if(cpcDataLine[0]=='('){
            m_expirationTime = STRING_TO_EPOCH2(cpcDataLine+1);
        }
    }

    return true;
}

static const size_t s_cunNonExpireStrLen = strlen(NON_EXPIRE_STRING);


size_t pitz::daq::EntryParams::Mask::WriteDataToLineBuffer(char* a_entryLineBuffer, size_t a_unBufferSize)const
{
    size_t unReturn(0);
    switch(m_expirationTime){
    case NOT_MASKED_TO_TIME:
        if(a_unBufferSize<5){
            return 0;
        }
        memcpy(a_entryLineBuffer,"false",5);
        return 5;
    case NON_EXPIRE_TIME:
        if(a_unBufferSize<(6+s_cunNonExpireStrLen)){
            return 0;
        }
        memcpy(a_entryLineBuffer,"true",4);
        unReturn = 4;
        a_entryLineBuffer[unReturn++]='(';
        memcpy(a_entryLineBuffer+unReturn,NON_EXPIRE_STRING,s_cunNonExpireStrLen);
        unReturn += s_cunNonExpireStrLen;
        a_entryLineBuffer[unReturn++]=')';
        break;
    default:
        if(a_unBufferSize<(16+s_cunNonExpireStrLen)){
            return 0;
        }
        memcpy(a_entryLineBuffer,"true",4);
        unReturn = 4;
        a_entryLineBuffer[unReturn++]='(';
        unReturn += EPOCH_TO_STRING3(m_expirationTime,a_entryLineBuffer+unReturn,a_unBufferSize-unReturn-1);
        a_entryLineBuffer[unReturn++]=')';
        break;
    }

    return unReturn;
}


time_t pitz::daq::EntryParams::Mask::expirationTime() const
{
    return m_expirationTime;
}


bool pitz::daq::EntryParams::Mask::isMasked()const
{
    return m_expirationTime != NOT_MASKED_TO_TIME;
}


/*////////////////////////////////////////////////////////////////////////////////////////*/

namespace pitz{ namespace daq{

time_t STRING_TO_EPOCH(const char* _a_string,const char* a_cpcInf)
{
    char *pcNext;
    const char *pcTmp;
    struct tm aTm;

    if(a_cpcInf && (a_cpcInf[0]!=0)&&(strncmp((_a_string),a_cpcInf,strlen(a_cpcInf))==0)){return NON_EXPIRE_TIME;}

    aTm.tm_year = static_cast<decltype (aTm.tm_year)>(strtol((_a_string),&pcNext,10) - 1900);
    if((pcNext++)==(_a_string)){return -errorsFromConstructor::syntax;}

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


static time_t STRING_TO_EPOCH2(const char* a_string)
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


const char* EPOCH_TO_STRING2(const time_t& a_epoch, const char* a_cpcInf, char* a_buffer, int a_bufferLength)
{

    if((a_epoch>0) && (a_epoch!=NON_EXPIRE_TIME)){
        struct tm aTm;
        localtime_r(&a_epoch,&aTm);
        snprintf(a_buffer,static_cast<size_t>(a_bufferLength),"%d.%.2d.%.2d-%.2d:%.2d",aTm.tm_year+1900,aTm.tm_mon+1,aTm.tm_mday,aTm.tm_hour,aTm.tm_min);
    }
    else{
        snprintf(a_buffer,static_cast<size_t>(a_bufferLength),"%s", a_cpcInf);
    }

    return a_buffer;

}


static size_t EPOCH_TO_STRING3(const time_t& a_epoch, char* a_buffer, size_t a_bufferLength)
{
    struct tm aTm;
    localtime_r(&a_epoch,&aTm);
    return static_cast<size_t>(snprintf(a_buffer,a_bufferLength,"%d.%.2d.%.2d-%.2d:%.2d",aTm.tm_year+1900,aTm.tm_mon+1,aTm.tm_mday,aTm.tm_hour,aTm.tm_min));
}

}}

