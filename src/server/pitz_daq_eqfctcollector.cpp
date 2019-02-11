
// pitz_daq_eqfctcollector.cpp
// 2017 Nov 24

#include "pitz_daq_eqfctcollector.hpp"
#include <signal.h>
#include <TFile.h>
#include <dirent.h>
#include "mailsender.h"
#include <sys/timeb.h>
#include <sys/time.h>
#include <time.h>

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

static void SignalHandler(int){}

pitz::daq::EqFctCollector::EqFctCollector()
        :
          EqFct("Name = location"),
          m_genEvent("GEN_EVENT value",this),
          m_rootLength("ROOT_LENGTH max length of file", this),
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
    m_unErrorUnableToWriteToDcacheNum = 0;

    m_genEvent.set_ro_access();
    m_numberOfEntries.set_ro_access();
    m_rootDirPathBaseRemote.set_ro_access();
    m_rootDirPathBaseLocal.set_ro_access();
    m_folderName.set_ro_access();
    m_rootFileNameBase.set_ro_access();
    m_numberOfFillThreadsDesigned.set_ro_access();
    m_numberOfFillThreadsFinal.set_ro_access();

    m_pEntryFirst = NULL;
    m_networkFirst=m_networkLast = NULL;
    m_bRootStopped = true;

}


pitz::daq::EqFctCollector::~EqFctCollector()
{
    EqFctCollector::cancel();
}


void pitz::daq::EqFctCollector::CalcLocalDir2(std::string* a_localDirName)STUPID_NON_CONST
{
    *a_localDirName = m_rootDirPathBaseLocal.value() + std::string("/") + m_folderName.value();
}


void pitz::daq::EqFctCollector::CalculateRemoteDirPathAndFileName(std::string* a_fileName,std::string* a_remoteDirPath)STUPID_NON_CONST
{
    time_t  aWalltime (::time(0));
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
    DUMMY_ARGS__(aWalltime);

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
    // old complete
    {
        FILE* fpConfig;
        char data[data_length];
        char vcNewConfFileName[512];


        m_entriesInError.set_value("");
        DEBUG_APP_INFO(2,"!!!!!!!!!!!!!!!!!!!!!!!!!!!!! setting to 0");
        m_numberOfEntriesInError.set_value(0);


        m_bRootStopped=true;
        m_nWork = 1;
        m_nNumberOfFillThreadsDesigned = m_numberOfFillThreadsDesigned.value();
        m_nIndexToNextNet = 0;
        if(m_nNumberOfFillThreadsDesigned<=0){m_nNumberOfFillThreadsDesigned=1;m_numberOfFillThreadsDesigned.set_value(1);}
        m_nNumberOfFillThreadsFinal = 0;

        m_nNumberOfEntries = 0;
        snprintf(vcNewConfFileName,511,"%s.nconf",name().c_str());
        fpConfig = fopen(vcNewConfFileName,"r");

        DEBUG_APP_INFO(1,"!!!!!!!!!!!!!!!!!!!!!!!!!!!! flName=%s, filePtr=%p ", vcNewConfFileName, fpConfig );

        if(!fpConfig){return;}

        while ( fgets(data, data_length, fpConfig) != NULL)
        {
            if(  data[0] == '#'  ) continue;
            AddNewEntry(entryCreationType::fromConfigFile, data);
        }
        fclose(fpConfig);
        if(m_nNumberOfEntries>0){m_bRootStopped=false;}
    }

    // old post_init
    {
        DEBUG_APP_INFO(1," ");

        InitSocketLibrary();

        g_nLogLevel = m_logLevel.value();

        m_numberOfEntries.set_value(m_nNumberOfEntries);  // avelord e

        m_threadRoot = STDN::thread(&EqFctCollector::RootThreadFunction,this);
        m_threadLocalFileDeleter = STDN::thread(&EqFctCollector::LocalFileDeleterThread,this);
    }
}


void pitz::daq::EqFctCollector::DataGetterThreadPrivate(SNetworkStruct* a_pNet)
{
    struct sigaction sigAction;

    sigAction.sa_sigaction = NULL;
    sigAction.sa_handler = SignalHandler;
    sigfillset(&sigAction.sa_mask);
    sigdelset(&sigAction.sa_mask,SIGNAL_FOR_CANCELATION);
    sigAction.sa_flags = 0;
    sigAction.sa_restorer = NULL; // not used

    // we have to init sig handle, because in some cases, we will stop by interrupt
    sigaction(SIGNAL_FOR_CANCELATION,&sigAction,NULL);

    this->DataGetterThread(a_pNet);
}


