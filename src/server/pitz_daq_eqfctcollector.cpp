
// pitz_daq_eqfctcollector.cpp
// 2017 Nov 24

#include <cstdlib>
#include "pitz_daq_eqfctcollector.hpp"
#include <signal.h>
#include "pitz_daq_eqfctcollector.cpp.hpp"
#include <dirent.h>
#include "mailsender.h"
#include <sys/timeb.h>
#include <sys/time.h>
#include <time.h>
#include <pitz_daq_data_collector_getter_common.h>
#include <iostream>
#include <ios>
#include "pitz_daq_singleentry.cpp.hpp"

#define CONF_FILE_VERSION_START "DAQ_VERSION="
static const size_t s_cunDaqVersionForConfigLen = strlen(CONF_FILE_VERSION_START);

#define MINIMUM_ROOT_FILE_SIZE_HARD     1000

//#define SEMA_WAIT_TIME_MS       10000
#define WAIT_INFINITE  -1
#define BUF_LEN_FOR_STRFTIME    64
#define data_length             1024
#define NUMER_OF_WAITING_FOR_ERR_REP    36000

#define HANDLE_MEM_DIFF(_pointer,...) \
    do{ \
        if(!(_pointer)){ \
            exit(1); \
        } \
    }while(0)

static const char*      s_LN = "#_QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm1234567890";

static int NewSystemStat(const char *a_command);

static void SignalHandler(int){}

pitz::daq::EqFctCollector::EqFctCollector()
        :
          EqFct("Name = location"),
          //m_testProp("TEST_VOID",this),
          m_genEvent("GEN_EVENT value",this),
          m_fileMaxSize("FILE_SIZE_DESIGNED approximate size of final file", this),
          m_numberOfEntries("NUMBER.OF.ENTRIES",this),
          m_logLevel("LOG.LEVEL log level of server", this),
          m_rootDirPathBaseRemote("ROOT.DIR.PATH.REMOTE",this),
          m_rootDirPathBaseLocal("ROOT.DIR.PATH.LOCAL",this),
          m_folderName("FOLDER.NAME for example 'pitznoadc0'",this),
          m_rootFileNameBase("ROOT.FILE.NAME.BASE",this),
          m_expertsMailingList("EXPERTS.MAILING.LIST  should be ; separated",this),
          m_numberOfFillThreadsDesigned("NUMBER.OF.FILL.THREDS.DESIGNED",this),
          m_numberOfFillThreadsFinal("NUMBER.OF.FILL.THREDS.FINAL",this),
          m_currentFileSize("CURRENT.FILE.SIZE",this),
          m_addNewEntry("ADD.NEW.ENTRY",this),
          m_removeEntry("DELETE.ENTRY",this),
          m_loadOldConfig("LOAD.OLD.CONFIG",this),
          m_numberOfEntriesInError("NUMBER.OF.ENTRIES.IN.ERROR",this),
          m_entriesInError("ENTRIES.IN.ERROR",this),
          m_entriesReturnDelimeter("ENTRIES.RETURN.DELIMETER valid delimeters '\t' '\n' ' ' ',' ';'",this)
{
	m_pRootFile = nullptr;
    m_pNextNetworkToAdd = NEWNULLPTR2;

    m_unErrorUnableToWriteToDcacheNum = 0;
    m_shouldWork  = 0;
    m_bitwise64Reserved = 0;

    m_unErrorUnableToWriteToDcacheNum = 0;

    EqFct::err_.set_ro_access();
    m_genEvent.set_ro_access();
    // m_fileMaxSize
    m_numberOfEntries.set_ro_access();
    //m_logLevel("LOG.LEVEL log level of server", this),
    //m_rootDirPathBaseRemote("ROOT.DIR.PATH.REMOTE",this),
    //m_rootDirPathBaseLocal("ROOT.DIR.PATH.LOCAL",this),
    //m_folderName("FOLDER.NAME for example 'pitznoadc0'",this),
    //m_rootFileNameBase("ROOT.FILE.NAME.BASE",this),
    //m_expertsMailingList("EXPERTS.MAILING.LIST  should be ; separated",this),
    //m_numberOfFillThreadsDesigned("NUMBER.OF.FILL.THREDS.DESIGNED",this),
    m_numberOfFillThreadsFinal.set_ro_access();
    m_currentFileSize.set_ro_access();
    //m_addNewEntry("ADD.NEW.ENTRY",this),
    //m_removeEntry("DELETE.ENTRY",this),
    //m_loadOldConfig("LOAD.OLD.CONFIG",this),
    m_numberOfEntriesInError.set_ro_access();
    m_entriesInError.set_ro_access();

}


