
// pitz_daq_eqfctcollector.hpp
// 2017 Nov 24

#ifndef PITZ_DAQ_EQFCTCOLLECTOR_HPP
#define PITZ_DAQ_EQFCTCOLLECTOR_HPP

#include "pitz_daq_collectorproperties.hpp"
#include <stdint.h>
#include <pitz_daq_internal.h>
#include <queue>
#include <string>

#define     CODE_EVENT_BASED_DAQ  301	// eq_fct_type number for the .conf file

#define     NULL_THREAD_HANDLE      0

//#define USE_SHARED_LOCK

#ifdef USE_SHARED_LOCK
#define OVERRIDE3   OVERRIDE2
#else
#define OVERRIDE3
#endif

#ifdef NEW_GETTER_THREAD
#define CLEAR_FUNC_NAME                clear
#define CLEAR_RET_TYPE                 int
#define CAST_CLEAR_RET(_retValue)      (_retValue)
#else
#define CLEAR_FUNC_NAME                cancel
#define CLEAR_RET_TYPE                 void
#define CAST_CLEAR_RET(_retValue)
#endif


namespace pitz{namespace daq{

class EntryLock
#ifdef USE_SHARED_LOCK
        : public ::STDN::shared_mutex
#endif
{
public:
    EntryLock();
    void WriteLockWillBeCalled();
    void lock( ) OVERRIDE3;
    void unlock() OVERRIDE3;
#ifndef USE_SHARED_LOCK
    void lock_shared();
    void unlock_shared();
#endif
private:
    pthread_t   m_writerThread;
    uint32_t    m_nLocksCount;
    uint32_t    m_isGoingToWriteLock : 1;
    uint32_t    m_bitwiseReserved : 31;
};

struct SStructForFill{
    SingleEntry*            entry;
    DEC_OUT_PD(SingleData)* data;
};

class EqFctCollector : public EqFct
{
    //friend class D_addNewEntry;
    //friend class D_removeEntry;
    //friend class D_genEvent;
    friend class SNetworkStruct;

protected:
    EqFctCollector();
    virtual ~EqFctCollector() OVERRIDE2 ;

    // API to be inherited
private:
    virtual pitz::daq::SingleEntry* CreateNewEntry(entryCreationType::Type type,const char* entryLine)=0;
    virtual void DataGetterThread(SNetworkStruct* pNet)=0;

    // API can be used, as well inherited by child
protected:
    virtual bool IsAllowedToAdd2(const char* newEntryName);
    virtual SNetworkStruct* CreateNewNetworkStruct();

    // API should be used by childs
protected:
    bool AddJobForRootThread(DEC_OUT_PD(SingleData)* data, SingleEntry* pEntry);
    uint64_t  shouldWork()const;

    // API inherited from EqFct,
private:
    void init(void) OVERRIDE2 FINAL2 ; // use complete (called before init) and post_init (called after init) or online (before)
    CLEAR_RET_TYPE CLEAR_FUNC_NAME(void) OVERRIDE2 FINAL2 ;  // use 'virtual void cancel(void)' in the child (called before cancel)
    int  write          (fstream &fprt) OVERRIDE2 FINAL2;   

    // API for internal usage
private:
    SStructForFill  GetAndDeleteFirstData();
    void AddNewEntryNotLocked(entryCreationType::Type type, const char* entryLine);
    void TryToRemoveEntryNotLocked(SingleEntry* pEntry);
    pitz::daq::SingleEntry* FindEntry(const char* entryName);
    bool FindEntryInAdding(const char* entryName);
    bool FindEntryInDeleting(const char* entryName);
    void CalculateRemoteDirPathAndFileName(std::string* fileName,std::string* remoteDirPath)STUPID_NON_CONST;
    void CalcLocalDir2(std::string* localDirPath)STUPID_NON_CONST;

    void CopyFileToRemoteAndMakeIndexing(const std::string& localFilePath, const std::string& remoteFilePath);
    void WriteEntriesToConfig()const;
    void CheckGenEventError(int* a_nPreviousTime, int* a_nPreviousGenEvent);

    void RootThreadFunction() ;
    void LocalFileDeleterThread();
    void EntryAdderDeleter();

public:
    // API public, for DOOCS properties usage
    int  parse_old_config2(const std::string& daqConfFilePath);
    void IncrementErrors(const char* entryName);
    void DecrementErrors(const char* entryName);

    void WriteLockEntries2();
    void WriteUnlockEntries2();
    void ReadLockEntries2();
    void ReadUnlockEntries2();

    bool AddNewEntryByUser(const char* entryLine);
    bool RemoveEntryByUser(const char* entryName);


private:
    D_int                               m_genEvent; // +
    D_int                               m_fileMaxSize; //+
    D_int                               m_numberOfEntries; // +
    D_logLevel                          m_logLevel;
    D_string                            m_rootDirPathBaseRemote;       // '/acs/pitz/daq'
    D_string                            m_rootDirPathBaseLocal;        // '/doocs/data/DAQdata/daqL'
    D_string                            m_folderName;                  // 'pitznoadc0'
    D_string                            m_rootFileNameBase;            // 'pitznoadc0'
    D_string                            m_expertsMailingList;            // 'davit.kalantaryan@desy.de'
    D_int                               m_numberOfFillThreadsDesigned;
    D_int                               m_numberOfFillThreadsFinal;
    D_int                               m_currentFileSize;
    D_int                               m_maxUnsavedDataInTheFile;
    D_addNewEntry                       m_addNewEntry;
    D_removeEntry                       m_removeEntry;
    D_loadOldConfig                     m_loadOldConfig;
    D_int                               m_numberOfEntriesInError;
    D_text                              m_entriesInError;
    ::STDN::thread                      m_threadForEntryAdding;
    ::STDN::thread                      m_threadRoot; // +
    ::STDN::thread                      m_threadLocalFileDeleter;
    ::std::list< SNetworkStruct* >      m_networsList;
    ::std::mutex                        m_mutexForEntriesInError;
    ::std::queue< SStructForFill >      m_fifoToFill;
    ::std::queue< ::std::string >       m_entriesToAdd;
    ::std::queue< ::std::string >       m_fifoForLocalFileDeleter;
    common::UnnamedSemaphoreLite        m_semaForRootThread;
    common::UnnamedSemaphoreLite        m_semaForEntryAdder;
    common::UnnamedSemaphoreLite        m_semaForLocalFileDeleter;
    EntryLock                           m_lockForEntries2;
    SNetworkStruct*                     m_pNextNetworkToAdd;
    uint64_t                            m_unErrorUnableToWriteToDcacheNum : 16;
    uint64_t                            m_shouldWork : 1;
    uint64_t                            m_bitwise64Reserved : 47;
    int                                 m_nNumberOfEntries;
    int                                 m_nNumberOfFillThreadsFinal;

};

}} // namespace pitz{ namespace daq{

#endif // PITZ_DAQ_EQFCTCOLLECTOR_HPP
