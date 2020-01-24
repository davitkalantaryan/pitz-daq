
// pitz_daq_singleentry.hpp
// 2017 Sep 15

#ifndef PITZ_DAQ_SINGLEENTRY_HPP
#define PITZ_DAQ_SINGLEENTRY_HPP

#include <cpp11+/thread_cpp11.hpp>
#include <eq_fct.h>
#include <time.h>
#include <signal.h>
#include <stdint.h>
#include <pitz_daq_internal.h>
#include <pitz_daq_data_handling.h>
#include <pitz_daq_data_handling_daqdev.h>
#include <list>
#include <TTree.h>

#ifndef PITZ_DAQ_UNSPECIFIED_DATA_TYPE
#define PITZ_DAQ_UNSPECIFIED_DATA_TYPE  -1
#endif  // #ifndef PITZ_DAQ_UNKNOWN_DATA_TYPE

#ifndef PITZ_DAQ_UNSPECIFIED_NUMBER_OF_SAMPLES
#define PITZ_DAQ_UNSPECIFIED_NUMBER_OF_SAMPLES  -1
#endif  // #ifndef PITZ_DAQ_UNKNOWN_DATA_TYPE

#define NOT_MASKED_TO_TIME          STATIC_CAST2(time_t,-1)
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
#define MASK_COLLECTION_KEY_STR     "maskCollection"
#define MASK_ERRORS_KEY_STR         "maskErrors"
#define POSIIBLE_TERM_SYMBOLS       " \t\",;\n"

#define STACK_SIZE                  32
#define SIGNAL_FOR_CANCELATION      SIGTSTP
#define NUMBER_OF_PENDING_PACKS     4


namespace pitz{ namespace daq{

class SNetworkStruct;
class SingleEntry;
class EqFctCollector;
class TreeForSingleEntry;

typedef const char* TypeConstCharPtr;

namespace entryCreationType{enum Type{fromOldFile,fromConfigFile,fromUser,unknownCreation};}
namespace errorsFromConstructor{enum Error{noError=0,syntax=10,lowMemory, type,doocsUnreachable};}

//time_t STRING_TO_EPOCH(const char* _a_string,const char* a_cpcInf);
//const char* EPOCH_TO_STRING2(const time_t& a_epoch, const char* a_cpcInf, char* a_buffer, int a_bufferLength);

#define D_BASE_FOR_STR  D_text

namespace EntryParams{

class Base;

typedef void (*TypeClbk)(Base*,void*);

class Base
{
public:
    Base(const char* entryParamName, ::std::list<EntryParams::Base*>* pContainer);
    virtual ~Base();

    const char* paramName()const;
    bool        FindAndGetFromLine(const char* entryLine);
    size_t      WriteToLineBuffer(char* entryLineBuffer, size_t unBufferSize)const;
    void        SetParentAndClbk(void* pParent, TypeClbk fpClbk);

private:
    virtual bool   GetDataFromLine(const char* entryLine)=0;
    virtual size_t WriteDataToLineBuffer(char* entryLineBuffer, size_t unBufferSize)const=0;

protected:
    const char* m_paramName;
    TypeClbk    m_fpClbk;
    void*       m_pParent;
};

template <typename IntType>
class IntParam : public Base
{
public:
    IntParam(const char* entryParamName, ::std::list<EntryParams::Base*>* pContainer);
    virtual ~IntParam() OVERRIDE2 ;
    virtual bool   GetDataFromLine(const char* entryLine) OVERRIDE2;
    virtual size_t WriteDataToLineBuffer(char* entryLineBuffer, size_t unBufferSize)const OVERRIDE2;
    operator const IntType&()const;
    void operator=(const IntType& newValue);
    void operator++();

protected:
    IntType    m_value;
};

class Mask : public Base
{
public:
    Mask(const char* entryParamName, ::std::list<EntryParams::Base*>* pContainer);
    ~Mask() OVERRIDE2 ;

    bool   GetDataFromLine(const char* entryLine) OVERRIDE2;
    size_t WriteDataToLineBuffer(char* entryLineBuffer, size_t unBufferSize)const OVERRIDE2;
    time_t expirationTime()const;
    bool   isMasked()const;

private:
    time_t m_expirationTime;
};

} // namespace EntryParams{



class SingleEntry : protected D_BASE_FOR_STR
{
    friend class SNetworkStruct;
    friend class TreeForSingleEntry;
public:
    SingleEntry( /*DEC_OUT_PD(BOOL2) a_bDubRootString,*/ entryCreationType::Type creationType,const char* entryLine, TypeConstCharPtr* a_pHelper);
    virtual ~SingleEntry() OVERRIDE2;