pitz::daq::EqFctCollector::~EqFctCollector()
{
    EqFctCollector::CLEAR_FUNC_NAME();
}


void pitz::daq::EqFctCollector::CalcLocalDir(std::string* a_localDirName)STUPID_NON_CONST
{
    *a_localDirName = m_rootDirPathBaseLocal.value() + std::string("/") + m_folderName.value();
}


void pitz::daq::EqFctCollector::CalculateRemoteDirPathAndFileName(std::string* a_fileName,std::string* a_remoteDirPath)STUPID_NON_CONST
{
    time_t  aWalltime (::time(NEWNULLPTR2));
    struct tm * timeinfo(localtime(&aWalltime));
    char
            vcYear[BUF_LEN_FOR_STRFTIME], vcMonth[BUF_LEN_FOR_STRFTIME],
            vcDay[BUF_LEN_FOR_STRFTIME], vcHour[BUF_LEN_FOR_STRFTIME],
            vcMinute[BUF_LEN_FOR_STRFTIME], vcSeconds[BUF_LEN_FOR_STRFTIME];

    strftime (vcYear,BUF_LEN_FOR_STRFTIME,"%Y",timeinfo);
    strftime (vcMonth,BUF_LEN_FOR_STRFTIME,"%m",timeinfo);
    strftime (vcDay,BUF_LEN_FOR_STRFTIME,"%d",timeinfo);
    strftime (vcHour,BUF_LEN_FOR_STRFTIME,"%H",timeinfo);
    strftime (vcMinute,BUF_LEN_FOR_STRFTIME,"%M",timeinfo);
    strftime (vcSeconds,BUF_LEN_FOR_STRFTIME,"%S",timeinfo);
    DUMMY_ARGS2(aWalltime);

    *a_remoteDirPath = m_rootDirPathBaseRemote.value()+std::string("/")+
             std::string(vcYear) + "/"+
             std::string(vcMonth)+ "/"+
             std::string(m_folderName.value());

    *a_fileName = m_rootFileNameBase.value()+std::string(".")+
            std::string(vcYear)+"-"+std::string(vcMonth)+"-"+
            std::string(vcDay)+"-"+std::string(vcHour)+std::string(vcMinute) +vcSeconds +
            std::string(".root");
}


int pitz::daq::EqFctCollector::write(fstream &a_fprt)
{
    int nReturn = EqFct::write(a_fprt);
    NewSharedLockGuard< ::STDN::shared_mutex > aSharedGuard(&m_lockForEntries);
    writeEntriesToConfig();
    return nReturn;
}


