//include "pitz_daq_singleentry.h"
// pitz_daq_singleentry.cpp
// 2017 Sep 15


#include "pitz_daq_singleentry.hpp"
#include <string.h>
#include <stdlib.h>
#include "pitz_daq_collectorproperties.hpp"
#include "pitz_daq_eqfctcollector.hpp"
#include <signal.h>

#define DATA_SIZE_TO_SAVE   50000  // 40 kB
#define MIN_NUMBER_OF_FILLS 20

static void SignalHandler(int){}


pitz::daq::SingleEntry::SingleEntry(entryCreationType::Type a_creationType,const char* a_entryLine)
        :
        m_daqName(NEWNULLPTR2),
        m_pDoocsProperty(NEWNULLPTR2),
        m_pNetworkParent(NEWNULLPTR2),
        m_pTreeOnRoot(NEWNULLPTR2),
        m_pBranchOnTree(NEWNULLPTR2)
{
    const char* pLine = strpbrk(a_entryLine,POSIIBLE_TERM_SYMBOLS);
    const char* pcNext ;
    size_t strStart;
    size_t daqNameLen;
    int nError(errorsFromConstructor::noError);

    if(!pLine){throw errorsFromConstructor::syntax;}

    m_isPresentInCurrentFile = 0;

    m_willBeDeletedOrAddedToRootAtomic = 0;

    // 0,{PITZ_DAQ_UNSPECIFIED_DATA_TYPE,PITZ_DAQ_UNSPECIFIED_NUMBER_OF_SAMPLES}
    m_unOffset = 0;
    m_branchInfo = {PITZ_DAQ_UNSPECIFIED_DATA_TYPE,PITZ_DAQ_UNSPECIFIED_NUMBER_OF_SAMPLES};

    m_isPresentInCurrentFile = 0;
    m_pForRoot = NEWNULLPTR2;
    m_bufferSize = 0;
    m_unOffset = 0;
    m_unAllocatedBufferSize = 0;
    //this->SetEntryInfo(a_unOffset,a_branchInfo);

    m_nLastEventNumberHandled = 0;

    daqNameLen = static_cast<size_t>(pLine-a_entryLine);
    m_daqName = static_cast<char*>(malloc(daqNameLen+1));
    if(!m_daqName){throw errorsFromConstructor::lowMemory;}
    memcpy(m_daqName,a_entryLine,daqNameLen);
    m_daqName[daqNameLen] = 0;

    strStart = strspn (pLine,POSIIBLE_TERM_SYMBOLS);
    if(pLine[strStart]==0){nError = errorsFromConstructor::syntax;goto reurnPoint;}
    pLine += strStart;

    switch (a_creationType)
    {
    case entryCreationType::fromOldFile:
        m_pp.numOfFilesIn = 0;
        m_pp.numberInAllFiles = 0;
        m_pp.creationTime = time(&m_pp.creationTime);
        m_pp.expirationTime = NON_EXPIRE_TIME;
        m_pp.masked = false;
        m_pp.mp.unmaskTime = NON_EXPIRE_TIME;
        break;
    case entryCreationType::fromConfigFile:

        //#define CREATION_STR    "creation"
        //#define EXPIRATION_STR    "expiration"
        //#define ERROR_KEY_STR   "error"
        //#define NUM_IN_CUR_FL_KEY_STR "entries"
        //#define NUM_OF_FILES_IN_KEY_STR "files"
        //#define NUM_OF_ALL_ENTRIES_KEY_STR "allentries"
        //#define NUM_OF_ERRORS_KEY_STR   "errors"

        pcNext = strstr(pLine,CREATION_STR "=");
        if(!pcNext){nError = errorsFromConstructor::syntax;goto reurnPoint; }
        pcNext += strlen(CREATION_STR "=");
        m_pp.creationTime = STRING_TO_EPOCH(pcNext,"");
        if((m_pp.creationTime<0)&&(m_pp.creationTime!=NON_EXPIRE_TIME)){nError = -1;goto reurnPoint;}

        pcNext = strstr(pLine,EXPIRATION_STR "=");
        if(!pcNext){nError = errorsFromConstructor::syntax;goto reurnPoint; }
        pcNext += strlen(EXPIRATION_STR "=");
        m_pp.expirationTime = STRING_TO_EPOCH(pcNext,NON_EXPIRE_STRING);
        if((m_pp.expirationTime<0)&&(m_pp.expirationTime!=NON_EXPIRE_TIME)){nError = -2;goto reurnPoint;}

        pcNext = strstr(pLine,NUM_OF_FILES_IN_KEY_STR "=");
        if(!pcNext){nError = errorsFromConstructor::syntax;goto reurnPoint; }
        pcNext += strlen(NUM_OF_FILES_IN_KEY_STR "=");
        m_pp.numOfFilesIn = atoi(pcNext);


        pcNext = strstr(pLine,NUM_OF_ALL_ENTRIES_KEY_STR "=");
        if(!pcNext){nError = errorsFromConstructor::syntax;goto reurnPoint; }
        pcNext += strlen(NUM_OF_ALL_ENTRIES_KEY_STR "=");
        m_pp.numberInAllFiles = atoi(pcNext);

        m_pp.masked = false;
        pcNext = strstr(pLine,MASK_KEY_STR "=");
        if(pcNext){
            pcNext += strlen(MASK_KEY_STR "=");
            pcNext = strstr(pcNext,"true");
            if(pcNext){  // not false
                //char vcBuf[32];
                m_pp.masked = true;
                pcNext += (strlen("true")+1);
                m_pp.mp.unmaskTime = STRING_TO_EPOCH(pcNext,MASK_NO_EXPIRE_STRING);
                if(m_pp.mp.unmaskTime<0){m_pp.mp.unmaskTime=NON_EXPIRE_TIME;}
            }
        }

        break;

    case entryCreationType::fromUser:
        m_pp.numOfFilesIn = 0;
        m_pp.numberInAllFiles = 0;
        m_pp.creationTime = time(&m_pp.creationTime);

        m_pp.expirationTime = NON_EXPIRE_TIME;
        pcNext = strstr(pLine,EXPIRATION_STR "=");
        if(pcNext){
            pcNext += strlen(EXPIRATION_STR "=");
            m_pp.expirationTime = STRING_TO_EPOCH(pcNext,NON_EXPIRE_STRING);
            if((m_pp.expirationTime<0)&&(m_pp.expirationTime!=NON_EXPIRE_TIME)){m_pp.expirationTime = NON_EXPIRE_TIME;}
        }


        m_pp.masked = false;
        m_pp.mp.unmaskTime = NON_EXPIRE_TIME;
        pcNext = strstr(pLine,MASK_KEY_STR "=");
        if(pcNext){
            pcNext += strlen(NUM_OF_ALL_ENTRIES_KEY_STR "=");
            pcNext = strstr(pcNext,"true");
            if(pcNext){  // not false
                //char vcBuf[32];
                m_pp.masked = true;
                pcNext += (strlen(NUM_OF_ALL_ENTRIES_KEY_STR "=")+1);
                m_pp.mp.unmaskTime = STRING_TO_EPOCH(pcNext,MASK_NO_EXPIRE_STRING);
                if(m_pp.mp.unmaskTime<0){m_pp.mp.unmaskTime=NON_EXPIRE_TIME;}
            }
        }

        break;

    default:
        break;
    }

    m_nNumOfErrors = 0;

    return;

reurnPoint:

    if(nError != errorsFromConstructor::noError){
        // some common stuff
        m_pp.collect=false;

        if(a_creationType == entryCreationType::fromUser){
            free(m_daqName);
            throw nError;
        }
    }
    else{m_pp.collect=true;}

}


