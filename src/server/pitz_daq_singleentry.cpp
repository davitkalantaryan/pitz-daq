//include "pitz_daq_singleentry.h"
// pitz_daq_singleentry.cpp
// 2017 Sep 15


#include "pitz_daq_singleentry.hpp"
#include <string.h>
#include <stdlib.h>
#include "pitz_daq_collectorproperties.hpp"
#include "pitz_daq_eqfctcollector.hpp"

#define DATA_SIZE_TO_SAVE   50000  // 40 kB
#define MIN_NUMBER_OF_FILLS 20


pitz::daq::SingleEntry::SingleEntry(entryCreationType::Type a_creationType,const char* a_entryLine)
        :
        next(NULL),
        prev(NULL),
        m_daqName(NULL),
        //m_forRoot(NULL),
        m_pDoocsProperty(NULL),
        m_pNetworkParent(NULL),
        m_pTreeOnRoot(NULL),
        m_pBranchOnTree(NULL)
{
    const char* pLine = strpbrk(a_entryLine,POSIIBLE_TERM_SYMBOLS);
    const char* pcNext ;
    size_t strStart;
    size_t daqNameLen;
    int nError(errorsFromConstructor::noError);

    if(!pLine){throw errorsFromConstructor::syntax;}

    m_isMemoriesInited = 0;
    m_nLastEventNumberHandled = 0;

    daqNameLen = (size_t)(pLine-a_entryLine);
    m_daqName = (char*)malloc(daqNameLen+1);
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
        if((m_pp.creationTime<0)&&(m_pp.creationTime!=NON_EXPIRE_TIME)){nError = -((int)m_pp.creationTime);goto reurnPoint;}

        pcNext = strstr(pLine,EXPIRATION_STR "=");
        if(!pcNext){nError = errorsFromConstructor::syntax;goto reurnPoint; }
        pcNext += strlen(EXPIRATION_STR "=");
        m_pp.expirationTime = STRING_TO_EPOCH(pcNext,NON_EXPIRE_STRING);
        if((m_pp.expirationTime<0)&&(m_pp.expirationTime!=NON_EXPIRE_TIME)){nError = -((int)m_pp.expirationTime);goto reurnPoint;}

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

    if(!m_isMemoriesInited){return;}

    for(int i=0; i<STACK_SIZE;++i){
        delete stack[i];
        stack[i] = NULL;
    }
    delete m_forRoot;

}


void pitz::daq::SingleEntry::RemoveDoocsProperty()
{
    if(m_pDoocsProperty && m_pNetworkParent && m_pNetworkParent->parent()){
        EqFct* pFct = (EqFct*)m_pNetworkParent->parent();
        pFct->rem_property(m_pDoocsProperty);
    }

    delete m_pDoocsProperty;
    m_pDoocsProperty = NULL;
}


bool pitz::daq::SingleEntry::CreateAllMemories() // called after constructor
{
    int i, nToDelete;

    if(m_isMemoriesInited){return true;}

    m_forRoot=CreateMemoryInherit();

    for(i=0; i<STACK_SIZE;++i){
        stack[i] = CreateMemoryInherit();
        if(!stack[i]){
            //destroyEntry();
            nToDelete = i;
            goto falseReturnPoint; //"Low memory 2";
        }
        stack[i]->SetParent(this);
    }

    m_nMaxFillUnsavedCount = DATA_SIZE_TO_SAVE/stack[0]->streamSize();
    if(m_nMaxFillUnsavedCount<MIN_NUMBER_OF_FILLS){m_nMaxFillUnsavedCount=MIN_NUMBER_OF_FILLS;}


    m_isMemoriesInited = 1;
    return true;

falseReturnPoint:

    for(i=0; i<nToDelete;++i){

        delete stack[i];
    }

    //delete m_forRoot;m_forRoot = NULL;
    m_isMemoriesInited = 0;

    return false;
}