void pitz::daq::EqFctCollector::init(void)
{
    int nConfigFileVersion = PITZ_DAQ_CURRENT_VERSION;
    NewSharedLockGuard< ::STDN::shared_mutex > aSharedGuard;
    SNetworkStruct* pNetworkToAdd2;
    int i;
    int nNumberOfFillThreadsDesigned;
    FILE* fpConfig;
    char data[data_length];
    char vcNewConfFileName[512];
    char* pn;

    if(m_shouldWork){return;}
    m_shouldWork = 1;

    set_error(0,"ok");

    m_numberOfEntries.set_value(0);
    m_entriesInError.set_value("");
    m_numberOfEntriesInError.set_value(0);
    m_currentFileSize.set_value(0);
    if(m_fileMaxSize.value()<MINIMUM_ROOT_FILE_SIZE_HARD){m_fileMaxSize.set_value(MINIMUM_ROOT_FILE_SIZE_HARD);}

    g_nLogLevel = m_logLevel.value();
    InitSocketLibrary();

    aSharedGuard.LockShared(&m_lockForEntries);

    nNumberOfFillThreadsDesigned = m_numberOfFillThreadsDesigned.value();
    if(nNumberOfFillThreadsDesigned<=0){nNumberOfFillThreadsDesigned=1;m_numberOfFillThreadsDesigned.set_value(1);}

    snprintf(vcNewConfFileName,511,"%s.nconf",name().c_str());
    fpConfig = fopen(vcNewConfFileName,"r");

    DEBUG_APP_INFO(1,"!!!!!!!!!!!!!!!!!!!!!!!!!!!! flName=%s, filePtr=%p ", vcNewConfFileName, static_cast<void*>(fpConfig) );

    for(i=0;i<nNumberOfFillThreadsDesigned;++i){

        pNetworkToAdd2 = this->CreateNewNetworkStruct();
        m_networsList.push_back(pNetworkToAdd2);
        pNetworkToAdd2->m_thisIter = --m_networsList.end();

    }
    m_numberOfFillThreadsFinal.set_value(static_cast<int>(m_networsList.size()));
    m_pNextNetworkToAdd = m_networsList.front();

    if(!fpConfig){goto finalizeStuffPoint;}

    while ( fgets(data, data_length, fpConfig) ){
        //if(  data[0] == '#'  ) continue;
        pn = strpbrk(data,s_LN);
        if( ( !pn ) || ( pn[0] == '#' ) ) continue;
        if(strncmp(CONF_FILE_VERSION_START,pn,s_cunDaqVersionForConfigLen)==0){
            nConfigFileVersion =atoi(pn+s_cunDaqVersionForConfigLen);
            ::std::cout<<"DAQ version when config file is created is " << nConfigFileVersion << ::std::endl;
            continue;
        }
        if(IsAllowedToAdd(pn)){
            AddNewEntryNotLocked(entryCreationType::fromConfigFile, data);
        }
    }
    fclose(fpConfig);

finalizeStuffPoint:
    aSharedGuard.UnlockShared();
    DEBUG_APP_INFO(1," ");

    m_threadRoot = ::STDN::thread(&EqFctCollector::RootThreadFunction,this);
    m_threadLocalFileDeleter = ::STDN::thread(&EqFctCollector::LocalFileDeleterThread,this);
}


bool pitz::daq::EqFctCollector::IsAllowedToAdd(const char* a_newEntryLine)
{
    // should be more general search
    ::std::string aEntryName(a_newEntryLine);
    char* pcEntryName = const_cast<char*>(aEntryName.data());
    char* cpcBrk = strpbrk(pcEntryName,POSIIBLE_TERM_SYMBOLS);
    if(cpcBrk){*cpcBrk=0;}

    if( FindEntry(pcEntryName) ){
        return false;
    }

    return true;
}


void pitz::daq::EqFctCollector::DataGetterThread(SNetworkStruct* a_pNet)
{
    NewSharedLockGuard< ::STDN::shared_mutex > lockGuard;
    ::std::vector<SingleEntry*> vectForEntries;
    struct timeval aTimeBeforeSleepFncCall, aTimeAfterSleepFncCall;

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

enteTryPoint:
    try {
        while(shouldWork() && a_pNet->m_shouldRun){
            vectForEntries.clear();
            lockGuard.LockShared(&m_lockForEntries);
            for(auto pEntry : a_pNet->daqEntries()){
                if(pEntry->lockEntryForNetwork()){
                    vectForEntries.push_back(pEntry);
                }
            }
            lockGuard.UnlockShared();

            if(vectForEntries.size()>0){
                gettimeofday(&aTimeBeforeSleepFncCall,nullptr);
                this->DataGetterFunctionWithWait(a_pNet,vectForEntries);
                gettimeofday(&aTimeAfterSleepFncCall,nullptr);
                if((aTimeAfterSleepFncCall.tv_sec==aTimeBeforeSleepFncCall.tv_sec)&&(aTimeAfterSleepFncCall.tv_usec<(aTimeBeforeSleepFncCall.tv_usec+4))){
                    sleep(2);
                }
            }
            else{
                sleep(2);
            }

            lockGuard.LockShared(&m_lockForEntries);
            for(auto pEntry : a_pNet->daqEntries()){
                pEntry->resetNetworkLockAndReturnIfDeletable();
            }
            lockGuard.UnlockShared();
        }

    } catch (...) {
        lockGuard.UnlockShared();
        sleep(5);
        goto enteTryPoint;
    }


}


pitz::daq::SNetworkStruct* pitz::daq::EqFctCollector::CreateNewNetworkStruct()
{
    return new SNetworkStruct(this);
}