pitz::daq::SingleEntry::~SingleEntry()
{
    SetError(0);
    delete m_pDoocsProperty;
    free(this->m_daqName);
}

#define VALUE_FOR_DELETE            1u
#define VALUE_FOR_ADD_TO_ROOT       static_cast<uint32_t>(1<<4)
#define VALUE_FOR_ADD_TO_NETW       static_cast<uint32_t>(1<<8)
#define VALUE_FOR_UNKNOWN_STATE     static_cast<uint32_t>(1<<12)
#define NON_DELETABLE_BITS          (VALUE_FOR_ADD_TO_ROOT | VALUE_FOR_ADD_TO_NETW)

bool pitz::daq::SingleEntry::markEntryForDeleteAndReturnIfPossibleNow()
{
    //uint32_t nReturn = __atomic_fetch_add (&m_willBeDeletedOrAddedToRootAtomic,VALUE_FOR_DELETE,__ATOMIC_RELAXED);
    //uint32_t nReturn = __atomic_fetch_and (&m_willBeDeletedOrAddedToRootAtomic,VALUE_FOR_DELETE,__ATOMIC_RELAXED);
    uint32_t nReturn = __atomic_fetch_or (&m_willBeDeletedOrAddedToRootAtomic,VALUE_FOR_DELETE,__ATOMIC_RELAXED);

    //if(!nReturn){
    //    return true;
    //}
    //
    //if(nReturn&VALUE_FOR_DELETE){
    //    __atomic_fetch_sub(&m_willBeDeletedOrAddedToRootAtomic,VALUE_FOR_DELETE,__ATOMIC_RELAXED);
    //}
    //
    ////__atomic_fetch_sub(&m_willBeDeletedOrAddedToRootAtomic,VALUE_FOR_DELETE,__ATOMIC_RELAXED);
    //return false;

    return nReturn ? false : true;
}


