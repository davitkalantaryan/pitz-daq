
// pitz_daq_eqfctcollector.hpp
// 2017 Nov 24

#ifndef PITZ_DAQ_EQFCTCOLLECTOR_HPP
#define PITZ_DAQ_EQFCTCOLLECTOR_HPP

#include "pitz_daq_collectorproperties.hpp"
#include <stdint.h>
#include <pitz_daq_internal.h>
#include <queue>
#include <string>
#include <vector>
#include <fstream>

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

class NewTFile;

template <typename QueueType>
class ProtectedQueue : private ::std::queue<QueueType>
{
public:
    void  pushBack(const QueueType& newElem);
    bool  frontAndPop(QueueType* a_pBuffer);

private:
    ::std::mutex    m_mutexForQueue;
};

template< typename TypeMutex >
class NewLockGuard
{
public:
    NewLockGuard(TypeMutex* aMutex=nullptr);
    ~NewLockGuard();
    void Lock(TypeMutex*   aMutex);
    void Unlock();
private:
    TypeMutex*   m_pMutex;
};

template< typename TypeSharedMutex >
class NewSharedLockGuard
{
public:
    NewSharedLockGuard(TypeSharedMutex* aMutex=nullptr);
    ~NewSharedLockGuard();
    void LockShared(TypeSharedMutex*   aMutex);
    void UnlockShared();
private:
    TypeSharedMutex*   m_pMutex;
};

struct SStructForFill{
	SingleEntry*            entry;
	DEC_OUT_PD(Header)*		data;
};


class EqFctCollector : public EqFct
{
    friend class SNetworkStruct;
    friend class SingleEntry;

protected:
    EqFctCollector();
    virtual ~EqFctCollector() OVERRIDE2 ;

    // API to be inherited
private:
    virtual pitz::daq::SingleEntry* CreateNewEntry(entryCreationType::Type type,const char* entryLine)=0;
    virtual void DataGetterFunctionWithWait(const SNetworkStruct* pNet, const ::std::vector<SingleEntry*>& pEntries)=0;

    // API can be used, as well inherited by child
protected:
    virtual bool IsAllowedToAdd(const char* newEntryName);
    virtual SNetworkStruct* CreateNewNetworkStruct();

    // API should be used by childs
protected:
	bool AddJobForRootThread(DEC_OUT_PD(Header)* data, SingleEntry* pEntry);
    uint64_t  shouldWork()const;

    // API inherited from EqFct,
private:
    void init(void) OVERRIDE2 FINAL2 ; // use complete (called before init) and post_init (called after init) or online (before)
    CLEAR_RET_TYPE CLEAR_FUNC_NAME(void) OVERRIDE2 FINAL2 ;  // use 'virtual void cancel(void)' in the child (called before cancel)
	int  write          ( ::std::ostream &fprt) OVERRIDE2 FINAL2;

    // API for internal usage
private:
    void AddNewEntryNotLocked(entryCreationType::Type type, const char* entryLine);
    void TryToRemoveEntryNotLocked(SingleEntry* pEntry);
    pitz::daq::SingleEntry* FindEntry(const char* entryName);
    void CalculateRemoteDirPathAndFileName(std::string* fileName,std::string* remoteDirPath)STUPID_NON_CONST;
    void CalcLocalDir(std::string* localDirPath)STUPID_NON_CONST;
    void CopyFileToRemoteAndMakeIndexing(const std::string& localFilePath, const std::string& remoteFilePath);
    void writeEntriesToConfig()const;
	bool RootFileCreator(std::string* a_pFilePathLocal, std::string* a_pFilePathRemote);

    // thread functions
    void  RootThreadFunction() ;
    void  LocalFileDeleterThread();
    void  DataGetterThread(SNetworkStruct* pNet);

public:
    // API public, for DOOCS properties usage
    int  parse_old_config(const std::string& daqConfFilePath);
    void IncrementErrors(const char* entryName);
    void DecrementErrors(const char* entryName);

    bool AddNewEntryByUser(const char* entryLine);
    bool RemoveEntryByUser(const char* entryName);


private:
    //D_fct                               m_testProp;
	D_closeFile							m_closeFile;
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
    D_addNewEntry                       m_addNewEntry;
    D_removeEntry                       m_removeEntry;
    D_loadOldConfig                     m_loadOldConfig;
    D_int                               m_numberOfEntriesInError;
    D_text                              m_entriesInError;
    D_string                            m_entriesReturnDelimeter;
    ::STDN::thread                      m_threadRoot; // +
    ::STDN::thread                      m_threadLocalFileDeleter;
    ::std::list< SNetworkStruct* >      m_networsList;
    ::std::mutex                        m_mutexForEntriesInError;
    ProtectedQueue< SStructForFill >    m_fifoToFill;
    ProtectedQueue< ::std::string >     m_fifoForLocalFileDeleter;
    common::UnnamedSemaphoreLite        m_semaForRootThread;
    common::UnnamedSemaphoreLite        m_semaForLocalFileDeleter;
    ::STDN::shared_mutex                m_lockForEntries;
    SNetworkStruct*                     m_pNextNetworkToAdd;
    uint64_t                            m_unErrorUnableToWriteToDcacheNum : 16;
    uint64_t                            m_shouldWork : 1;
    uint64_t                            m_bitwise64Reserved : 47;
	/*__thisthread */NewTFile*			m_pRootFile;

};

}} // namespace pitz{ namespace daq{

#ifndef PITZ_DAQ_EQFCTCOLLECTOR_IMPL_HPP
#include "pitz_daq_eqfctcollector.impl.hpp"
#endif

#endif // PITZ_DAQ_EQFCTCOLLECTOR_HPP