bool pitz::daq::EqFctCollector::IsAllowedToAdd2(SingleEntry* a_newEntry)
{
    // should be more general search
    return FindEntry(a_newEntry->daqName()) ? false : true;
}


pitz::daq::SingleEntry* pitz::daq::EqFctCollector::FindEntry(const char* a_entryName)
{
    for(SingleEntry* pCurEntry(m_pEntryFirst);pCurEntry;pCurEntry=pCurEntry->next){
        if(strcmp(pCurEntry->daqName(),a_entryName)==0){return pCurEntry;}
    }

    return NULL;
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

        memmove(pcNext,pcNext+entryNameLen+1,unWholeStrLen-(pcNext-pcBuffer)-entryNameLen);
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
    bool bRootInitial (m_bRootStopped);

    m_bRootStopped = true;

    fpConfig =	fopen(daqConfFilePath.c_str(),"r");
    DEBUG_APP_INFO(1,"fpConfig=%p",fpConfig);
    if(!fpConfig){
        std::cerr<<"Unable to open the file "<<daqConfFilePath<<std::endl;
        perror("\n");
        m_bRootStopped = bRootInitial;
        return 1;
    }
    DEBUG_APP_INFO(2," ");
    while ( fgets(data, data_length, fpConfig) != NULL)
    {
        pn = strpbrk(data,s_LN);
        if( ( pn == 0 ) || ( pn[0] == '#' ) ) continue;
        //DEBUG_("\n");
        AddNewEntry(entryCreationType::fromOldFile, data);
        //DEBUG_("\n");
    }
    fclose(fpConfig);
    //m_fifoToFill.ResizeCash(m_nNumberOfEntries*4);
    DEBUG_APP_INFO(0,"!!!!!!! numberOfEntries=%d, numberOfEntriesDcs=%d",m_nNumberOfEntries,m_numberOfEntries.value());

    WriteEntriesToConfig();
    m_bRootStopped = false;
    return 0;

}



bool pitz::daq::EqFctCollector::AddNewEntry(entryCreationType::Type a_creationType,
                                            const char* a_entryLine)
{
    SNetworkStruct* pNetworkToAdd;
    SingleEntry *pCurEntry(NULL);
    bool bRet(false), bNewCreated(false);

    m_mutexForEntries.writeLock();

    if(m_nNumberOfFillThreadsFinal<m_nNumberOfFillThreadsDesigned )
    {
        STDN::thread* pThread;
        pNetworkToAdd = new SNetworkStruct(this,m_networkLast);
        if(!pNetworkToAdd){
            bRet = false;
            goto returnPoint;
        }
        pThread = new STDN::thread(&EqFctCollector::DataGetterThreadPrivate,this,pNetworkToAdd);
        pNetworkToAdd->SetThread(pThread);
        m_nNumberOfFillThreadsFinal++;
        bNewCreated = true;
    }
    else{
        pNetworkToAdd = m_pLastNetworkAdded->next();
        if(!pNetworkToAdd){pNetworkToAdd=m_networkFirst;}
    }

    if(!pNetworkToAdd){bRet = false;goto returnPoint;}

    try{
        //if(memcmp(a_entryLine,"TDS_X2TIMER_EVENT2_20150821",strlen("TDS_X2TIMER_EVENT2_20150821"))==0){
//            DEBUG_APP_INFO(3,"found!");
        //}
        pCurEntry = CreateNewEntry(a_creationType,a_entryLine);
    }
    catch(...){
        bRet = false;
        pCurEntry = NULL;
    }

    if(!pCurEntry){bRet = false;goto returnPoint;}
    if(!IsAllowedToAdd2(pCurEntry)){bRet = false;goto returnPoint;}

    bRet = pNetworkToAdd->AddNewEntry(pCurEntry);
    if(!bRet){
        delete pCurEntry;pCurEntry=NULL;
        if(bNewCreated){delete pNetworkToAdd;--m_nNumberOfFillThreadsFinal;pNetworkToAdd=NULL;}
        goto returnPoint;
    }

    if(bNewCreated){if(!m_networkFirst){m_networkFirst=pNetworkToAdd;}m_networkLast = pNetworkToAdd;}

    //if(pNetworkToAdd == m_networkFirst){m_pEntryFirst=m_networkFirst->first();}
    m_pEntryFirst=m_networkFirst->first();
    ++m_nNumberOfEntries;
    bRet = true;
    m_pLastNetworkAdded = pNetworkToAdd;

returnPoint:

    m_mutexForEntries.unlock();
    if(!bRet && pNetworkToAdd && pCurEntry){ pNetworkToAdd->RemoveEntry(pCurEntry);}
    m_numberOfEntries.set_value(m_nNumberOfEntries);
    m_numberOfFillThreadsFinal.set_value(m_nNumberOfFillThreadsFinal);
    return bRet;
}