bool pitz::daq::SingleEntry::lockEntryForRoot()
{
    //uint32_t nReturn = __atomic_fetch_add (&m_willBeDeletedOrAddedToRootAtomic,VALUE_FOR_ADD_TO_ROOT,__ATOMIC_RELAXED);
    //uint32_t nReturn = __atomic_fetch_and (&m_willBeDeletedOrAddedToRootAtomic,VALUE_FOR_ADD_TO_ROOT,__ATOMIC_RELAXED);
    uint32_t nReturn = __atomic_fetch_or (&m_willBeDeletedOrAddedToRootAtomic,VALUE_FOR_ADD_TO_ROOT,__ATOMIC_RELAXED);

    if(!(nReturn&VALUE_FOR_DELETE)){
        return true;
    }

    __atomic_fetch_and (&m_willBeDeletedOrAddedToRootAtomic,~VALUE_FOR_ADD_TO_ROOT,__ATOMIC_RELAXED);
    //__atomic_fetch_sub(&m_willBeDeletedOrAddedToRootAtomic,VALUE_FOR_ADD_TO_ROOT,__ATOMIC_RELAXED);
    //__atomic_store_n(&m_willBeDeletedOrAddedToRootAtomic,nReturn,__ATOMIC_RELAXED);
    return false;
}


bool pitz::daq::SingleEntry::lockEntryForNetwork()
{
    //uint32_t nReturn = __atomic_fetch_add (&m_willBeDeletedOrAddedToRootAtomic,VALUE_FOR_ADD_TO_NETW,__ATOMIC_RELAXED);
    //uint32_t nReturn = __atomic_fetch_and (&m_willBeDeletedOrAddedToRootAtomic,VALUE_FOR_ADD_TO_NETW,__ATOMIC_RELAXED);
    uint32_t nReturn = __atomic_fetch_or (&m_willBeDeletedOrAddedToRootAtomic,VALUE_FOR_ADD_TO_NETW,__ATOMIC_RELAXED);

    if(!(nReturn&VALUE_FOR_DELETE)){
        return true;
    }

    //__atomic_fetch_sub(&m_willBeDeletedOrAddedToRootAtomic,VALUE_FOR_ADD_TO_NETW,__ATOMIC_RELAXED);
    __atomic_fetch_and (&m_willBeDeletedOrAddedToRootAtomic,~VALUE_FOR_ADD_TO_NETW,__ATOMIC_RELAXED);
    return false;
}


bool pitz::daq::SingleEntry::resetRootLockAndReturnIfDeletable()
{
    //uint32_t nReturn = __atomic_exchange_n (&m_willBeDeletedOrAddedToRootAtomic,0,__ATOMIC_RELAXED);
    //uint32_t nReturn = __atomic_load_n (&m_willBeDeletedOrAddedToRootAtomic,__ATOMIC_RELAXED);
    //uint32_t nReturn = __atomic_sub_fetch(&m_willBeDeletedOrAddedToRootAtomic,VALUE_FOR_ADD_TO_ROOT,__ATOMIC_RELAXED);
    uint32_t nReturn = __atomic_and_fetch (&m_willBeDeletedOrAddedToRootAtomic,~VALUE_FOR_ADD_TO_ROOT,__ATOMIC_RELAXED);

    if(nReturn == VALUE_FOR_DELETE){ return true; }

    //__atomic_fetch_sub(&m_willBeDeletedOrAddedToRootAtomic,VALUE_FOR_ADD_TO_ROOT,__ATOMIC_RELAXED);
    //__atomic_store_n(&m_willBeDeletedOrAddedToRootAtomic,nReturn,__ATOMIC_RELAXED);
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

    //__atomic_store_n(&m_willBeDeletedOrAddedToRootAtomic,nReturn,__ATOMIC_RELAXED);
    return false;
}