void pitz::daq::SingleEntry::SetNetworkParent(SNetworkStruct* a_pNetworkParent)
{

    if(a_pNetworkParent == m_pNetworkParent){return;}

    if(this->m_pDoocsProperty){
        m_pNetworkParent->parent()->rem_property(this->m_pDoocsProperty);
        delete this->m_pDoocsProperty;
        this->m_pDoocsProperty = NULL;
    }

    m_pNetworkParent = a_pNetworkParent;

    if(a_pNetworkParent && a_pNetworkParent->parent()){
        char vcBuffer[512];
        snprintf(vcBuffer,511,"_ENTRY.%s",m_daqName ? m_daqName : "UNKNOWN");
        this->m_pDoocsProperty = new D_stringForEntry(vcBuffer,this);
        a_pNetworkParent->parent()->add_property(m_pDoocsProperty);
    }
}



pitz::daq::SNetworkStruct* pitz::daq::SingleEntry::networkParent()
{
    return m_pNetworkParent;
}


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

#if 0
class MyTBranch : public TBranch
{
    public:
    MyTBranch(TTree* a_tree, const char* a_name, void* a_address, const char* a_leaflist)
            :
            TBranch(a_tree, a_name, a_address, a_leaflist)
    {
    }

    void SetAddress(void* a_add){
        //printf("!!!!!!! Setting address %p\n", a_add);
        TBranch::ResetAddress();
        TBranch::SetAddress(a_add);
        this->fAddress = (char*)a_add;
    }
};
#endif


void pitz::daq::SingleEntry::SetRootTree3(TTree* a_tree, const char* a_cpcBranchName)
{
    m_pTreeOnRoot = a_tree;
    ++m_pp.numOfFilesIn;
    m_nNumberInCurrentFile = 0;
    m_isPresent = false;
    if(m_pDoocsProperty){
        //m_pDoocsProperty->CalculateAndSetString();
    }
    m_nFillUnsavedCount = 0;

    //m_pBranchOnTree = m_pTreeOnRoot->Branch(a_cpcBranchName,NULL,this->rootFormatString());
    //m_pBranchOnTree = new MyTBranch(m_pTreeOnRoot,a_cpcBranchName,NULL,this->rootFormatString());
    //m_pBranchOnTree->SetTree(m_pTreeOnRoot);
    //return m_pBranchOnTree;

    m_pTreeOnRoot->Branch(a_cpcBranchName,m_forRoot->bufferForRoot(),this->rootFormatString());
}


void pitz::daq::SingleEntry::SetBranchAddress(bool* a_pbAutosave, data::memory::ForServerBase* a_pNewMemory)
{
    *a_pbAutosave = false;
    if(!m_pTreeOnRoot){return;}

    if(m_nFillUnsavedCount++>m_nMaxFillUnsavedCount){
        *a_pbAutosave = true;
        m_nFillUnsavedCount = 0;
    }

    //return m_pBranchOnTree;
    m_forRoot->copyFrom(a_pNewMemory);
}