pitz::daq::SingleEntry* pitz::daq::EqFctCollector::FindEntry(const char* a_entryName)
{
    SingleEntry* pCurEntry;
    ::std::list< SingleEntry* >::const_iterator pIter, pIterEnd;
    const ::std::list< SingleEntry* >* pList;

    for( auto netStruct : m_networsList){
        pList = &netStruct->daqEntries();
        pIterEnd = pList->end();
        for(pIter=pList->begin();pIter!=pIterEnd;++pIter){
            pCurEntry = *pIter;
            if(strcmp(pCurEntry->daqName(),a_entryName)==0){return pCurEntry;}
        }
    }

    return NEWNULLPTR2;
}


#define DELIMETER_SYMBOL  "\n"


void pitz::daq::EqFctCollector::IncrementErrors(const char* a_entryName)
{
    std::string strEntries ;
    const char* cpcBuffer;
    const char* cpcEntryInTheGroup;
    //char vcBuffer[1024];
    const size_t entryNameLen = strlen(a_entryName);
    int nNumber = 0;
    unsigned int unCurrentError;

    m_mutexForEntriesInError.lock();
    unCurrentError = static_cast<unsigned int>(get_error());
    unCurrentError |= ENTRY_IN_ERROR;
    set_error(static_cast<int>(unCurrentError));
    strEntries = m_entriesInError.value();
    cpcBuffer = strEntries.c_str();
    cpcEntryInTheGroup = strstr(cpcBuffer,a_entryName);
    if(cpcEntryInTheGroup){
        if(*(cpcEntryInTheGroup+entryNameLen)==DELIMETER_SYMBOL[0]){
            m_mutexForEntriesInError.unlock();
            return;
        }
    }

    nNumber = m_numberOfEntriesInError.value();

    m_numberOfEntriesInError.set_value(++nNumber);
    DEBUG_APP_INFO(2,"!!!!!!!!!!!!!!! IncrementErrors setting to %d",nNumber);
    //snprintf(vcBuffer,1023,"%s%s;",cpcEntries,a_entryName);
    strEntries += (std::string(a_entryName)+DELIMETER_SYMBOL);
    m_entriesInError.set_value(strEntries);
    m_mutexForEntriesInError.unlock();
}


void pitz::daq::EqFctCollector::DecrementErrors(const char* a_entryName)
{
    NewLockGuard< ::std::mutex > lockGuard;
    int nNumber;
    unsigned int unCurrentError;

    lockGuard.Lock(&m_mutexForEntriesInError);

    nNumber = m_numberOfEntriesInError.value();

    if(nNumber>0){
        char* pcNext;
        char* pcBuffer ;
        const size_t entryNameLen(strlen(a_entryName));
        size_t unWholeStrLen;
        //char vcBuffer[1024];
        std::string strEntries ;

        strEntries = m_entriesInError.value();
        pcBuffer = const_cast<char*>(strEntries.c_str());
        pcNext=strstr(pcBuffer,a_entryName);
        if(!pcNext){m_mutexForEntriesInError.unlock();return;}

        if(*(pcNext+entryNameLen)!=DELIMETER_SYMBOL[0]){
            lockGuard.Unlock();
            return;
        }

        unWholeStrLen = strEntries.length();
        if(!(--nNumber)){
            unCurrentError = static_cast<unsigned int>(get_error());
            unCurrentError &= (~ENTRY_IN_ERROR);
            set_error(static_cast<int>(unCurrentError));
        }
        m_numberOfEntriesInError.set_value(nNumber);
        DEBUG_APP_INFO(2,"!!!!!!!!!!!!!!! DecrementErrors setting to %d",nNumber);
        //strncpy(vcBuffer,cpcEntries,1023);

        memmove(pcNext,pcNext+entryNameLen+1,unWholeStrLen-static_cast<size_t>(pcNext-pcBuffer)-entryNameLen);
        m_entriesInError.set_value(pcBuffer);

    }

    lockGuard.Unlock();
}