int pitz::daq::SingleEntry::SetEntryInfo(uint32_t a_unOffset, const DEC_OUT_PD(BranchDataRaw)& a_branchInfo)
{
    if((a_branchInfo.dataType != PITZ_DAQ_UNSPECIFIED_DATA_TYPE)&&(a_branchInfo.itemsCountPerEntry!=PITZ_DAQ_UNSPECIFIED_NUMBER_OF_SAMPLES)){
        uint32_t bufferNewSize (DaqEntryMemorySize(&a_branchInfo));
        uint32_t allocatedBufferNewSize = bufferNewSize+a_unOffset;

        if(allocatedBufferNewSize>m_unAllocatedBufferSize){
            DEC_OUT_PD(SingleData)* pNewData = ResizeDataWithNewOffset(m_pForRoot,a_unOffset,m_bufferSize);
            if(!pNewData){
                return -1;
            }
            m_pForRoot = pNewData;
            m_unAllocatedBufferSize = allocatedBufferNewSize;
        }

        m_unOffset = a_unOffset;
        m_bufferSize = bufferNewSize;

    }

    m_branchInfo = a_branchInfo;

    return 0;
}


void pitz::daq::SingleEntry::RemoveDoocsProperty()
{
    if(m_pDoocsProperty && m_pNetworkParent && m_pNetworkParent->parent()){
        m_pNetworkParent->parent()->rem_property(m_pDoocsProperty);
    }

    delete m_pDoocsProperty;
    m_pDoocsProperty = NEWNULLPTR2;
}


void pitz::daq::SingleEntry::SetNetworkParent(SNetworkStruct* a_pNetworkParent)
{

    if(a_pNetworkParent == m_pNetworkParent){return;}

    if(this->m_pDoocsProperty){
        m_pNetworkParent->parent()->rem_property(this->m_pDoocsProperty);
        delete this->m_pDoocsProperty;
        this->m_pDoocsProperty = NEWNULLPTR2;
    }

    if(m_pNetworkParent){
        m_pNetworkParent->RemoveEntryNoDelete(this);
    }

    m_pNetworkParent = a_pNetworkParent;

    if(a_pNetworkParent && a_pNetworkParent->parent()){
        char vcBuffer[512];
        snprintf(vcBuffer,511,"_ENTRY.%s",m_daqName ? m_daqName : "UNKNOWN");
        this->m_pDoocsProperty = new D_stringForEntry(vcBuffer,this);
        a_pNetworkParent->parent()->add_property(m_pDoocsProperty);
    }
}



//pitz::daq::SNetworkStruct* pitz::daq::SingleEntry::networkParent()
//{
//    return m_pNetworkParent;
//}


bool pitz::daq::SingleEntry::KeepEntry()const
{
    if(m_pp.expirationTime>0){
        time_t currentTime;
        currentTime = time(&currentTime);
        if(currentTime>=m_pp.expirationTime){
            return false;
        }
    }

    return true;
}


void pitz::daq::SingleEntry::MaskErrors(const char* a_maskResetTime)
{
    m_pp.mp.unmaskTime = STRING_TO_EPOCH(a_maskResetTime,MASK_NO_EXPIRE_STRING);
    if(m_pp.mp.unmaskTime<0){m_pp.mp.unmaskTime=NON_EXPIRE_TIME;}
    m_pp.masked = true;

    SetError(0);
}


void pitz::daq::SingleEntry::UnmaskErrors()
{
    m_pp.masked = false;
    m_pp.mp.unmaskTime = NON_EXPIRE_TIME;
}


int pitz::daq::SingleEntry::LastEventNumberHandled(void)const
{
    return m_nLastEventNumberHandled;
}


