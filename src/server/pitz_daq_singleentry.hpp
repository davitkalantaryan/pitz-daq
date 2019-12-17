
// pitz_daq_singleentry.hpp
// 2017 Sep 15

#ifndef PITZ_DAQ_SINGLEENTRY_HPP
#define PITZ_DAQ_SINGLEENTRY_HPP

//#include "common/common_fifofast.hpp"
//#include <pitz/daq/data/memory/forserver.hpp>
#include <cpp11+/thread_cpp11.hpp>
#include "eq_fct.h"
#include "TTree.h"
#include <time.h>
#include <signal.h>
#include <stdint.h>
#include <pitz_daq_internal.h>
#include <pitz_daq_data_handling.h>
#include <pitz_daq_data_handling_daqdev.h>
#include <list>

#ifndef PITZ_DAQ_UNSPECIFIED_DATA_TYPE
#define PITZ_DAQ_UNSPECIFIED_DATA_TYPE  -1
#endif  // #ifndef PITZ_DAQ_UNKNOWN_DATA_TYPE

#ifndef PITZ_DAQ_UNSPECIFIED_NUMBER_OF_SAMPLES
#define PITZ_DAQ_UNSPECIFIED_NUMBER_OF_SAMPLES  -1
#endif  // #ifndef PITZ_DAQ_UNKNOWN_DATA_TYPE

#define NON_EXPIRE_TIME             STATIC_CAST2(time_t,0)
#define NON_EXPIRE_STRING           "never"
#define MASK_NO_EXPIRE_STRING       "never"

#define CREATION_STR                "creation"
#define EXPIRATION_STR              "expiration"
#define ERROR_KEY_STR               "error"
#define NUM_IN_CUR_FL_KEY_STR       "entries"
#define NUM_OF_FILES_IN_KEY_STR     "files"
#define NUM_OF_ALL_ENTRIES_KEY_STR  "allentries"
#define NUM_OF_ERRORS_KEY_STR       "errorsnumber"
#define MASK_KEY_STR                "mask"
#define POSIIBLE_TERM_SYMBOLS       " \t\",;\n"

#define STACK_SIZE                  32
#define SIGNAL_FOR_CANCELATION      SIGTSTP
#define NUMBER_OF_PENDING_PACKS     4


namespace pitz{ namespace daq{

class SNetworkStruct;
class SingleEntry;
class EqFctCollector;

typedef const char* TypeConstCharPtr;

namespace entryCreationType{enum Type{fromOldFile,fromConfigFile,fromUser,unknownCreation};}
namespace errorsFromConstructor{enum Error{noError=0,syntax=1,lowMemory, type,doocsUnreachable};}

time_t STRING_TO_EPOCH(const char* _a_string,const char* a_cpcInf);
const char* EPOCH_TO_STRING2(const time_t& a_epoch, const char* a_cpcInf, char* a_buffer, int a_bufferLength);

#define D_BASE_FOR_STR  D_text

//class D_stringForEntry : public D_BASE_FOR_STR
//{
//public:
//    D_stringForEntry(const char* pn, SingleEntry* parent);
//    ~D_stringForEntry();
//
//private:
//    void    get (EqAdr * dcsAddr, EqData * dataFromUser, EqData * dataToUser, EqFct * location);
//    void    set (EqAdr * dcsAddr, EqData * dataFromUser, EqData * dataToUser, EqFct * location);
//
//private:
//    SingleEntry* m_pParent;
//};


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
    uint64_t    masked : 1;
    uint64_t    collect : 1;
    uint64_t    reserved64bit : 62;
    SMaskParam  mp;
};


class SingleEntry : protected D_BASE_FOR_STR
{
    //friend class D_stringForEntry;
    friend class SNetworkStruct;
    //friend class EqFctCollector;
protected:
public:
    virtual ~SingleEntry() OVERRIDE2;
public:
    SingleEntry( /*DEC_OUT_PD(BOOL2) a_bDubRootString,*/ entryCreationType::Type creationType,const char* entryLine, TypeConstCharPtr* a_pHelper);