int pitz::daq::EqFctCollector::parse_old_config(const std::string& a_daqConfFilePath)
{
    NewSharedLockGuard< ::STDN::shared_mutex > aSharedGuard;
    FILE *fpConfig;
    const char* pn;
    const std::string& daqConfFilePath = a_daqConfFilePath;
    char    data[data_length];
    int     nReturn(-1);

    aSharedGuard.LockShared(&m_lockForEntries);

    fpConfig =	fopen(daqConfFilePath.c_str(),"r");
    DEBUG_APP_INFO(1,"fpConfig=%p",static_cast<void*>(fpConfig));
    if(!fpConfig){
        std::cerr<<"Unable to open the file "<<daqConfFilePath<<std::endl;
        perror("\n");
        goto returnPoint;
    }
    DEBUG_APP_INFO(2," ");
    while ( fgets(data, data_length, fpConfig) ){
        pn = strpbrk(data,s_LN);
        if( ( !pn ) || ( pn[0] == '#' ) ) continue;
        if(IsAllowedToAdd(pn)){
            AddNewEntryNotLocked(entryCreationType::fromOldFile, data);
        }
    }
    fclose(fpConfig);
    //m_fifoToFill.ResizeCash(m_nNumberOfEntries*4);
    DEBUG_APP_INFO(0,"!!!!!!! numberOfEntries=%d, numberOfEntriesDcs=%d",m_numberOfEntries.value(),m_numberOfEntries.value());
    nReturn = 0;

returnPoint:
    writeEntriesToConfig();
    aSharedGuard.UnlockShared();
    return nReturn;

}


void pitz::daq::EqFctCollector::AddNewEntryNotLocked(entryCreationType::Type a_creationType,const char* a_entryLine)
{
    ::std::list< SNetworkStruct* >::iterator    nextIterator;
    SingleEntry *pCurEntry(nullptr);
    int nNumberOfEntries = m_numberOfEntries.value();

    try{
        pCurEntry = CreateNewEntry(a_creationType,a_entryLine);
    }
    catch(...){
        return;
    }

    m_pNextNetworkToAdd->AddNewEntry(pCurEntry);
    add_property(pCurEntry); pCurEntry->m_pParent = this;
    nextIterator = m_pNextNetworkToAdd->m_thisIter;
    if( (++nextIterator)==m_networsList.end() ){
        nextIterator = m_networsList.begin();
    }
    m_pNextNetworkToAdd = *nextIterator;

    m_numberOfEntries.set_value(++nNumberOfEntries);
}


CLEAR_RET_TYPE pitz::daq::EqFctCollector::CLEAR_FUNC_NAME(void)
{
    NewSharedLockGuard< ::STDN::shared_mutex > aSharedGuard;

    DEBUG_APP_INFO(0,"!!!!!!!!!!!!!!!!!!!!!!!!!! %s, m_nWork=%d",__FUNCTION__,static_cast<int>(m_shouldWork));

    if(!m_shouldWork){return CAST_CLEAR_RET(0);}
    m_shouldWork = 0;

    aSharedGuard.LockShared(&m_lockForEntries);
    writeEntriesToConfig();
    aSharedGuard.UnlockShared();

    m_semaForRootThread.post();
    m_semaForLocalFileDeleter.post();

    m_threadRoot.join();
    m_threadLocalFileDeleter.join();

    // no need to synchronize, because all threads are gone

    for( auto netStruct : m_networsList){
        DEBUG_APP_INFO(0,"!!!!!! stopping and deleting network\n");
        delete netStruct;
    }

    m_networsList.clear();

    CleanSocketLibrary();

    DEBUG_APP_INFO(1," ");    
    return CAST_CLEAR_RET(0);
}


// this api is not safe, should be synchronized
void pitz::daq::EqFctCollector::writeEntriesToConfig()const
{
    FILE* fpConfig;
    char vcNewConfFileName[512];
    SingleEntry* pCurEntry;
    ::std::list< SingleEntry* >::const_iterator pIter, pIterEnd;
    const ::std::list< SingleEntry* >* pList;

    snprintf(vcNewConfFileName,511,"%s.nconf",name().c_str());
    fpConfig = fopen(vcNewConfFileName,"w");
    DEBUG_APP_INFO(2,"!!!!!!!!!!!!!!!!!!!!!!!!!!!! flName=%s, filePtr=%p ",vcNewConfFileName,static_cast<void*>(fpConfig) );

    if(fpConfig){
        fprintf(fpConfig,CONF_FILE_VERSION_START "%d\n",PITZ_DAQ_CURRENT_VERSION);
        for( auto netStruct : m_networsList){
            pList = &netStruct->daqEntries();
            pIterEnd = pList->end();
            for(pIter=pList->begin();pIter!=pIterEnd;++pIter){
                pCurEntry = *pIter;
                pCurEntry->writeContentToTheFile(fpConfig); // this takes care whether entry is deleted or not
            }
        }

        fclose(fpConfig);
        fpConfig=nullptr;
    }
}