void pitz::daq::SingleEntry::SetLastEventNumberHandled(int a_nLastEventNumber)
{
    if(a_nLastEventNumber>m_nLastEventNumberHandled){m_nLastEventNumberHandled=a_nLastEventNumber;}
}


void pitz::daq::SingleEntry::SetProperty(const char* a_propertyAndAttributes)
{
    const char* pcNext = strstr(a_propertyAndAttributes,MASK_KEY_STR);
    if(pcNext){
        pcNext += strlen(MASK_KEY_STR);

        if(strncmp(pcNext,"=false",strlen("=false"))==0){
            UnmaskErrors();
        }
        else{
            size_t unStrLen(strlen("=true("));
            if(strncmp(pcNext,"=true(",unStrLen)==0){
                MaskErrors(pcNext+unStrLen);
            }
            else{
                MaskErrors(MASK_NO_EXPIRE_STRING);
            }
        } // else{ of if(strcmp(pcNext,"=false")){

    }  // if(pcNext){

    pcNext = strstr(a_propertyAndAttributes,"unmask");
    if(pcNext){UnmaskErrors();}

    pcNext = strstr(a_propertyAndAttributes,EXPIRATION_STR "=");

    if(pcNext){
        pcNext += strlen(EXPIRATION_STR "=");
        m_pp.expirationTime = STRING_TO_EPOCH(pcNext,NON_EXPIRE_STRING);
    }
}


void pitz::daq::SingleEntry::SetRootTreeAndBranchAddress(TTree* a_tree)
{
    m_pTreeOnRoot = a_tree;
    if(!a_tree){return;}
    ++m_pp.numOfFilesIn;
    m_nNumberInCurrentFile = 0;
    m_isPresentInCurrentFile = 0;
    if(m_pDoocsProperty){
        //m_pDoocsProperty->CalculateAndSetString();
    }
    m_nFillUnsavedCount = 0;

    //m_pBranchOnTree = m_pTreeOnRoot->Branch(a_cpcBranchName,NULL,this->rootFormatString());
    //m_pBranchOnTree = new MyTBranch(m_pTreeOnRoot,a_cpcBranchName,NULL,this->rootFormatString());
    //m_pBranchOnTree->SetTree(m_pTreeOnRoot);
    //return m_pBranchOnTree;

    if(m_pForRoot){
        m_pTreeOnRoot->Branch(this->daqName(),m_pForRoot,this->rootFormatString());
    }

}


DEC_OUT_PD(SingleData)* pitz::daq::SingleEntry::GetNewMemoryForNetwork()
{
    return CreateDataWithOffset(m_unOffset,m_bufferSize);
}


void pitz::daq::SingleEntry::SetMemoryBack( DEC_OUT_PD(SingleData)* a_pMemory )
{
    FreeDataWithOffset(a_pMemory,m_unOffset);
}


void pitz::daq::SingleEntry::SetNextFillableData(bool* a_pbAutosave, DEC_OUT_PD(SingleData)* a_pNewMemory)
{
    *a_pbAutosave = false;
    if(!m_pTreeOnRoot){return;}

    if(m_nFillUnsavedCount++>m_nMaxFillUnsavedCount){
        *a_pbAutosave = true;
        m_nFillUnsavedCount = 0;
    }

    memcpy(m_pForRoot,a_pNewMemory,m_bufferSize);
}


void pitz::daq::SingleEntry::AddExistanceInRootFile(int a_second, int a_eventNumber)
{
    if(!m_isPresentInCurrentFile){
        m_firstSecond = a_second;
        m_firstEventNumber = a_eventNumber;
        m_isPresentInCurrentFile = 1;
    }

    ++m_nNumberInCurrentFile;
    ++m_pp.numberInAllFiles;
    SetError(0);

    m_lastSecond = a_second;
    m_lastEventNumber = a_eventNumber;
}


