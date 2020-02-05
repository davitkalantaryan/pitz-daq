
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

#define SPECIAL_KEY_DATA_TYPE       "type"
#define SPECIAL_KEY_DATA_SAMPLES    "samples"
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


#define ROOT_ERROR                  1
#define NETWORK_READ_ERROR          (1<<1)
#define DATA_TYPE_MISMATCH_ERROR    (1<<2)


namespace pitz{ namespace daq{

class SNetworkStruct;
class SingleEntry;
class EqFctCollector;
class TreeForSingleEntry;

typedef const char* TypeConstCharPtr;

namespace entryCreationType{enum Type{fromOldFile,fromConfigFile,fromUser,unknownCreation};}
namespace errorsFromConstructor{enum Error{noError=0,syntax=10,lowMemory, type,doocsUnreachable};}


#define D_BASE_FOR_STR  D_text

namespace EntryParams{

class Base;

typedef void (*TypeClbk)(Base*,void*);

class Base
{
public:
    Base(const char* entryParamName);
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
    IntParam(const char* entryParamName);
    virtual ~IntParam() OVERRIDE2 ;
    virtual bool   GetDataFromLine(const char* entryLine) OVERRIDE2;
    virtual size_t WriteDataToLineBuffer(char* entryLineBuffer, size_t unBufferSize)const OVERRIDE2;
    operator const IntType&()const;
    void operator=(const IntType& newValue);
    void operator++();

protected:
    IntType    m_value;
};


class SomeInts : protected IntParam<int32_t>
{
public:
    SomeInts(const char* entryParamName);
    virtual ~SomeInts() OVERRIDE2 ;

    int value()const;
    Base* thisPtr();
    size_t WriteDataToLineBuffer(char* entryLineBuffer, size_t unBufferSize)const OVERRIDE2;

private:
    virtual ::std::string additionalString()const=0;

};


class Error : public SomeInts
{
public:
    Error(const char* entryParamName);
    ~Error() OVERRIDE2 ;

    void setError(int error, const ::std::string& errorString);

private:
    ::std::string additionalString()const OVERRIDE2;

private:
    std::string m_errorString;
};


class DataType : public SomeInts
{
public:
    DataType(const char* entryParamName);
    ~DataType() OVERRIDE2 ;

    void set(int32_t type);

private:
    ::std::string additionalString()const OVERRIDE2;

};


class Date : public Base
{
public:
    Date(const char* entryParamName);
    virtual ~Date() OVERRIDE2 ;

    virtual bool   GetDataFromLine(const char* entryLine) OVERRIDE2;
    virtual size_t WriteDataToLineBuffer(char* entryLineBuffer, size_t unBufferSize)const OVERRIDE2;
    time_t dateSeconds()const;
    void setDateSeconds(time_t a_dateSeconds);

protected:
    time_t m_epochSeconds;
};


class Mask : public Date
{
public:
    Mask(const char* entryParamName);
    ~Mask() OVERRIDE2 ;

    bool   GetDataFromLine(const char* entryLine) OVERRIDE2;
    size_t WriteDataToLineBuffer(char* entryLineBuffer, size_t unBufferSize)const OVERRIDE2;
    bool   isMasked()const;

};

class String : public Base
{
public:
    String(const char* entryParamName);
    ~String() OVERRIDE2 ;

    bool   GetDataFromLine(const char* entryLine) OVERRIDE2;
    size_t WriteDataToLineBuffer(char* entryLineBuffer, size_t unBufferSize)const OVERRIDE2;
    const ::std::string& value()const;
    void setValue(const ::std::string& newValue);

private:
    ::std::string m_string;
};

} // namespace EntryParams{



class SingleEntry : protected D_BASE_FOR_STR
{
    friend class SNetworkStruct;
    friend class TreeForSingleEntry;
public:
    SingleEntry( entryCreationType::Type creationType,const char* entryLine, TypeConstCharPtr* a_pHelper);
    virtual ~SingleEntry() OVERRIDE2;