void pitz::daq::EqFctCollector::LocalFileDeleterThread()
{
    std::string filePathLocal;

    while( shouldWork()  ){
        m_semaForLocalFileDeleter.wait(-1);
        while(m_fifoForLocalFileDeleter.frontAndPop(&filePathLocal)){
            DEBUG_APP_INFO(1," deleting file %s",filePathLocal.c_str());
            remove(filePathLocal.c_str());
        }
    } // while( m_nWork  ){
}


bool pitz::daq::EqFctCollector::AddNewEntryByUser(const char* a_entryLine)
{
    NewSharedLockGuard< ::STDN::shared_mutex > aGuard;
    bool bRet(false);

    aGuard.LockShared(&m_lockForEntries);

    if(IsAllowedToAdd(a_entryLine)){
        bRet=true;
        AddNewEntryNotLocked(entryCreationType::fromUser, a_entryLine);
    }

    aGuard.UnlockShared();

    return bRet;
}


bool pitz::daq::EqFctCollector::RemoveEntryByUser(const char* a_entryName)
{
    bool bRet(false);
    SingleEntry* pEntry;
    NewLockGuard< ::STDN::shared_mutex> lockGuard;

    lockGuard.Lock(&m_lockForEntries);

    if( (pEntry=FindEntry(a_entryName))){
        bRet=true;
        TryToRemoveEntryNotLocked(pEntry);
    }

    lockGuard.Unlock();

    return bRet;
}


void pitz::daq::EqFctCollector::TryToRemoveEntryNotLocked(SingleEntry* a_pEntry)
{
    bool bIsAllowedToDelete;
    int nNumberOfEntries(m_numberOfEntries.value());

    if(!a_pEntry){
        return;
    }

    bIsAllowedToDelete = a_pEntry->markEntryForDeleteAndReturnIfPossibleNow();

    m_pNextNetworkToAdd = a_pEntry->networkParent();
    m_numberOfEntries.set_value(--nNumberOfEntries);

    DEBUG_APP_INFO(0,"Number of entries remained is: %d",nNumberOfEntries);
    rem_property(a_pEntry);

    if(bIsAllowedToDelete){
        delete a_pEntry;
    }
}


void pitz::daq::EqFctCollector::RootFileCreator(std::string* a_pFilePathLocal, std::string* a_pFilePathRemote)
{
    Int_t nVersion = PITZ_DAQ_CURRENT_VERSION;
    int64_t llnCurFileSize;
    TTree* pTreeForVersion;
    TBranch *pBranchVersion;
	int nFailureIterations;
    std::string localDirPath, remoteDirPath, fileName;
	::std::vector<SingleEntry*> vectorEntries;

	{
		NewSharedLockGuard< ::STDN::shared_mutex > aGuard(&m_lockForEntries);
		for(const auto& aNetStrucs : m_networsList){
			for(SingleEntry* pNextEntry : aNetStrucs->daqEntries()){
				if(pNextEntry->lockEntryForRootFile()){
					vectorEntries.push_back(pNextEntry);
				}
			}
		}
	}

    CalculateRemoteDirPathAndFileName(&fileName,&remoteDirPath);
    CalcLocalDir(&localDirPath);
    *a_pFilePathLocal = localDirPath+"/"+fileName;
    *a_pFilePathRemote = remoteDirPath+"/"+fileName;
    //std::cout<<"locDirPath="<<localDirPath<<std::endl;
    //std::cout<<"remDirPath="<<remoteDirPath<<std::endl;
    mkdir_p(localDirPath.c_str(), S_IRWXU|S_IRWXG|S_IRWXO);
    mkdir_p(remoteDirPath.c_str(), S_IRWXU|S_IRWXG|S_IRWXO);

	nFailureIterations = 0;
	do{
		m_pRootFile = new NewTFile(a_pFilePathLocal->c_str());// SetCompressionLevel(1)
		if ((!m_pRootFile) || m_pRootFile->IsZombie() || (!m_pRootFile->IsOpen())){
			fprintf(stderr,"!!!! Error opening ROOT file (ln:%d) ",__LINE__);
			if((++nFailureIterations)>10){
				fprintf(stderr,"going to exit.\n");
				exit(1);
			}
			else{
				fprintf(stderr,"waiting for 5 s and trying again.\n");
				SleepMs(5000);
			}
		}
	}
	while((!m_pRootFile) || m_pRootFile->IsZombie() || (!m_pRootFile->IsOpen()));
	m_pRootFile->cd();
	m_pRootFile->Initialize(vectorEntries);

    pTreeForVersion = new TTree(VERSION_TREE_AND_BRANCH_NAME,"DATA");
    pBranchVersion=pTreeForVersion->Branch(VERSION_TREE_AND_BRANCH_NAME,nullptr,"version/I");
    if(!pBranchVersion){
        fprintf(stderr,"!!!! Error opening ROOT file going to exit. ln:%d\n",__LINE__);
        exit(-1);
    }
    pBranchVersion->SetAddress(&nVersion);
    pTreeForVersion->Fill();
    pTreeForVersion->AutoSave("SaveSelf");

	llnCurFileSize=m_pRootFile->GetSize();m_currentFileSize.set_value(static_cast<int>(llnCurFileSize));
    DEBUG_APP_INFO(2," ");

}