void pitz::daq::EqFctCollector::cancel(void)
{
    SNetworkStruct* pNetStrNext;

    WriteEntriesToConfig();

    if(m_nWork==0){return;}
    m_nWork = 0;

    m_semaForRootThread.post();
    m_semaForLocalFileDeleter.post();

    m_threadRoot.join();
    m_threadLocalFileDeleter.join();

    for(SNetworkStruct* pNetStr=m_networkFirst;pNetStr != NULL;){
        pNetStrNext = pNetStr->next();
        delete pNetStr;
        pNetStr = pNetStrNext;
    }

    CleanSocketLibrary();

    DEBUG_APP_INFO(1," ");
}


void pitz::daq::EqFctCollector::WriteEntriesToConfig()const
{
    FILE* fpConfig;
    char vcNewConfFileName[512];

    snprintf(vcNewConfFileName,511,"%s.nconf",name().c_str());
    fpConfig = fopen(vcNewConfFileName,"w");
    DEBUG_APP_INFO(2,"!!!!!!!!!!!!!!!!!!!!!!!!!!!! flName=%s, filePtr=%p ", vcNewConfFileName, fpConfig );

    if(fpConfig){
        SingleEntry* pEntry = m_pEntryFirst;

        while(pEntry){
            pEntry->WriteContentToTheFile(fpConfig);
            fprintf(fpConfig,"\n");
            pEntry = pEntry->next;
        }

        fclose(fpConfig);
        fpConfig=NULL;
    }
}


void pitz::daq::EqFctCollector::LocalFileDeleterThread()
{
    std::string filePathLocal;

    while( m_nWork  ){
        m_semaForLocalFileDeleter.wait(-1);
        while(m_fifoForLocalFileDeleter.Extract(&filePathLocal)){
            DEBUG_APP_INFO(1," deleting file %s",filePathLocal.c_str());
            remove(filePathLocal.c_str());
        }
    } // while( m_nWork  ){
}


