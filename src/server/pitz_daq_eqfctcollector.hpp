
// pitz_daq_eqfctcollector.hpp
// 2017 Nov 24

#ifndef PITZ_DAQ_EQFCTCOLLECTOR_HPP
#define PITZ_DAQ_EQFCTCOLLECTOR_HPP

#include "pitz_daq_collectorproperties.hpp"
#include <stdint.h>
#include <common/lists.hpp>


namespace pitz{namespace daq{

class EqFctCollector : public EqFct
{
    friend class D_addNewEntry;
    friend class D_removeEntry;
    friend class D_genEvent;
protected:
    EqFctCollector();
    virtual ~EqFctCollector();

    // API to be inferited
private:
    virtual pitz::daq::SingleEntry* CreateNewEntry(entryCreationType::Type type,const char* entryLine)=0;
    virtual void DataGetterThread(SNetworkStruct* pNet)=0;

    // API inherited from EqFct,
protected:
    void init(void) __OVERRIDE__ __FINAL__ ; // use complete (called before init) and post_init (called after init)
    void cancel(void) __OVERRIDE__ __FINAL__ ; // better to use 'virtual int clear(void)' in the child
    int  write          (fstream &fprt) __OVERRIDE__ __FINAL__;

    // API can be used, as well inherited by child
    virtual bool IsAllowedToAdd2(SingleEntry* newEntry);

    // API should be used by childs
    bool AddJobForRootThread(data::memory::ForServerBase* data);

    // API for internal usage
private:
    bool AddNewEntry(entryCreationType::Type type, const char* entryLine);
    pitz::daq::SingleEntry* RemoveOneEntry(SingleEntry* pEntry);
    pitz::daq::SingleEntry* FindEntry(const char* entryName);
    void CalculateRemoteDirPathAndFileName(std::string* fileName,std::string* remoteDirPath)STUPID_NON_CONST;
    void CalcLocalDir2(std::string* localDirPath)STUPID_NON_CONST;

    void CopyFileToRemoteAndMakeIndexing(const std::string& localFilePath, const std::string& remoteFilePath);

    void WriteEntriesToConfig()const;

    void DataGetterThreadPrivate(SNetworkStruct* pNet);
    void RootThreadFunction() ;
    void LocalFileDeleterThread();

public:
    // API public, for DOOCS properties usage
    int  parse_old_config2(const std::string& daqConfFilePath);
    void IncrementErrors(const char* entryName);
    void DecrementErrors(const char* entryName);

protected:
    D_int               m_genEvent; // +
private:
    D_int               m_rootLength; //+
    D_int               m_numberOfEntries; // +
    D_logLevel          m_logLevel;
    D_string            m_rootDirPathBaseRemote;       // '/acs/pitz/daq'
    D_string            m_rootDirPathBaseLocal;        // '/doocs/data/DAQdata/daqL'
    D_string            m_folderName;                  // 'pitznoadc0'
    D_string            m_rootFileNameBase;            // 'pitznoadc0'
    D_string            m_expertsMailingList;            // 'pitznoadc0'
    D_int               m_numberOfFillThreadsDesigned;
    D_int               m_numberOfFillThreadsFinal;
    D_int               m_currentFileSize;
    D_addNewEntry       m_addNewEntry;
    D_removeEntry       m_removeEntry;
    D_loadOldConfig     m_loadOldConfig;
    D_int               m_numberOfEntriesInError;
    D_text              m_entriesInError;

    SingleEntry         *m_pEntryFirst;
    int                 m_nNumberOfEntries;
    STDN::thread        m_threadRoot; // +
    STDN::thread        m_threadLocalFileDeleter;
    SNetworkStruct                                  *m_networkFirst, *m_networkLast;
    SNetworkStruct                                  *m_pLastNetworkAdded;
    int                                             m_nNumberOfFillThreadsDesigned;
    int                                             m_nNumberOfFillThreadsFinal;
    int                                             m_nIndexToNextNet;
    STDN::mutex             m_mutexForEntriesInError;
private:
    //common::FifoFastDyn<pitz::daq::MemoryBase*>     m_fifoToFill;
    common::listN::Fifo<data::memory::ForServerBase*>        m_fifoToFill;
    common::FifoFast<std::string,1024>                m_fifoForLocalFileDeleter;
    common::UnnamedSemaphoreLite                    m_semaForRootThread;
    common::UnnamedSemaphoreLite                    m_semaForLocalFileDeleter;
    bool                                            m_bRootStopped;
protected:
    //int                             m_nEventNumber;
    volatile int                    m_nWork;  // +
    ::STDN::shared_mutex     m_mutexForEntries;

    uint64_t                        m_unErrorUnableToWriteToDcacheNum : 16;

};

}} // namespace pitz{ namespace daq{

#endif // PITZ_DAQ_EQFCTCOLLECTOR_HPP