void pitz::daq::SingleEntry::WriteContentToTheFile(FILE* a_fpFile)const
{
    char vcBufForCrt[32],vcBufForExp[32],vcBufForMask[32];

    fprintf(a_fpFile,
            "%s "
            NUM_OF_FILES_IN_KEY_STR "=%d; "
            NUM_OF_ALL_ENTRIES_KEY_STR "=%d; "
            CREATION_STR "=%s; "
            EXPIRATION_STR "=%s; "
            MASK_KEY_STR "=%s(%s); ",


            m_daqName,
            m_pp.numOfFilesIn,
            m_pp.numberInAllFiles,
            EPOCH_TO_STRING2(m_pp.creationTime,"",vcBufForCrt,31),
            EPOCH_TO_STRING2(m_pp.expirationTime,NON_EXPIRE_STRING,vcBufForExp,31),
            m_pp.masked?"true":"false",EPOCH_TO_STRING2(m_pp.mp.unmaskTime,MASK_NO_EXPIRE_STRING, vcBufForMask, 31));

    PermanentDataIntoFile(a_fpFile);
}


//CREATION_STR    "creation"
//#define EXPIRATION_STR    "expiration"
//#define ERROR_KEY_STR   "error"
//#define NUM_IN_CUR_FL_KEY_STR "numberInCurrentFile"
//#define NUM_OF_FILES_IN_KEY_STR "numOfFiles"
//#define NUM_OF_ALL_ENTRIES_KEY_STR "numOfAllEntries"
//#define NUM_OF_ERRORS_KEY_STR   "numOfErrors"

void pitz::daq::SingleEntry::ValueStringByKey2(const char* a_request, char* a_buffer, int a_bufferLength)const
{
    char* pcBufToWrite(a_buffer);
    int nWritten,nBufLen(a_bufferLength);

    bool bReadAll(false);

    if((!a_request)||(a_request[0]==0)){bReadAll=true;}

    if(bReadAll ||strstr(ERROR_KEY_STR,a_request)){
        nWritten = snprintf(pcBufToWrite,static_cast<size_t>(nBufLen),ERROR_KEY_STR "=%d; ",m_nError2);
        pcBufToWrite += nWritten;
        nBufLen -= nWritten;
        if(nBufLen<=0){return;}
    }

    if(bReadAll || strstr(NUM_IN_CUR_FL_KEY_STR,a_request)){
        nWritten = snprintf(pcBufToWrite,static_cast<size_t>(nBufLen),NUM_IN_CUR_FL_KEY_STR "=%d; ",m_nNumberInCurrentFile);
        pcBufToWrite += nWritten;
        nBufLen -= nWritten;
        if(nBufLen<=0){return;}
    }

    if(bReadAll || strstr(NUM_OF_FILES_IN_KEY_STR,a_request)){
        nWritten = snprintf(pcBufToWrite,static_cast<size_t>(nBufLen),NUM_OF_FILES_IN_KEY_STR "=%d; ",m_pp.numOfFilesIn);
        pcBufToWrite += nWritten;
        nBufLen -= nWritten;
        if(nBufLen<=0){return;}
    }

    if(bReadAll || strstr(NUM_OF_ALL_ENTRIES_KEY_STR,a_request)){
        nWritten = snprintf(pcBufToWrite,static_cast<size_t>(nBufLen),NUM_OF_ALL_ENTRIES_KEY_STR "=%d; ",m_pp.numberInAllFiles);
        pcBufToWrite += nWritten;
        nBufLen -= nWritten;
        if(nBufLen<=0){return;}
    }

    if(bReadAll || strstr(NUM_OF_ERRORS_KEY_STR,a_request)){
        nWritten = snprintf(pcBufToWrite,static_cast<size_t>(nBufLen),NUM_OF_ERRORS_KEY_STR "=%d; ",m_nNumOfErrors);
        pcBufToWrite += nWritten;
        nBufLen -= nWritten;
        if(nBufLen<=0){return;}
    }

    if( bReadAll || strstr(CREATION_STR,a_request)){
        char vcBufForTime[32];
        nWritten = snprintf(pcBufToWrite,static_cast<size_t>(nBufLen),CREATION_STR "=%s; ",EPOCH_TO_STRING2(m_pp.creationTime,"",vcBufForTime,31));
        pcBufToWrite += nWritten;
        nBufLen -= nWritten;
        if(nBufLen<=0){return;}
    }

    if(bReadAll || strstr(EXPIRATION_STR,a_request)){
        char vcBufForTime[32];
        nWritten = snprintf(pcBufToWrite,static_cast<size_t>(nBufLen),EXPIRATION_STR "=%s; ",EPOCH_TO_STRING2(m_pp.expirationTime,NON_EXPIRE_STRING,vcBufForTime,31));
        pcBufToWrite += nWritten;
        nBufLen -= nWritten;
        if(nBufLen<=0){return;}
    }

    if(bReadAll || strstr(MASK_KEY_STR,a_request)){
        char vcBufForTime[32];
        nWritten = snprintf(pcBufToWrite,static_cast<size_t>(nBufLen),MASK_KEY_STR "=%s(%s); ",m_pp.masked?"true":"false",EPOCH_TO_STRING2(m_pp.mp.unmaskTime,MASK_NO_EXPIRE_STRING, vcBufForTime, 31));
        pcBufToWrite += nWritten;
        nBufLen -= nWritten;
        if(nBufLen<=0){return;}
    }

    ValueStringByKeyInherited(bReadAll,a_request,pcBufToWrite,nBufLen);
    return;

}


