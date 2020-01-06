
// pitz_daq_eqfctcollector.cpp
// 2017 Nov 24

#include <cstdlib>
#define atoll       atol
#define strtoull    strtoul
#include "pitz_daq_eqfctcollector.hpp"
#include <signal.h>
#include <TFile.h>
#include <dirent.h>
#include "mailsender.h"
#include <sys/timeb.h>
#include <sys/time.h>
#include <time.h>

#define MINIMUM_ROOT_FILE_SIZE_HARD     1000

#define SEMA_WAIT_TIME_MS       10000
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

pitz::daq::EqFctCollector::EqFctCollector()
        :
          EqFct("Name = location"),
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
          m_entriesInError("ENTRIES.IN.ERROR",this)
{
    m_pNextNetworkToAdd = NEWNULLPTR2;

    m_unErrorUnableToWriteToDcacheNum = 0;
    m_shouldWork  = 0;
    m_bitwise64Reserved = 0;

    m_nNumberOfEntries = 0;
    m_nNumberOfFillThreadsFinal = 0;

    m_unErrorUnableToWriteToDcacheNum = 0;

    m_genEvent.set_ro_access();
    m_numberOfEntries.set_ro_access();
    m_rootDirPathBaseRemote.set_ro_access();
    //m_rootDirPathBaseLocal.set_ro_access();
    //m_folderName.set_ro_access();
    //m_rootFileNameBase.set_ro_access();
    //m_numberOfFillThreadsDesigned.set_ro_access();
    m_numberOfFillThreadsFinal.set_ro_access();

}


pitz::daq::EqFctCollector::~EqFctCollector()
{
    EqFctCollector::CLEAR_FUNC_NAME();
}


void pitz::daq::EqFctCollector::CalcLocalDir2(std::string* a_localDirName)STUPID_NON_CONST
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
            vcMinute[BUF_LEN_FOR_STRFTIME];

    strftime (vcYear,BUF_LEN_FOR_STRFTIME,"%Y",timeinfo);
    strftime (vcMonth,BUF_LEN_FOR_STRFTIME,"%m",timeinfo);
    strftime (vcDay,BUF_LEN_FOR_STRFTIME,"%d",timeinfo);
    strftime (vcHour,BUF_LEN_FOR_STRFTIME,"%H",timeinfo);
    strftime (vcMinute,BUF_LEN_FOR_STRFTIME,"%M",timeinfo);
    DUMMY_ARGS2(aWalltime);

    *a_remoteDirPath = m_rootDirPathBaseRemote.value()+std::string("/")+
             std::string(vcYear) + "/"+
             std::string(vcMonth)+ "/"+
             std::string(m_folderName.value());

    *a_fileName = m_rootFileNameBase.value()+std::string(".")+
            std::string(vcYear)+"-"+std::string(vcMonth)+"-"+
            std::string(vcDay)+"-"+std::string(vcHour)+vcMinute +
            std::string(".root");
}


int pitz::daq::EqFctCollector::write(fstream &a_fprt)
{
    int nReturn = EqFct::write(a_fprt);
    WriteEntriesToConfig();
    return nReturn;
}