void pitz::daq::EqFctCollector::RootThreadFunction()
{
    TTree* pTree2;
    TFile*  file;
    MemoryBase* pMemToProc;
    SingleEntry *pCurEntry;
    //volatile struct timeval prevTime={0,0};
    //struct timeval currentTime;
    int nPreviousTime(0);
    struct timeb currentTime;
    std::string filePathLocal, filePathRemote, localDirPath, remoteDirPath, fileName;
    int nEntries;
    int nFileSize;
    int	nContinueFill;
    int nSeconds, nEventNumber;
    int nSemaReturn;
    int nPreviousGenEvent(0), nGenEvent, nError=get_error();
    bool bTimeToSave;
    char vcError[128];

    while( m_nWork  )
    {
        if(m_bRootStopped){
            SleepMs(2000);
            continue;
        }

        m_mutexForEntries.readLock();
        nEntries = m_nNumberOfEntries;
        m_mutexForEntries.unlock();

        if(nEntries<1){
            SleepMs(2000);
            continue;
        }

        CalculateRemoteDirPathAndFileName(&fileName,&remoteDirPath);
        CalcLocalDir2(&localDirPath);
        filePathLocal = localDirPath+"/"+fileName;
        filePathRemote = remoteDirPath+"/"+fileName;

        std::cout<<"locDirPath="<<localDirPath<<std::endl;
        std::cout<<"remDirPath="<<remoteDirPath<<std::endl;
        mkdir_p(localDirPath.c_str(), S_IRWXU|S_IRWXG|S_IRWXO);
        mkdir_p(remoteDirPath.c_str(), S_IRWXU|S_IRWXG|S_IRWXO);

        m_currentFileSize.set_value(0);
        file = new TFile(filePathLocal.c_str(),"UPDATE","DATA",1);// SetCompressionLevel(1)
        if ((!file) || file->IsZombie()){
            //walltime = ::time(0);
            //fprintf(stderr,"%s : Error opening ROOT file  %s\n",eq_fct_->ConfigName,asctime(localtime(&(walltime))));
            fprintf(stderr,"!!!! going to exit. ln:%d\n",__LINE__);
            exit(-1);
        }
        file->cd(); gFile = file;
        DEBUG_APP_INFO(2," ");

        m_mutexForEntries.readLock();
        nEntries = 0;
        for(pCurEntry=m_pEntryFirst; pCurEntry; pCurEntry=pCurEntry->next){
            //DEBUG_("pCurEntry->type=%d, daq_name=%s, file=%p\n",pCurEntry->type,pCurEntry->daq_name,file);
            file->cd(); gFile = file;
            pTree2 = new TTree(pCurEntry->daqName(), "DATA");
            HANDLE_MEM_DIFF(pTree2," ");
            pCurEntry->SetRootTree3(pTree2,pCurEntry->daqName());
            //pCurEntry->tree(&bTimeToSave)->Branch(pCurEntry->daqName(),pCurEntry->bufferForRoot(),pCurEntry->rootFormatString());
            //pCurEntry->isPresent = false; // done in SetTree
            ++nEntries;

            //TBranch* pBranch;
            //pBranch->Set
        }
        m_mutexForEntries.unlock();

        DEBUG_APP_INFO(1,"nEntries=%d",nEntries);
        nContinueFill = 1;

        while( m_nWork && nContinueFill  )
        {
            nSemaReturn = m_semaForRootThread.wait(SEMA_WAIT_TIME_MS);
            if(nSemaReturn){
                nError = errno;
                set_error(nError,strerror_r(nError,vcError,127));
                continue;
            }
            else{
                ftime(&currentTime);
                if((currentTime.time-nPreviousTime)>=(SEMA_WAIT_TIME_MS/1000)){
                    nGenEvent = m_genEvent.value();
                    if(nGenEvent==nPreviousGenEvent){
                        nError = 2018;
                        set_error(nError,"Gen event error");
                    }
                    else{
                        if(nError){
                            nError = 0;
                            set_error(nError,"ok");
                        }
                        nPreviousGenEvent = nGenEvent;
                    }

                    nPreviousTime = currentTime.time;
                }
            }
            m_mutexForEntries.readLock();
            while(m_fifoToFill.Extract(&pMemToProc)>0){
                pCurEntry = (SingleEntry*)pMemToProc->Parent();
                //pCurEntry->copyForRoot(pMemToProc);   // --> possible error place
                nSeconds = pMemToProc->time();nEventNumber=pMemToProc->eventNumber();
                //DEBUG_("set to stack (num=%d)",++g_nInTheStack);
                //pCurEntry->stack.SetToStack(pMemToProc); // we have all necessary info, we can free mem  -->   1.

                file->cd(); gFile = file;

                pTree2 = pCurEntry->tree2();
                if(!pTree2){ // new added entry

                    pTree2 = new TTree(pCurEntry->daqName(), "DATA");
                    HANDLE_MEM_DIFF(pTree2," ");
                    pCurEntry->SetRootTree3(pTree2,pCurEntry->daqName());
                    //HANDLE_MEM_DIFF(pBranch," ");
                    //pTree->Branch(pCurEntry->daqName(),pCurEntry->bufferForRoot(),pCurEntry->rootFormatString());
                }
                //pBranch->ResetAddress();
                //pBranch->SetAddress(pMemToProc->bufferForRoot());
                //pBranch->SetBufferAddress(
                pCurEntry->SetBranchAddress(&bTimeToSave,pMemToProc);
                pCurEntry->AddExistanceInRootFile(nSeconds,nEventNumber);
                pTree2->Fill();                                                                                 // --> 2.
                pCurEntry->stack.SetToStack(pMemToProc); // we have all necessary info, we can free mem  -->   1.

                if(bTimeToSave){
                    pTree2->AutoSave("SaveSelf");
                    nFileSize = file->GetSize();
                    //printf("!!!!!!!!!!!!!!!!!!!!!!!!!!! SaveSelf, fileSize=%d\n",nFileSize);
                    m_currentFileSize.set_value(nFileSize);
                    if( nFileSize >= m_rootLength.value() ) { nContinueFill = 0; }
                }
            } // while(m_fifoToFill.Extract(&pMemToProc)){
            m_mutexForEntries.unlock();
        }// while( (Close_File_R == 0) && (eq_fct_->ThreadStatus_.value() == 1) )

        m_mutexForEntries.readLock();
        for(pCurEntry=m_pEntryFirst; pCurEntry; pCurEntry=pCurEntry->next){
            pTree2 = pCurEntry->tree2();
            if(!pTree2){continue;}
            pTree2->AutoSave("SaveSelf");
        }
        m_mutexForEntries.unlock();


        file->cd();
        gFile = file;
        file->TDirectory::DeleteAll();
        file->TDirectory::Close();
        delete file;
        file = 0;

        CopyFileToRemoteAndMakeIndexing(filePathLocal,filePathRemote);

    } // while( m_nWork && (m_threadStatus.value() == 1) )

}