void pitz::daq::SingleEntry::AddExistanceInRootFile(int a_second, int a_eventNumber)
{
    if(!m_isPresent){
        m_firstSecond = a_second;
        m_firstEventNumber = a_eventNumber;
        m_isPresent = true;
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

    if((a_request==NULL)||(a_request[0]==0)){bReadAll=true;}

    if(bReadAll ||strstr(ERROR_KEY_STR,a_request)){
        nWritten = snprintf(pcBufToWrite,nBufLen,ERROR_KEY_STR "=%d; ",m_nError2);
        pcBufToWrite += nWritten;
        nBufLen -= nWritten;
        if(nBufLen<=0){return;}
    }

    if(bReadAll || strstr(NUM_IN_CUR_FL_KEY_STR,a_request)){
        nWritten = snprintf(pcBufToWrite,nBufLen,NUM_IN_CUR_FL_KEY_STR "=%d; ",m_nNumberInCurrentFile);
        pcBufToWrite += nWritten;
        nBufLen -= nWritten;
        if(nBufLen<=0){return;}
    }

    if(bReadAll || strstr(NUM_OF_FILES_IN_KEY_STR,a_request)){
        nWritten = snprintf(pcBufToWrite,nBufLen,NUM_OF_FILES_IN_KEY_STR "=%d; ",m_pp.numOfFilesIn);
        pcBufToWrite += nWritten;
        nBufLen -= nWritten;
        if(nBufLen<=0){return;}
    }

    if(bReadAll || strstr(NUM_OF_ALL_ENTRIES_KEY_STR,a_request)){
        nWritten = snprintf(pcBufToWrite,nBufLen,NUM_OF_ALL_ENTRIES_KEY_STR "=%d; ",m_pp.numberInAllFiles);
        pcBufToWrite += nWritten;
        nBufLen -= nWritten;
        if(nBufLen<=0){return;}
    }

    if(bReadAll || strstr(NUM_OF_ERRORS_KEY_STR,a_request)){
        nWritten = snprintf(pcBufToWrite,nBufLen,NUM_OF_ERRORS_KEY_STR "=%d; ",m_nNumOfErrors);
        pcBufToWrite += nWritten;
        nBufLen -= nWritten;
        if(nBufLen<=0){return;}
    }

    if( bReadAll || strstr(CREATION_STR,a_request)){
        char vcBufForTime[32];
        nWritten = snprintf(pcBufToWrite, nBufLen,CREATION_STR "=%s; ",EPOCH_TO_STRING2(m_pp.creationTime,"",vcBufForTime,31));
        pcBufToWrite += nWritten;
        nBufLen -= nWritten;
        if(nBufLen<=0){return;}
    }

    if(bReadAll || strstr(EXPIRATION_STR,a_request)){
        char vcBufForTime[32];
        nWritten = snprintf(pcBufToWrite, nBufLen,EXPIRATION_STR "=%s; ",EPOCH_TO_STRING2(m_pp.expirationTime,NON_EXPIRE_STRING,vcBufForTime,31));
        pcBufToWrite += nWritten;
        nBufLen -= nWritten;
        if(nBufLen<=0){return;}
    }

    if(bReadAll || strstr(MASK_KEY_STR,a_request)){
        char vcBufForTime[32];
        nWritten = snprintf(pcBufToWrite,nBufLen,MASK_KEY_STR "=%s(%s); ",m_pp.masked?"true":"false",EPOCH_TO_STRING2(m_pp.mp.unmaskTime,MASK_NO_EXPIRE_STRING, vcBufForTime, 31));
        pcBufToWrite += nWritten;
        nBufLen -= nWritten;
        if(nBufLen<=0){return;}
    }

    ValueStringByKeyInherited(bReadAll,a_request,pcBufToWrite,nBufLen);
    return;

}


void pitz::daq::SingleEntry::SetError(int a_error)
{
    if((a_error == 0)&&(m_nError2==0)){return;}

    if(a_error&&(m_nError2==0)){
        if(m_pp.masked){return;}
        ++m_nNumOfErrors;
        EqFctCollector* pClc = m_pNetworkParent?(EqFctCollector*)m_pNetworkParent->parent():NULL;
        if(pClc){pClc->IncrementErrors(m_daqName);}
    }
    else if((a_error==0)&&m_nError2){
        EqFctCollector* pClc = m_pNetworkParent?(EqFctCollector*)m_pNetworkParent->parent():NULL;
        if(pClc){pClc->DecrementErrors(m_daqName);}
    }

    m_nError2=a_error;
}


/*//////////////////////////////////////////////////*/

pitz::daq::SNetworkStruct::SNetworkStruct(EqFct* a_parent, SNetworkStruct* a_prev)
        :
        m_pParent(a_parent),
        m_prev(a_prev),
        m_next(NULL)
{
    if(a_prev){a_prev->m_next = this;}
    m_pThread = NULL;
    m_last = NULL;
    if(a_prev){m_first = a_prev->m_last;} // keep last from prev
    else {m_first = NULL;}
    m_nNumberOfEntries = 0;
}


pitz::daq::SNetworkStruct::~SNetworkStruct()
{
    if(m_prev){m_prev->m_next = m_next;}
    if(m_next){m_next->m_prev = m_prev;}
    StopAndDeleteThread();
}


pitz::daq::SingleEntry* pitz::daq::SNetworkStruct::first()
{
    return m_first;
}


//SingleEntry* last()
pitz::daq::SingleEntry* pitz::daq::SNetworkStruct::last()
{
    return m_last;
}


EqFct* pitz::daq::SNetworkStruct::parent()
{
    return m_pParent;
}


size_t pitz::daq::SNetworkStruct::numberOfEntries()const
{
    return static_cast<size_t>(m_nNumberOfEntries);
}


void pitz::daq::SNetworkStruct::SetThread(STDN::thread* a_pThread)
{
    StopAndDeleteThread();
    m_pThread = a_pThread;
}

void pitz::daq::SNetworkStruct::StopAndDeleteThread()
{
    if(m_pThread){
        pthread_t handleToThread = (pthread_t)m_pThread->native_handle();
        pthread_kill(handleToThread,SIGNAL_FOR_CANCELATION);

        m_pThread->join();
        delete m_pThread;
        m_pThread = NULL;
    }
}


pitz::daq::SingleEntry* pitz::daq::SNetworkStruct::RemoveEntry(SingleEntry *a_newEntry,int* a_pnNumberOfEntries)
{

    SingleEntry* pNext;
    if(!a_newEntry){return NULL;}
    pNext = a_newEntry->next;

    //.lock();
    if(a_newEntry->next){a_newEntry->next->prev=a_newEntry->prev;}
    if(a_newEntry->prev){a_newEntry->prev->next=a_newEntry->next;}
    if(a_newEntry == m_first){m_first=a_newEntry->next;}
    if(a_newEntry == m_last){m_last=a_newEntry->prev;}

    delete a_newEntry;

    if(a_pnNumberOfEntries){*a_pnNumberOfEntries = --m_nNumberOfEntries;}
    //mutex.unlock();

    return pNext;

}


bool pitz::daq::SNetworkStruct::AddNewEntry(SingleEntry *a_newEntry)
{
    //mutex.lock();

    if(!a_newEntry->CreateAllMemories()){
#ifndef NEW_GETTER_THREAD
        return false;
#endif
    }

    // case of first
    if(m_first && !m_last){m_first->next = a_newEntry;a_newEntry->prev = m_first;m_first=nullptr;}
    //a_newEntry->m_pNetworkParent = NULL;
    if(m_first){
        a_newEntry->prev = m_first->prev;
        if(m_first->prev){m_first->prev->next = a_newEntry;}
        m_first->prev = a_newEntry;
    }
    a_newEntry->next = m_first;
    if(!m_last){m_last=a_newEntry;}
    m_first = a_newEntry;

    ++m_nNumberOfEntries;
    a_newEntry->SetNetworkParent(this);

    //mutex.unlock();

    return true;
}


/*////////////////////////////////////*/
pitz::daq::D_stringForEntry::D_stringForEntry(const char* a_pn, SingleEntry* a_parent)
        :
        D_BASE_FOR_STR(a_pn,NULL),
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

    aTm.tm_year = strtol((_a_string),&pcNext,10) - 1900;
    if(((const char*)pcNext++)==(_a_string)){return -errorsFromConstructor::syntax;}

    pcTmp = pcNext;
    aTm.tm_mon = strtol(pcTmp,&pcNext,10) - 1;
    if(((const char*)pcNext++)==pcTmp){return -errorsFromConstructor::syntax;}

    pcTmp = pcNext;
    aTm.tm_mday = strtol(pcTmp,&pcNext,10);
    if(((const char*)pcNext++)==pcTmp){return -errorsFromConstructor::syntax;}

    pcTmp = pcNext;
    aTm.tm_hour = strtol(pcTmp,&pcNext,10) ;
    if(((const char*)pcNext++)==pcTmp){return -errorsFromConstructor::syntax;}

    pcTmp = pcNext;
    aTm.tm_min = strtol(pcTmp,&pcNext,10);
    if(((const char*)pcNext++)==pcTmp){return -errorsFromConstructor::syntax;}

    aTm.tm_sec = 0;
    aTm.tm_isdst = -1;

    return mktime(&aTm);

}


const char* EPOCH_TO_STRING2(const time_t& a_epoch, const char* a_cpcInf, char* a_buffer, int a_bufferLength)
{

    if((a_epoch>0) && (a_epoch!=NON_EXPIRE_TIME)){
        struct tm aTm;
        localtime_r(&a_epoch,&aTm);
        snprintf(a_buffer, a_bufferLength,"%d.%.2d.%.2d-%.2d:%.2d",aTm.tm_year+1900,aTm.tm_mon+1,aTm.tm_mday,aTm.tm_hour,aTm.tm_min);
    }
    else{
        snprintf(a_buffer, a_bufferLength,"%s", a_cpcInf);
    }

    return a_buffer;

}

}}