void pitz::daq::EqFctCollector::RootThreadFunction()
{
	m_pRootFile=nullptr;
    std::string filePathLocal, filePathRemote;
    SStructForFill strToFill;
    int64_t llnCurFileSize, llnMaxFileSize/*, llnCurFileSizeLastSaved=0*/;

    while( this->shouldWork()  ){

        m_semaForRootThread.wait(WAIT_INFINITE);

        while( m_fifoToFill.frontAndPop(&strToFill) ){

			if(!m_pRootFile){  // open root file
				// RootFileCreator will not exit if file creation is not done
				RootFileCreator(&filePathLocal,&filePathRemote);
            }

            strToFill.entry->Fill(strToFill.data);

			llnCurFileSize=m_pRootFile->GetSize();
            llnMaxFileSize = static_cast<Long64_t>(m_fileMaxSize.value());
            m_currentFileSize.set_value(static_cast<int>(llnCurFileSize));

            if(llnCurFileSize>=llnMaxFileSize){ // close root file
				m_pRootFile->FinalizeAndSaveAllTrees();
				m_pRootFile->TDirectory::DeleteAll();
				m_pRootFile->TDirectory::Close();
				m_pRootFile->Cleanup();
				delete m_pRootFile;
				m_pRootFile = NEWNULLPTR2;
                m_currentFileSize.set_value(-1);
                CopyFileToRemoteAndMakeIndexing(filePathLocal,filePathRemote);
            }

        } // while( m_fifoToFill.frontAndPop(&strToFill) ){

    } // while( this->shouldWork() )

	if(m_pRootFile){
		m_pRootFile->FinalizeAndSaveAllTrees();
		m_pRootFile->cd();
		m_pRootFile->TDirectory::DeleteAll();
		m_pRootFile->TDirectory::Close();
		m_pRootFile->Cleanup();
		delete m_pRootFile;
		m_pRootFile = NEWNULLPTR2;
        m_currentFileSize.set_value(-1);
        CopyFileToRemoteAndMakeIndexing(filePathLocal,filePathRemote);
    }

}