void pitz::daq::EqFctCollector::CopyFileToRemoteAndMakeIndexing(const std::string& a_fileLocal, const std::string& a_fileRemote)
{
    SingleEntry* pCurEntry;
    fstream index_fl;
    char vcBuffer[1024];

    snprintf(vcBuffer,1023,"dccp -d 0 %s %s", a_fileLocal.c_str(), a_fileRemote.c_str());
    if(system(vcBuffer)!=0){
        if(m_unErrorUnableToWriteToDcacheNum++==0){
            // send an email
            SendEmailCpp(m_rootFileNameBase.value(),m_expertsMailingList.value(),"Unable to copy the file", "Unable to copy the file");
        }
        if(m_unErrorUnableToWriteToDcacheNum == NUMER_OF_WAITING_FOR_ERR_REP){m_unErrorUnableToWriteToDcacheNum=0;}
        return;
    }

    m_mutexForEntries.readLock();

    for(pCurEntry= m_pEntryFirst;pCurEntry;pCurEntry=pCurEntry->next){
        if(pCurEntry->isPresent()){
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
    } // for(pCurEntry= m_pEntryFrst;pCurEntry;pCurEntry=pCurEntry->next){

    m_mutexForEntries.unlock();

    m_fifoForLocalFileDeleter.AddElement1(a_fileLocal);
    m_semaForLocalFileDeleter.post();
}


pitz::daq::SingleEntry* pitz::daq::EqFctCollector::RemoveOneEntry(SingleEntry* a_pEntry)
{
    SingleEntry* pCurEntry;
    SNetworkStruct* pNetworkParent;
    int nNumOfEntries;

    if(!a_pEntry){return NULL;}
    pNetworkParent = a_pEntry->networkParent();
    if(!pNetworkParent){fprintf(stderr,"!!!!!!!!!! Entry without parent!\n"); return NULL;}

    //m_mutexForEntries.writeLock();
    pCurEntry = a_pEntry->next;
    if(a_pEntry==m_pEntryFirst){m_pEntryFirst = a_pEntry->next;}
    pNetworkParent->RemoveEntry(a_pEntry,&nNumOfEntries);
    if(nNumOfEntries<=0){
        //RemoveNetworkStructure(pNetworkParent);
        if(pNetworkParent==m_networkFirst){m_networkFirst=m_networkFirst->next();}
        if(pNetworkParent==m_networkLast){m_networkLast=m_networkLast->prev();}
        delete pNetworkParent;
    }
    //m_mutexForEntries.unlock();
    m_numberOfEntries.set_value(--m_nNumberOfEntries);
    return pCurEntry;
}


bool pitz::daq::EqFctCollector::AddJobForRootThread(pitz::daq::MemoryBase* a_pData)
{
    if(m_fifoToFill.AddElement(a_pData)){
        m_semaForRootThread.post();
        return true;
    }
    else{
        DEBUG_APP_INFO(0,"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! No place in the fifo!\n");
    }

    ((SingleEntry*)a_pData->Parent())->stack.SetToStack(a_pData);
    return false;
}