    virtual const char* rootFormatString()const=0;
    virtual void        FreeUsedMemory(DEC_OUT_PD(SingleData)* usedMemory);
    SNetworkStruct*     networkParent();
    uint64_t            isValid()const;
    void                SetValid();
    void                SetInvalid();
    bool                markEntryForDeleteAndReturnIfPossibleNow();
    bool                lockEntryForRoot();
    bool                lockEntryForCurrentFile();
    bool                lockEntryForNetwork();
    bool                lockEntryForRootFile();
    bool                resetRootLockAndReturnIfDeletable();
    bool                resetNetworkLockAndReturnIfDeletable();
    bool                resetRooFileLockAndReturnIfDeletable();
    bool                isLockedForAnyAction()const;
    void                Fill(DEC_OUT_PD(SingleData)* pNewMemory, int a_second, int a_eventNumber);
    const char*         daqName()const{return m_daqName;}
    int                 firstSecond()const{return m_firstSecond;}
    int                 firstEventNumber()const{return m_firstEventNumber;}
    int                 lastSecond()const{return m_lastSecond;}
    int                 lastEventNumber()const{return m_lastEventNumber;}
    uint64_t            isPresentInCurrentFile()const{return m_isPresentInCurrentFile;}
    void                WriteContentToTheFile(FILE* fpFile)const;
    void                SetError(int a_error, const ::std::string& a_errorString);

    // This API will be used only by inheritors (childs, grandchilds etc.)
protected:
    void                LoadFromLine(const char* a_entryLine, bool isIniting, bool isInitingByUserSet);
    void                AddNewParameterToEnd(EntryParams::Base* newParam, bool isUserSetable, bool isPermanent);
    void                AddNewParameterToBeg(EntryParams::Base* newParam, bool isUserSetable, bool isPermanent);

private:
    // DOOCS callbacks
    void                get(EqAdr* /*a_dcsAddr*/, EqData* a_dataFromUser, EqData* a_dataToUser,EqFct* /*a_loc*/) OVERRIDE2;
    void                set(EqAdr* a_dcsAddr, EqData* a_dataFromUser, EqData* a_dataToUser,EqFct* a_loc) OVERRIDE2 ;
        
private:
    ::std::list< SingleEntry* >::iterator   m_thisIter;
    ::std::list<EntryParams::Base*>         m_allParams;
    ::std::list<EntryParams::Base*>         m_userSetableParams;
    ::std::list<EntryParams::Base*>         m_permanentParams;
    EntryParams::IntParam<int>              m_numberInCurrentFile;
    EntryParams::IntParam<int>              m_numberInAllFiles;
    EntryParams::Date                       m_expirationTime;
    EntryParams::Date                       m_creationTime;
    EntryParams::Mask                       m_collectionMask;
    EntryParams::Mask                       m_errorMask;
    EntryParams::Error                      m_errorWithString;

protected:
    EntryParams::DataType                   m_dataType;
    EntryParams::IntParam<int32_t>          m_itemsCountPerEntry;

    // the story is following
    // everihhing, that is not nullable is set before the member m_nReserved
    // everything that should not be set to 0, should be declared before this line
private:
    char*                                   m_daqName;
    int                                     m_firstEventNumber,m_lastEventNumber;
    int                                     m_firstSecond,m_lastSecond;

    SNetworkStruct*                         m_pNetworkParent;
    TTree*                                  m_pTreeOnRoot2;
    TBranch*                                m_pBranchOnTree;

    mutable uint64_t                        m_willBeDeletedOrIsUsedAtomic64 ;

    uint64_t                                m_isPresentInCurrentFile : 1;
    //uint64_t                                m_isCleanEntryInheritableCalled : 1;
    uint64_t                                m_isValid : 1;
    uint64_t                                m_bitwise64Reserved : 61;

    int                                     m_nReserved1;
    int                                     m_nReserved2;

private:
    SingleEntry(const SingleEntry&) = delete ;
};


class SNetworkStruct
{
    friend class EqFctCollector;
    friend class SingleEntry;
public:
    SNetworkStruct(EqFctCollector* parent);
    virtual ~SNetworkStruct();

    bool AddNewEntry(SingleEntry *newEntry);
    const ::std::list< SingleEntry* >& daqEntries()/*const*/;

protected:
    void StopThreadThenDeleteAndClearEntries();

private:
    EqFctCollector*                             m_pParent;
    STDN::thread                                m_thread;
    uint64_t                                    m_shouldRun : 1;
    uint64_t                                    m_bitwise64Reserved : 63 ;
    ::std::list< SingleEntry* >                 m_daqEntries;
    ::std::list< SNetworkStruct* >::iterator    m_thisIter;
private:
    SNetworkStruct(const SNetworkStruct&){}

};

}}

#ifndef PITZ_DAQ_SINGLEENTRY_IMPL_HPP
#include "pitz_daq_singleentry.impl.hpp"
#endif

#endif // PITZ_DAQ_SINGLEENTRY_HPP