void pitz::daq::EqFctCollector::init(void)
{
    SNetworkStruct* pNetworkToAdd2;
    int i;
    int nNumberOfFillThreadsDesigned;
    FILE* fpConfig;
    char data[data_length];
    char vcNewConfFileName[512];
    char* pn;

    if(m_shouldWork){return;}
    m_shouldWork = 1;

    m_numberOfEntries.set_value(0);
    m_entriesInError.set_value("");
    m_numberOfEntriesInError.set_value(0);
    m_currentFileSize.set_value(0);
    if(m_fileMaxSize.value()<MINIMUM_ROOT_FILE_SIZE_HARD){m_fileMaxSize.set_value(MINIMUM_ROOT_FILE_SIZE_HARD);}

    g_nLogLevel = m_logLevel.value();
    InitSocketLibrary();

    WriteLockEntries2();

    nNumberOfFillThreadsDesigned = m_numberOfFillThreadsDesigned.value();
    if(nNumberOfFillThreadsDesigned<=0){nNumberOfFillThreadsDesigned=1;m_numberOfFillThreadsDesigned.set_value(1);}
    m_nNumberOfFillThreadsFinal = 0;

    m_nNumberOfEntries = 0;
    snprintf(vcNewConfFileName,511,"%s.nconf",name().c_str());
    fpConfig = fopen(vcNewConfFileName,"r");

    DEBUG_APP_INFO(1,"!!!!!!!!!!!!!!!!!!!!!!!!!!!! flName=%s, filePtr=%p ", vcNewConfFileName, static_cast<void*>(fpConfig) );

    for(i=0;i<nNumberOfFillThreadsDesigned;++i){

        pNetworkToAdd2 = this->CreateNewNetworkStruct();
        m_networsList.push_front(pNetworkToAdd2);
        pNetworkToAdd2->m_thisIter = m_networsList.begin();

    }
    m_pNextNetworkToAdd = m_networsList.front();
    m_nNumberOfFillThreadsFinal = static_cast<decltype (m_nNumberOfFillThreadsFinal)>(m_networsList.size());

    if(!fpConfig){goto finalizeStuffPoint;}

    while ( fgets(data, data_length, fpConfig) ){
        //if(  data[0] == '#'  ) continue;
        pn = strpbrk(data,s_LN);
        if( ( !pn ) || ( pn[0] == '#' ) ) continue;
        if(IsAllowedToAdd2(pn)){
            AddNewEntryNotLocked(entryCreationType::fromConfigFile, data);
        }
    }
    fclose(fpConfig);

finalizeStuffPoint:
    WriteUnlockEntries2();
    DEBUG_APP_INFO(1," ");

    m_threadForEntryAdding = ::STDN::thread(&EqFctCollector::EntryAdderDeleter,this);
    m_threadRoot = ::STDN::thread(&EqFctCollector::RootThreadFunction,this);
    m_threadLocalFileDeleter = ::STDN::thread(&EqFctCollector::LocalFileDeleterThread,this);
}


bool pitz::daq::EqFctCollector::IsAllowedToAdd2(const char* a_newEntryLine)
{
    // should be more general search
    ::std::string aEntryName(a_newEntryLine);
    char* pcEntryName = const_cast<char*>(aEntryName.data());
    char* cpcBrk = strpbrk(pcEntryName,POSIIBLE_TERM_SYMBOLS);
    if(cpcBrk){*cpcBrk=0;}

    if( FindEntry(pcEntryName) ){
        return false;
    }
    if(FindEntryInAdding(pcEntryName)){
        return false;
    }
    if(FindEntryInDeleting(pcEntryName)){
        return false;
    }

    return true;
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


bool pitz::daq::EqFctCollector::FindEntryInAdding(const char* )
{
    return false;
}

bool pitz::daq::EqFctCollector::FindEntryInDeleting(const char *)
{
    return false;
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

    m_mutexForEntriesInError.lock();
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
    int nNumber = m_numberOfEntriesInError.value();

    if(nNumber>0){
        char* pcNext;
        char* pcBuffer ;
        const size_t entryNameLen(strlen(a_entryName));
        size_t unWholeStrLen;
        //char vcBuffer[1024];
        std::string strEntries ;

        m_mutexForEntriesInError.lock();

        strEntries = m_entriesInError.value();
        pcBuffer = const_cast<char*>(strEntries.c_str());
        pcNext=strstr(pcBuffer,a_entryName);
        if(!pcNext){m_mutexForEntriesInError.unlock();return;}

        if(*(pcNext+entryNameLen)!=DELIMETER_SYMBOL[0]){
            m_mutexForEntriesInError.unlock();
            return;
        }

        unWholeStrLen = strEntries.length();
        m_numberOfEntriesInError.set_value(--nNumber);
        DEBUG_APP_INFO(2,"!!!!!!!!!!!!!!! DecrementErrors setting to %d",nNumber);
        //strncpy(vcBuffer,cpcEntries,1023);

        memmove(pcNext,pcNext+entryNameLen+1,unWholeStrLen-static_cast<size_t>(pcNext-pcBuffer)-entryNameLen);
        m_entriesInError.set_value(pcBuffer);

        m_mutexForEntriesInError.unlock();
    }
}


int pitz::daq::EqFctCollector::parse_old_config2(const std::string& a_daqConfFilePath)
{
    FILE *fpConfig;
    const char* pn;
    const std::string& daqConfFilePath = a_daqConfFilePath;
    char    data[data_length];
    int     nReturn(-1);

    WriteLockEntries2();

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
        if(IsAllowedToAdd2(pn)){
            AddNewEntryNotLocked(entryCreationType::fromOldFile, data);
        }
    }
    fclose(fpConfig);
    //m_fifoToFill.ResizeCash(m_nNumberOfEntries*4);
    DEBUG_APP_INFO(0,"!!!!!!! numberOfEntries=%d, numberOfEntriesDcs=%d",m_nNumberOfEntries,m_numberOfEntries.value());
    nReturn = 0;

returnPoint:
    WriteEntriesToConfig();
    WriteUnlockEntries2();
    return nReturn;

}


