
// pitz_daq_singleentry.hpp
// 2017 Sep 15

#ifndef PITZ_DAQ_SINGLEENTRY_HPP
#define PITZ_DAQ_SINGLEENTRY_HPP

#include "common/common_fifofast.hpp"
#include <pitz/daq/data/memory/forserver.hpp>
#include <cpp11+/thread_cpp11.hpp>
#include "eq_fct.h"
#include "TTree.h"
#include <time.h>
#include <signal.h>
#include <stdint.h>

#define NON_EXPIRE_TIME ((time_t)0)
#define NON_EXPIRE_STRING "never"
#define MASK_NO_EXPIRE_STRING "never"

#define CREATION_STR    "creation"
#define EXPIRATION_STR    "expiration"
#define ERROR_KEY_STR   "error"
#define NUM_IN_CUR_FL_KEY_STR "entries"
#define NUM_OF_FILES_IN_KEY_STR "files"
#define NUM_OF_ALL_ENTRIES_KEY_STR "allentries"
#define NUM_OF_ERRORS_KEY_STR   "errorsnumber"
#define MASK_KEY_STR    "mask"
#define POSIIBLE_TERM_SYMBOLS   " \t\",;\n"

#define STACK_SIZE          32
#define SIGNAL_FOR_CANCELATION  SIGTSTP
#define NUMBER_OF_PENDING_PACKS 4

namespace pitz{ namespace daq{

class SNetworkStruct;
class SingleEntry;

namespace entryCreationType{enum Type{fromOldFile,fromConfigFile,fromUser};}
namespace errorsFromConstructor{enum Error{noError=0,syntax=1,lowMemory, type};}

time_t STRING_TO_EPOCH(const char* _a_string,const char* a_cpcInf);
const char* EPOCH_TO_STRING2(const time_t& a_epoch, const char* a_cpcInf, char* a_buffer, int a_bufferLength);

#define D_BASE_FOR_STR  D_text

class D_stringForEntry : public D_BASE_FOR_STR
{
public:
    D_stringForEntry(const char* pn, SingleEntry* parent);
    ~D_stringForEntry();

private:
    void    get (EqAdr * dcsAddr, EqData * dataFromUser, EqData * dataToUser, EqFct * location);
    void    set (EqAdr * dcsAddr, EqData * dataFromUser, EqData * dataToUser, EqFct * location);

private:
    SingleEntry* m_pParent;
};


struct SMaskParam
{
    time_t unmaskTime;
};


struct SPermanentParams2
{
    int         numOfFilesIn;
    int         numberInAllFiles;
    time_t      expirationTime;
    time_t      creationTime;
    bool        masked;
    bool        collect;
    SMaskParam  mp;
};


class SingleEntry
{
    //friend class D_stringForEntry;
    friend class SNetworkStruct;
    friend class EqFctCollector;
protected:
    virtual ~SingleEntry();
public:
    SingleEntry(entryCreationType::Type creationType,const char* entryLine);

    virtual const char* rootFormatString()const=0;
    virtual pitz::daq::data::memory::ForServerBase* CreateMemoryInherit()=0;
    virtual void PermanentDataIntoFile(FILE* fpFile)const=0;

private:
    virtual void ValueStringByKeyInherited(bool bReadAll, const char* request, char* buffer, int bufferLength)const=0;

public:
    //m_pBranchOnTree
    void SetBranchAddress(bool* pbTimeToSave, pitz::daq::data::memory::ForServerBase* pNewMemory);
    pitz::daq::SNetworkStruct* networkParent();
    void SetRootTree3(TTree* tree, const char* a_cpcBranchName);
    TTree* tree2(){return m_pTreeOnRoot;}
    void SetError(int a_error);
    const char* daqName()const{return m_daqName;}
    //void copyForRoot(const MemoryBase* a_cM){m_forRoot->copyFrom(a_cM);}
    void AddExistanceInRootFile(int second, int eventNumber);
    int firstSecond()const{return m_firstSecond;}
    int firstEventNumber()const{return m_firstEventNumber;}
    int lastSecond()const{return m_lastSecond;}
    int lastEventNumber()const{return m_lastEventNumber;}
    bool isPresent()const{return m_isPresent;}
    void RemoveDoocsProperty();
    void WriteContentToTheFile(FILE* fpFile)const;
    bool KeepEntry()const;
    void MaskErrors(const char* maskResetTime);
    void UnmaskErrors();
    // APIs for DOOCS property
    void ValueStringByKey2(const char* request, char* buffer, int bufferLength)const;
    void SetProperty(const char* propertyAndAttributes);
    int LastEventNumberHandled(void)const;
    void SetLastEventNumberHandled(int a_nLastEventNumber);

    // This API will be used only by
private:
    bool CreateAllMemories(); // called after constructor
    void SetNetworkParent(SNetworkStruct* a_pNetworkParent);
        
public:
    SingleEntry *next,*prev;
    common::SimpleStack<pitz::daq::data::memory::ForServerBase*,STACK_SIZE>  stack;

private:
    char*                   m_daqName;
    data::memory::ForServerBase*  m_forRoot;
    bool                    m_isPresent;
    int                     m_firstEventNumber,m_lastEventNumber;
    int                     m_firstSecond,m_lastSecond;
    D_stringForEntry*       m_pDoocsProperty;
    SNetworkStruct*         m_pNetworkParent;
    TTree*                  m_pTreeOnRoot;
    TBranch*                m_pBranchOnTree;
    // new for property
    int                     m_nNumberInCurrentFile;
    int                     m_nNumOfErrors;
    int                     m_nError2;
    int                     m_nFillUnsavedCount;
    int                     m_nMaxFillUnsavedCount;
    int                     m_nLastEventNumber;

    uint32_t                m_isMemoriesInited;

    SPermanentParams2       m_pp;

protected:
    SingleEntry(const SingleEntry&){}
};


class SNetworkStruct
{    
public:
    SNetworkStruct(EqFct* parent, SNetworkStruct* prev);
    virtual ~SNetworkStruct();

    SingleEntry* first();
    SingleEntry* last();

    EqFct*  parent();

    bool AddNewEntry(SingleEntry *newEntry);
    pitz::daq::SingleEntry* RemoveEntry(SingleEntry *entry,int* numberOfEntriesRemained=NULL);
    void SetThread(STDN::thread* thread);
    void StopAndDeleteThread();
    SNetworkStruct* prev(){return m_prev;}
    SNetworkStruct* next(){return m_next;}

private:
    SNetworkStruct(const SNetworkStruct&){}

private:
    EqFct* m_pParent;
    SingleEntry *m_first, *m_last;
    STDN::thread* m_pThread;
    SNetworkStruct *m_prev, *m_next;
    int m_nNumberOfEntries;

};

}}

#endif // PITZ_DAQ_SINGLEENTRY_HPP