void pitz::daq::SingleEntry::SetError(int a_error)
{
    if((!a_error)&&(!m_nError2)){return;}
    if(a_error && m_pp.masked){return;}

    EqFctCollector* pClc = m_pNetworkParent?m_pNetworkParent->parent():NEWNULLPTR2;

    if(a_error&&(!m_nError2)){
        ++m_nNumOfErrors;        
        if(pClc){pClc->IncrementErrors(m_daqName);}
    }
    else if((a_error==0)&&m_nError2){
        if(pClc){pClc->DecrementErrors(m_daqName);}
    }

    m_nError2=a_error;
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
    StopThread();
    for(auto pEntry : m_daqEntries){
        delete pEntry;
    }
}


void pitz::daq::SNetworkStruct::StopThread()
{
    if(m_pThread){
        pthread_t handleToThread = static_cast<pthread_t>(m_pThread->native_handle());

        m_shouldRun = 0;
        pthread_kill(handleToThread,SIGNAL_FOR_CANCELATION);
        m_pThread->join();
        delete m_pThread;
        m_pThread = NEWNULLPTR2;
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


void pitz::daq::SNetworkStruct::RemoveEntryNoDelete(SingleEntry *a_newEntry)
{
    if(!a_newEntry){return ;}
    m_daqEntries.erase(a_newEntry->m_thisIter);
}


bool pitz::daq::SNetworkStruct::AddNewEntry(SingleEntry *a_newEntry)
{
    if(!a_newEntry){return false;}
    m_daqEntries.push_front(a_newEntry);
    a_newEntry->m_thisIter = m_daqEntries.begin();
    a_newEntry->SetNetworkParent(this);

    return true;
}


/*////////////////////////////////////*/
pitz::daq::D_stringForEntry::D_stringForEntry(const char* a_pn, SingleEntry* a_parent)
        :
        D_BASE_FOR_STR(a_pn,NEWNULLPTR2),
        m_pParent(a_parent)
{
}


pitz::daq::D_stringForEntry::~D_stringForEntry()
{
}


void pitz::daq::D_stringForEntry::get(EqAdr* /*a_dcsAddr*/, EqData* a_dataFromUser, EqData* a_dataToUser,EqFct* /*a_loc*/)
{
    char vcBuffer[4096];
    char vcFromUser[512];
    //bool bFound = m_pParent->ValueStringByKey(dataFromUser,vcBuffer,511);

    a_dataFromUser->get_string(vcFromUser,511);

    m_pParent->ValueStringByKey2(vcFromUser,vcBuffer,4095);

    a_dataToUser->set(vcBuffer);
}


void pitz::daq::D_stringForEntry::set(EqAdr* a_dcsAddr, EqData* a_dataFromUser, EqData* a_dataToUser,EqFct* a_loc)
{

    char vcBuffer[1024];

    a_dataFromUser->get_string(vcBuffer,1023);

    m_pParent->SetProperty(vcBuffer);

    D_BASE_FOR_STR::set(a_dcsAddr, a_dataFromUser, a_dataToUser,a_loc);

#if 0
    std::string dataFromUser = a_dataFromUser->get_string();
    char vcBuffer[512];
    bool bFound = m_pParent->ValueStringByKey(dataFromUser,vcBuffer,511);

    CalculateAndSetString();

    if(bFound){
        a_dataToUser->set(vcBuffer);
    }
    else{
        D_BASE_FOR_STR::get(a_dcsAddr, a_dataFromUser, a_dataToUser,a_loc);
    }
#endif

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

}}