    virtual const char* rootFormatString()const=0;
    virtual void PermanentDataIntoFile(FILE* fpFile)const=0;

private:
    virtual void ValueStringByKeyInherited(bool bReadAll, const char* request, char* buffer, int bufferLength)const=0;

public:
    virtual void SetMemoryBack( DEC_OUT_PD(SingleData)* pMemory );
    virtual DEC_OUT_PD(SingleData)* GetNewMemoryForNetwork();

protected:
    virtual void CleanEntryNoFreeInheritable();

public:
    bool  markEntryForDeleteAndReturnIfPossibleNow();
    bool  lockEntryForRoot();
    bool  lockEntryForNetwork();
    bool  resetRootLockAndReturnIfDeletable();
    bool  resetNetworkLockAndReturnIfDeletable();
    bool  isLockedByRootOrNetwork()const;
    SNetworkStruct* CleanEntryNoFree();

public:
    void SetNextFillableData(DEC_OUT_PD(SingleData)* pNewMemory);
    //pitz::daq::SNetworkStruct* networkParent(){return m_pNetworkParent;}
    //void SetRootTreeAndBranchAddress(TTree* tree, const char* a_cpcBranchName);
    void SetRootTreeAndBranchAddress(TTree* tree);
    TTree* rootTree(){return m_pTreeOnRoot;}
    void SetError(int a_error);
    const char* daqName()const{return m_daqName;}
    //void copyForRoot(const MemoryBase* a_cM){m_forRoot->copyFrom(a_cM);}
    void AddExistanceInRootFile(int second, int eventNumber);
    int firstSecond()const{return m_firstSecond;}
    int firstEventNumber()const{return m_firstEventNumber;}
    int lastSecond()const{return m_lastSecond;}
    int lastEventNumber()const{return m_lastEventNumber;}
    uint64_t isPresentInCurrentFile()const{return m_isPresentInCurrentFile;}
    //void RemoveDoocsProperty();
    void WriteContentToTheFile(FILE* fpFile)const;
    //bool KeepEntry2()const;
    void MaskErrors(const char* maskResetTime);
    void UnmaskErrors();
    // APIs for DOOCS property
    void ValueStringByKey2(const char* request, char* buffer, int bufferLength)const;
    void SetProperty(const char* propertyAndAttributes);
    int LastEventNumberHandled(void)const;
    void SetLastEventNumberHandled(int a_nLastEventNumber);
    void RemoveDoocsProperty2();

    // This API will be used only by
protected:
    //int  SetEntryInfo(uint32_t a_unOffset, const DEC_OUT_PD(BranchDataRaw)& a_branchInfo);
    char*  ApplyEntryInfo( DEC_OUT_PD(BOOL2) a_bDubRootString );
    void   SetNetworkParent(SNetworkStruct* a_pNetworkParent);
        

private:
    char*                                   m_daqName;
    int                                     m_firstEventNumber,m_lastEventNumber;
    int                                     m_firstSecond,m_lastSecond;
    //D_stringForEntry*                       m_pDoocsProperty;
    SNetworkStruct*                         m_pNetworkParent;
    TTree*                                  m_pTreeOnRoot;
    TBranch*                                m_pBranchOnTree;
    // new for property
    int                                     m_nNumberInCurrentFile;
    int                                     m_nNumOfErrors;
    int                                     m_nError2;
    int                                     m_nLastEventNumberHandled;

    SPermanentParams2                       m_pp;
protected:
    ::std::list< SingleEntry* >::iterator   m_thisIter;
    DEC_OUT_PD(BranchDataRaw)               m_branchInfo;
    DEC_OUT_PD(SingleData)*                 m_pForRoot;
    uint32_t                                m_totalRootBufferSize;
    uint32_t                                m_onlyNetDataBufferSize;
    uint32_t                                m_unOffset;
    uint32_t                                m_unAllocatedBufferSize;
    mutable uint32_t                        m_willBeDeletedOrAddedToRootAtomic ;

    uint64_t                                m_isPresentInCurrentFile : 1;
    uint64_t                                m_isCleanEntryInheritableCalled : 1;
    uint64_t                                m_bitwise64Reserved : 62;


protected:
    SingleEntry(const SingleEntry&) = delete ;
};


class SNetworkStruct
{
    friend class EqFctCollector;
    friend class SingleEntry;
public:
    SNetworkStruct(EqFctCollector* parent);
    virtual ~SNetworkStruct();

    virtual void StopThreadAndClear();

    EqFctCollector*  parent();
    bool AddNewEntry(SingleEntry *newEntry);
    const ::std::list< SingleEntry* >& daqEntries()const;
    uint64_t shouldRun()const;

private:
    void DataGetterThread();
    //void RemoveEntryNoDeletePrivate(SingleEntry *entry);

private:
    SNetworkStruct(const SNetworkStruct&){}

protected:
    EqFctCollector*                             m_pParent;
    STDN::thread*                               m_pThread;
    uint64_t                                    m_shouldRun : 1;
    uint64_t                                    m_bitwise64Reserved : 63 ;
    ::std::list< SingleEntry* >                 m_daqEntries;
    ::std::list< SNetworkStruct* >::iterator    m_thisIter;

};

}}

#endif // PITZ_DAQ_SINGLEENTRY_HPP