void pitz::daq::EqFctCollector::AddNewEntryNotLocked(entryCreationType::Type a_creationType,const char* a_entryLine)
{
    ::std::list< SNetworkStruct* >::iterator    nextIterator;
    SingleEntry *pCurEntry(nullptr);

    try{
        pCurEntry = CreateNewEntry(a_creationType,a_entryLine);
    }
    catch(...){
        return;
    }

    m_pNextNetworkToAdd->AddNewEntry(pCurEntry);
    nextIterator = m_pNextNetworkToAdd->m_thisIter;
    if( (++nextIterator)==m_networsList.end() ){
        nextIterator = m_networsList.begin();
    }
    m_pNextNetworkToAdd = *nextIterator;

    m_numberOfEntries.set_value(++m_nNumberOfEntries);
    m_numberOfFillThreadsFinal.set_value(m_nNumberOfFillThreadsFinal);
}


CLEAR_RET_TYPE pitz::daq::EqFctCollector::CLEAR_FUNC_NAME(void)
{
    DEBUG_APP_INFO(0,"!!!!!!!!!!!!!!!!!!!!!!!!!! %s, m_nWork=%d",__FUNCTION__,static_cast<int>(m_shouldWork));

    if(!m_shouldWork){return CAST_CLEAR_RET(0);}
    m_shouldWork = 0;

    WriteEntriesToConfig();

    m_semaForRootThread.post();
    m_semaForLocalFileDeleter.post();
    m_semaForEntryAdder.post();

    m_threadRoot.join();
    m_threadLocalFileDeleter.join();
    m_threadForEntryAdding.join();

    for( auto netStruct : m_networsList){
        DEBUG_APP_INFO(0,"!!!!!! stopping network\n");
        netStruct->StopThreadAndClear();
        delete netStruct;
    }

    m_networsList.clear();

    CleanSocketLibrary();

    DEBUG_APP_INFO(1," ");    
    return CAST_CLEAR_RET(0);
}


// this api is not safe, should be synchronized
void pitz::daq::EqFctCollector::WriteEntriesToConfig()const
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
        for( auto netStruct : m_networsList){
            pList = &netStruct->daqEntries();
            pIterEnd = pList->end();
            for(pIter=pList->begin();pIter!=pIterEnd;++pIter){
                pCurEntry = *pIter;
                pCurEntry->WriteContentToTheFile(fpConfig);
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
        while(m_fifoForLocalFileDeleter.size()>0){
            filePathLocal = m_fifoForLocalFileDeleter.front();
            m_fifoForLocalFileDeleter.pop();
            DEBUG_APP_INFO(1," deleting file %s",filePathLocal.c_str());
            remove(filePathLocal.c_str());
        }
    } // while( m_nWork  ){
}


void pitz::daq::EqFctCollector::EntryAdderDeleter()
{
    while( m_shouldWork  ){
        m_semaForEntryAdder.wait(-1);

        if(m_entriesToAdd.size()){
            WriteLockEntries2();

            while(m_entriesToAdd.size()){
                //AddNewEntryPrivate(entryCreationType::fromUser, ::std::move(m_entriesToAdd.front()));
                AddNewEntryNotLocked(entryCreationType::fromUser, m_entriesToAdd.front().c_str());
                m_entriesToAdd.pop();
            }

            WriteUnlockEntries2();
        }  // if(bAction){

    }  // while( m_nWork2  ){
}


bool pitz::daq::EqFctCollector::AddNewEntryByUser(const char* a_entryLine)
{
    bool bRet(false);
    ReadLockEntries2();

    if(IsAllowedToAdd2(a_entryLine)){
        bRet=true;
        m_entriesToAdd.push(a_entryLine);
        m_semaForEntryAdder.post();
    }

    ReadUnlockEntries2();

    return bRet;
}