    SNetworkStruct*  networkParent();
    uint64_t  isValid()const;
    void SetValid();
    void SetInvalid();

private:
    virtual const char* rootFormatString()const=0;
    void get(EqAdr* /*a_dcsAddr*/, EqData* a_dataFromUser, EqData* a_dataToUser,EqFct* /*a_loc*/) OVERRIDE2;
    void set(EqAdr* a_dcsAddr, EqData* a_dataFromUser, EqData* a_dataToUser,EqFct* a_loc) OVERRIDE2 ;

public:
    virtual DEC_OUT_PD(SingleData)* GetNewMemoryForNetwork() = 0;
    virtual void FreeUsedMemory(DEC_OUT_PD(SingleData)* usedMemory);

public:
    bool  markEntryForDeleteAndReturnIfPossibleNow();
    bool  lockEntryForRoot();
    bool  lockEntryForCurrentFile();
    bool  lockEntryForNetwork();
    bool  lockEntryForRootFile();
    bool  resetRootLockAndReturnIfDeletable();
    bool  resetNetworkLockAndReturnIfDeletable();
    bool  resetRooFileLockAndReturnIfDeletable();
    bool  isLocked()const;

public:
    void Fill(DEC_OUT_PD(SingleData)* pNewMemory, int a_second, int a_eventNumber);
    const char* daqName()const{return m_daqName;}
    //void AddExistanceInRootFile(int second, int eventNumber);
    int firstSecond()const{return m_firstSecond;}
    int firstEventNumber()const{return m_firstEventNumber;}
    int lastSecond()const{return m_lastSecond;}
    int lastEventNumber()const{return m_lastEventNumber;}
    uint64_t isPresentInCurrentFile()const{return m_isPresentInCurrentFile;}
    void WriteContentToTheFile(FILE* fpFile)const;
    // APIs for DOOCS property
    //void ValueStringByKey2(const char* request, char* buffer, int bufferLength)const;
    //void SetProperty(const char* propertyAndAttributes);
    //void RemoveDoocsProperty();
    void SetError(int a_error);

    // This API will be used only by
protected:
    //int  SetEntryInfo(uint32_t a_unOffset, const DEC_OUT_PD(BranchDataRaw)& a_branchInfo);
    //char*  ApplyEntryInfo( DEC_OUT_PD(BOOL2) a_bDubRootString );
    void   SetNetworkParent(SNetworkStruct* a_pNetworkParent);
        
private:
    ::std::list< SingleEntry* >::iterator   m_thisIter;

protected:
    ::std::list<EntryParams::Base*>         m_allParams;
    ::std::list<EntryParams::Base*>         m_userSetableParams;
    ::std::list<EntryParams::Base*>         m_permanentParams;

    //DEC_OUT_PD(BranchDataRaw)               m_branchInfo;

private:
#if 0
    //struct SPermanentParams2
    //{
    //    int         numOfFilesIn;
    //    int         numberInAllFiles;
    //    time_t      expirationTime;
    //    time_t      creationTime;
    //    uint64_t    errorsMasked : 1;
    //    uint64_t    collectionMasked : 1;
    //    uint64_t    collect : 1;
    //    uint64_t    reserved64bit : 61;
    //    SMaskParam  collectingMaskParam;
    //    SMaskParam  errorMaskParam;
    //};
#endif
    EntryParams::IntParam<int>      m_numberInCurrentFile;
    EntryParams::IntParam<int>      m_numberInAllFiles;
    EntryParams::IntParam<time_t>   m_expirationTime;
    EntryParams::IntParam<time_t>   m_creationTime;
    EntryParams::Mask               m_collectionMask;
    EntryParams::Mask               m_errorMask;
    EntryParams::IntParam<int>      m_error;

    // the story is following
    // everihhing, that is not nullable is set before the member m_nReserved
    // everything that should not be set to 0, should be declared before this line
private:
    int                                     m_firstEventNumber,m_lastEventNumber;
    int                                     m_firstSecond,m_lastSecond;

    char*                                   m_daqName;
    SNetworkStruct*                         m_pNetworkParent;
    TTree*                                  m_pTreeOnRoot2;
    TBranch*                                m_pBranchOnTree;
    //SPermanentParams2                       m_pp;

protected:
    mutable uint64_t                        m_willBeDeletedOrAddedToRootAtomic64 ;

    uint64_t                                m_isPresentInCurrentFile : 1;
    uint64_t                                m_isCleanEntryInheritableCalled : 1;
    uint64_t                                m_isValid : 1;
    uint64_t                                m_bitwise64Reserved : 61;

    int                                     m_nReserved1;
    int                                     m_nReserved2;

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

    EqFctCollector*  parent();
    bool AddNewEntry(SingleEntry *newEntry);
    const ::std::list< SingleEntry* >& daqEntries()/*const*/;

private:
    //void DataGetterThread();
    //void RemoveEntryNoDeletePrivate(SingleEntry *entry);

private:
    SNetworkStruct(const SNetworkStruct&){}

protected:
    EqFctCollector*                             m_pParent;
    STDN::thread                                m_thread;
    uint64_t                                    m_shouldRun : 1;
    uint64_t                                    m_bitwise64Reserved : 63 ;
    ::std::list< SingleEntry* >                 m_daqEntries;
    ::std::list< SNetworkStruct* >::iterator    m_thisIter;

};

}}

#ifndef PITZ_DAQ_SINGLEENTRY_IMPL_HPP
#include "pitz_daq_singleentry.impl.hpp"
#endif

#endif // PITZ_DAQ_SINGLEENTRY_HPP