void pitz::daq::EqFctCollector::CopyFileToRemoteAndMakeIndexing(const std::string& a_fileLocal, const std::string& a_fileRemote)
{
    NewLockGuard< ::STDN::shared_mutex > aGuard;
    SingleEntry* pCurEntry;
    fstream index_fl;
    char vcBuffer[1024];

    ::std::list< SingleEntry* >::iterator pIter, pIterEnd, pIterToRemove;
    ::std::list< SingleEntry* >* pList;

    snprintf(vcBuffer,1023,"dccp -d 0 %s %s", a_fileLocal.c_str(), a_fileRemote.c_str());
    DEBUG_APP_INFO(2,"executing \"%s\"\n",vcBuffer);
    if(NewSystemStat(vcBuffer)){
        if(m_unErrorUnableToWriteToDcacheNum++==0){
            // send an email
            SendEmailCpp(m_rootFileNameBase.value(),m_expertsMailingList.value(),"Unable to copy the file", "Unable to copy the file");
        }
        if(m_unErrorUnableToWriteToDcacheNum == NUMER_OF_WAITING_FOR_ERR_REP){m_unErrorUnableToWriteToDcacheNum=0;}
        return;
    }

    aGuard.Lock(&m_lockForEntries);

    for( auto netStruct : m_networsList){
        pList = &netStruct->daqEntries();
        pIterEnd = pList->end();
        for(pIter=pList->begin();pIter!=pIterEnd;){
            pCurEntry = *pIter;
            if(pCurEntry->isPresentInLastFile()){
                sprintf(vcBuffer,"/doocs/data/DAQdata/INDEX/%s.idx",pCurEntry->daqName());
				index_fl.open(vcBuffer, ::std::ios_base::out | ::std::ios_base::app);
                if(index_fl.is_open()){
                    sprintf(vcBuffer,"%d:%02d,%d:%02d,%s",
                            pCurEntry->firstSecond(),pCurEntry->firstEventNumber(),
                            pCurEntry->lastSecond(),pCurEntry->lastEventNumber(),
                            a_fileRemote.c_str());
                    index_fl<<vcBuffer<<std::endl;
                    index_fl.close();
                } // if(index_fl.open(vcBuffer)){
                //pCurEntry->isPresent = false; // This is done automatically with SetTree(...) function
				if(pCurEntry->resetRootLockAndReturnIfDeletable()||pCurEntry->resetRooFileLockAndReturnIfDeletable()){
                    pIterToRemove = pIter++;
                    TryToRemoveEntryNotLocked(pCurEntry);
                    pList->erase(pIterToRemove);
                }
				else{++pIter;}
            } // if(pCurEntry->isPresentInLastFile){
            else{++pIter;}
        }
    }

    aGuard.Unlock();

    m_fifoForLocalFileDeleter.pushBack(a_fileLocal);
    m_semaForLocalFileDeleter.post();
}



uint64_t pitz::daq::EqFctCollector::shouldWork()const
{
    return m_shouldWork;
}


bool pitz::daq::EqFctCollector::AddJobForRootThread(DEC_OUT_PD(Header)* a_data, SingleEntry* a_pEntry)
{
    bool bPossibleToAdd = a_pEntry->lockEntryForRoot();

    if(bPossibleToAdd){
        m_fifoToFill.pushBack({a_pEntry,a_data});
        m_semaForRootThread.post();
    }

    return bPossibleToAdd;
}


/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////*/


pitz::daq::NewTFile::NewTFile(const char* a_filePath)
    :
      TFile(a_filePath,"UPDATE","DATA",1)
{
}


pitz::daq::NewTFile::~NewTFile()
{
}


void pitz::daq::NewTFile::Initialize(const ::std::vector< SingleEntry* >& a_list)
{
	for(auto& pEntry : a_list){
		pEntry->InitializeRootTree();
	}
}


void pitz::daq::NewTFile::Cleanup()
{
	//
}


void pitz::daq::NewTFile::FinalizeAndSaveAllTrees()
{
    //TList* keyList=GetListOfKeys();
    //if(keyList){
    //    TTree* pTree;
    //    const Int_t numberOfTrees = keyList->GetEntries();
    //    for(Int_t i(0);i<numberOfTrees;++i){
    //        pTree = static_cast<TTree *>(Get(keyList->At(i)->GetName()));
    //        if(pTree){
    //            pTree->AutoSave("SaveSelf");
    //        }
    //    }
    //}

    const size_t cunTreesNumber(m_trees.size());

    for(size_t i(0);i<cunTreesNumber;++i){
		m_trees[i]->Finalize();
        m_trees[i]->AutoSave("SaveSelf");
    }

}


void pitz::daq::NewTFile::AddNewTree(TreeForSingleEntry* a_pNewTree)
{
    m_trees.push_back(a_pNewTree);
}

/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

static int NewSystemStat(const char *a_command)
{
    int nReturn;
    int stdoutCopy = dup(STDOUT_FILENO);
    int stderrCopy = dup(STDERR_FILENO);
    freopen( "/dev/null", "w", stdout);
    freopen( "/dev/null", "w", stderr);
    nReturn = system(a_command);
    dup2(stdoutCopy,STDOUT_FILENO);
    dup2(stderrCopy,STDERR_FILENO);
    return nReturn;
}