bool pitz::daq::EqFctCollector::RemoveEntryByUser(const char* a_entryName)
{
    bool bRet(false);
    SingleEntry* pEntry;

    this->WriteLockEntries2();

    if(FindEntryInAdding(a_entryName)){
        //bRet = true;
        // todo: remove from adding entries list
        return true;
    }

    if( (pEntry=FindEntry(a_entryName))){
        bRet=true;
        TryToRemoveEntryNotLocked(pEntry);
    }


    this->WriteUnlockEntries2();

    return bRet;
}


void pitz::daq::EqFctCollector::TryToRemoveEntryNotLocked(SingleEntry* a_pEntry)
{
    bool bIsAllowedToDelete;
    SNetworkStruct* pNetworkParent;

    if(!a_pEntry){
        return;
    }

    bIsAllowedToDelete = a_pEntry->markEntryForDeleteAndReturnIfPossibleNow(); 

    //WriteLockEntries2();
    pNetworkParent = a_pEntry->CleanEntryNoFree();
    m_pNextNetworkToAdd = pNetworkParent;
    m_numberOfEntries.set_value(--m_nNumberOfEntries);
    //WriteUnlockEntries2();

    DEBUG_APP_INFO(0,"Number of entries remained is: %d",m_nNumberOfEntries);

    if(bIsAllowedToDelete){
        delete a_pEntry;
    }
}



void pitz::daq::EqFctCollector::WriteLockEntries2()
{
    pthread_t handleToThread;

    m_lockForEntries2.WriteLockWillBeCalled();


    for(auto pNetwork : m_networsList){
        handleToThread = static_cast<pthread_t>(pNetwork->m_pThread->native_handle());
        pthread_kill(handleToThread,SIGNAL_FOR_CANCELATION);
    }


    m_lockForEntries2.lock();

}


void pitz::daq::EqFctCollector::WriteUnlockEntries2()
{
    m_lockForEntries2.unlock();
}


void pitz::daq::EqFctCollector::ReadLockEntries2()
{
    m_lockForEntries2.lock_shared();
}


void pitz::daq::EqFctCollector::ReadUnlockEntries2()
{
    m_lockForEntries2.unlock_shared();
}


uint64_t pitz::daq::EqFctCollector::shouldWork()const
{
    return m_shouldWork;
}


pitz::daq::SStructForFill pitz::daq::EqFctCollector::GetAndDeleteFirstData()
{
    SStructForFill strToRet(m_fifoToFill.front());
    m_fifoToFill.pop();
    //strToRet.entry->m_isInTheRootFifo = 0;
    return strToRet;
}


void pitz::daq::EqFctCollector::CheckGenEventError(int* a_pnPreviousTime, int* a_pnPreviousGenEvent)
{
    struct timeb currentTime;
    int nGenEvent;

    ftime(&currentTime);
    if((currentTime.time-(*a_pnPreviousTime))>=(SEMA_WAIT_TIME_MS/1000)){
        nGenEvent = m_genEvent.value();
        if(nGenEvent==(*a_pnPreviousGenEvent)){
            set_error(2018,"Gen event error");
        }
        else{
            set_error(0,"ok");
            (*a_pnPreviousGenEvent) = nGenEvent;
        }

        (*a_pnPreviousTime) = static_cast<int>(currentTime.time);
    }
}


void pitz::daq::EqFctCollector::RootThreadFunction()
{
    TFile *pGlobalRootFileInitial,*pRootFile = NEWNULLPTR2;
    std::string filePathLocal, filePathRemote, localDirPath, remoteDirPath, fileName;
    int nSeconds, nEventNumber;
    int nPreviousGenEvent(0),nPreviousTime(0);
    SStructForFill strToFill;
    SingleEntry* pCurEntry;
    DEC_OUT_PD(SingleData)* pCurrentData;
    int64_t llnCurFileSize;

    while( this->shouldWork()  ){

        CalculateRemoteDirPathAndFileName(&fileName,&remoteDirPath);
        CalcLocalDir2(&localDirPath);
        filePathLocal = localDirPath+"/"+fileName;
        filePathRemote = remoteDirPath+"/"+fileName;
        std::cout<<"locDirPath="<<localDirPath<<std::endl;
        std::cout<<"remDirPath="<<remoteDirPath<<std::endl;
        mkdir_p(localDirPath.c_str(), S_IRWXU|S_IRWXG|S_IRWXO);
        mkdir_p(remoteDirPath.c_str(), S_IRWXU|S_IRWXG|S_IRWXO);

        pRootFile = new TFile(filePathLocal.c_str(),"UPDATE","DATA",1);// SetCompressionLevel(1)
        if ((!pRootFile) || pRootFile->IsZombie()){
            fprintf(stderr,"!!!! Error opening ROOT file going to exit. ln:%d\n",__LINE__);
            exit(-1);
        }
        pGlobalRootFileInitial = gFile;
        pRootFile->cd(); gFile = pRootFile;
        llnCurFileSize=pRootFile->GetSize();m_currentFileSize.set_value(llnCurFileSize);
        DEBUG_APP_INFO(2," ");

        // the while below corresponds to file filling
        for( ;this->shouldWork()&&(pRootFile->GetSize()<m_fileMaxSize.value());llnCurFileSize=pRootFile->GetSize(),m_currentFileSize.set_value(llnCurFileSize)  ){

            m_semaForRootThread.wait(SEMA_WAIT_TIME_MS);
            CheckGenEventError(&nPreviousTime,&nPreviousGenEvent);
            while( this->shouldWork() && (m_fifoToFill.size()>0) ){
                strToFill = GetAndDeleteFirstData();
                pCurEntry = strToFill.entry;
                pCurrentData = strToFill.data;
                nSeconds = pCurrentData->timestampSeconds;
                nEventNumber=pCurrentData->eventNumber;

                pCurEntry->Fill(strToFill.data,nSeconds,nEventNumber);

            } // while( this->shouldWork() && (m_fifoToFill.size()>0) ){
        }// while( this->shouldWork() && nContinueFill  ){

        pRootFile->cd();
        gFile = pRootFile;
        pRootFile->TDirectory::DeleteAll();
        pRootFile->TDirectory::Close();
        delete pRootFile;
        pRootFile = NEWNULLPTR2;
        gFile = pGlobalRootFileInitial;

        CopyFileToRemoteAndMakeIndexing(filePathLocal,filePathRemote);

    } // while( m_nWork && (m_threadStatus.value() == 1) )

}



void pitz::daq::EqFctCollector::CopyFileToRemoteAndMakeIndexing(const std::string& a_fileLocal, const std::string& a_fileRemote)
{
    SingleEntry* pCurEntry;
    fstream index_fl;
    char vcBuffer[1024];

    ::std::list< SingleEntry* >::const_iterator pIter, pIterEnd;
    const ::std::list< SingleEntry* >* pList;

    snprintf(vcBuffer,1023,"dccp -d 0 %s %s", a_fileLocal.c_str(), a_fileRemote.c_str());
    DEBUG_APP_INFO(2,"executing \"%s\"\n",vcBuffer);
    if(system(vcBuffer)!=0){
        if(m_unErrorUnableToWriteToDcacheNum++==0){
            // send an email
            SendEmailCpp(m_rootFileNameBase.value(),m_expertsMailingList.value(),"Unable to copy the file", "Unable to copy the file");
        }
        if(m_unErrorUnableToWriteToDcacheNum == NUMER_OF_WAITING_FOR_ERR_REP){m_unErrorUnableToWriteToDcacheNum=0;}
        return;
    }

    //m_mutexForEntries.lock_shared();

    for( auto netStruct : m_networsList){
        pList = &netStruct->daqEntries();
        pIterEnd = pList->end();
        for(pIter=pList->begin();pIter!=pIterEnd;++pIter){
            pCurEntry = *pIter;
            if(pCurEntry->isPresentInCurrentFile()){
                sprintf(vcBuffer,"/doocs/data/DAQdata/INDEX/%s.idx",pCurEntry->daqName());
                index_fl.open(vcBuffer,ios_base::out | ios_base::app);
                if(index_fl.is_open()){
                    sprintf(vcBuffer,"%d:%02d,%d:%02d,%s",
                            pCurEntry->firstSecond(),pCurEntry->firstEventNumber(),
                            pCurEntry->lastSecond(),pCurEntry->lastEventNumber(),
                            a_fileRemote.c_str());
                    index_fl<<vcBuffer<<std::endl;
                    index_fl.close();
                } // if(index_fl.open(vcBuffer)){
                //pCurEntry->isPresent = false; // This is done automatically with SetTree(...) function
            } // if(pCurEntry->isPresent){
        }
    }

    //m_mutexForEntries.unlock_shared();

    m_fifoForLocalFileDeleter.push(a_fileLocal);
    m_semaForLocalFileDeleter.post();
}


//void pitz::daq::EqFctCollector::RemoveOneEntry2(SingleEntry* a_pEntry)
//{
//    WriteLockEntries();
//    if(!IsAllowedToAdd2(a_entryLine)){bRet = false;goto returnPoint;}
//    AddNewEntryPrivate(a_creationType,a_entryLine);
//returnPoint:
//    WriteUnlockEntries();
//}


bool pitz::daq::EqFctCollector::AddJobForRootThread(DEC_OUT_PD(SingleData)* a_data, SingleEntry* a_pEntry)
{
    bool bPossibleToAdd = a_pEntry->lockEntryForRoot();

    if(bPossibleToAdd){
        m_fifoToFill.push({a_pEntry,a_data});
        //if(CHECK_QUEUE_ADD(m_fifoToFill.push({a_pEntry,a_data}))){
        //    m_semaForRootThread.post();
        //    return true;
        //}
        //else{
        //    DEBUG_APP_INFO(0,"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! No place in the fifo!\n");
        //    return false;
        //}
    }

    return bPossibleToAdd;
}


/*//////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#ifndef USE_SHARED_LOCK
#define MAX_READ_LOCK_COUNT       0xffff
#define WRITE_LOCK_ADD_VALUE      ((1<<16)|1)
#define WRITE_LOCK_COUNT(_value)  ((_value)>>16)
#define READ_LOCK_COUNT(_value)   static_cast<uint16_t>(_value)
#endif

pitz::daq::EntryLock::EntryLock()
{
    m_writerThread = NULL_THREAD_HANDLE;
    m_nLocksCount = 0;

    m_isGoingToWriteLock = 0;
    m_bitwiseReserved = 0;
}


void pitz::daq::EntryLock::WriteLockWillBeCalled()
{
    m_isGoingToWriteLock = 1;
}


void pitz::daq::EntryLock::lock()
{
    pthread_t thisThread = pthread_self();

#ifdef USE_SHARED_LOCK

    if(thisThread!=m_lockerThread){
        ::STDN::shared_mutex::lock();
        m_lockerThread = thisThread;
    }
    ++m_nLocksCount; // __atomic_fetch_add(&m_nRootStopCount,1,__ATOMIC_RELAXED);

#else   // #ifdef USE_SHARED_LOCK
    uint32_t nReturn = __atomic_fetch_add(&m_nLocksCount,WRITE_LOCK_ADD_VALUE,__ATOMIC_RELAXED);

    if(!nReturn){
        m_writerThread=thisThread;
        m_isGoingToWriteLock = 0;
        return;
    }

    if(m_writerThread==thisThread){
        m_isGoingToWriteLock = 0;
        return;
    }

    while(nReturn){
        SleepMs(1);
        __atomic_fetch_sub(&m_nLocksCount,WRITE_LOCK_ADD_VALUE,__ATOMIC_RELAXED);
        nReturn = __atomic_fetch_add(&m_nLocksCount,WRITE_LOCK_ADD_VALUE,__ATOMIC_RELAXED);
    }

    m_writerThread=thisThread;
    m_isGoingToWriteLock = 0;

#endif  // #ifdef USE_SHARED_LOCK
}


void pitz::daq::EntryLock::unlock()
{
    if(__atomic_fetch_sub(&m_nLocksCount,WRITE_LOCK_ADD_VALUE,__ATOMIC_RELAXED) == WRITE_LOCK_ADD_VALUE){
#ifdef USE_SHARED_LOCK
        ::STDN::shared_mutex::unlock();
#endif
        m_writerThread = NULL_THREAD_HANDLE;
    }
}


#ifndef USE_SHARED_LOCK

void pitz::daq::EntryLock::lock_shared()
{
    pthread_t thisThread = pthread_self();

    while(m_isGoingToWriteLock){
        SleepMs(1);
    }

    uint32_t nReturn = __atomic_fetch_add(&m_nLocksCount,1,__ATOMIC_RELAXED);

    while(nReturn>MAX_READ_LOCK_COUNT){
        if(m_writerThread==thisThread){
            return;
        }
        __atomic_fetch_sub(&m_nLocksCount,1,__ATOMIC_RELAXED);
        SleepMs(1);
        nReturn = __atomic_fetch_add(&m_nLocksCount,1,__ATOMIC_RELAXED);
    }
}


void pitz::daq::EntryLock::unlock_shared()
{
    __atomic_fetch_sub(&m_nLocksCount,1,__ATOMIC_RELAXED);
}

#endif   // #ifndef USE_SHARED_LOCK

