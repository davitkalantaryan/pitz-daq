/*
 *	File      : pitz_poluxclient_application.cpp
 *
 *	Created on: 22 Mar, 2017
 *	Author    : Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *
 */

//#define DEBUG_COL(_logLevel,...) do{printf("ln:%d -> ",__LINE__);printf(__VA_ARGS__);}while(0)
#define DEBUG_COL(_logLevel,...)

#define     ANY_COMMAND_PROP_NAME  "ANY.COMMAND"
#define     SLEEP_TIME_MS           1000
#define     COLLECTORS_DEVICE       "PITZ.DAQ/COLLECTOR"
#define     WATCHDOG_FACILITY       "WATCHDOG"

#include "pitz_daq_daqinterface_application.hpp"
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <common/newlockguards.hpp>

#ifdef _WIN32
#include <Windows.h>
#define SleepIntMs(_x) SleepEx((_x),TRUE)
#else
#include <unistd.h>
//#define SleepIntMs(_x) do{sleep((_x)/1000000);usleep((_x)%1000000);}while(0)
#define SleepIntMs(_x) do{sleep((_x)/1000);usleep(((_x)%1000)*1000);}while(0)
#endif

int g_nDebugApplication = 1;
typedef void* TypeVoidPointer__;


/* ////////////////////////////////////////////////////////////////////////////// */
pitz::daq::daqinterface::Application::Application(int& a_argc, char* a_argv[])
    :
      QApplication(a_argc,a_argv)
{
    connect(this,SIGNAL(TaskDoneSig1(pitz::daq::daqinterface::STaskDoneStruct)),
            this,SLOT(TaskDoneSlot1(pitz::daq::daqinterface::STaskDoneStruct)));
    m_nWork = 1;
    m_tasksThread = ::STDN::thread(&pitz::daq::daqinterface::Application::TasksThreadFunction,this);
    m_deviceRegularThread = ::STDN::thread(&pitz::daq::daqinterface::Application::DeviceRegularThreadFunction,this);
}


pitz::daq::daqinterface::Application::~Application()
{
    disconnect(this,SIGNAL(TaskDoneSig1(pitz::daq::daqinterface::STaskDoneStruct)),
               this,SLOT(TaskDoneSlot1(pitz::daq::daqinterface::STaskDoneStruct)));
    m_nWork = 0;
#ifdef _WIN32
    QueueUserAPC([](ULONG_PTR)->void{},m_deviceRegularThread.native_handle(),NULL);
#else
    pthread_kill(m_deviceRegularThread.native_handle(),SIGUSR1);
#endif
    m_semaphore.post();
    m_deviceRegularThread.join();
    m_tasksThread.join();
}


void pitz::daq::daqinterface::Application::TasksThreadFunction()
{
    EqData  eqDataIn, eqDataOut;
    EqCall  eqCall;
    EqAdr   doocsAdr;

    int64_t llnReturn;
    STaskStruct aTaskStruct;
    STaskDoneStruct aTaskDoneStruct1;

    int nType;

    while(m_nWork==1){
        m_semaphore.wait();

        while(m_taskFifo.Extract(&aTaskStruct)){
            // Do the job

            switch(aTaskStruct.args.type)
            {
#if 0
            case TaskType::AnyCommand:
                eqDataIn.init();
                eqDataOut.init();
                doocsAdr.adr(propertyName.c_str());

                eqDataIn.set_type(DATA_STRING);
                eqDataIn.set(aTaskStruct.args.input.c_str());
                llnReturn = eqCall.set(&doocsAdr,&eqDataIn,&eqDataOut);
                //printf("!!!!!!!!!!!!!line:%d\n",__LINE__);
                break;
            case TaskType::CheckDoocsAdr:
                propertyName = m_strFacility + "/" + m_strDevice + "/" + m_strLocation +
                        "/" ANY_COMMAND_PROP_NAME ;
                eqDataIn.init();
                eqDataOut.init();
                doocsAdr.adr(propertyName.c_str());

                llnReturn = eqCall.get(&doocsAdr,&eqDataIn,&eqDataOut);
                nType = eqDataOut.type();
                break;
#endif
            case TaskType::setToDoocsAddressInt:
                nType = DATA_INT;
                eqDataIn.init();
                eqDataOut.init();
                doocsAdr.adr(aTaskStruct.args.input);

                eqDataIn.set_type(nType);
                eqDataIn.set((int)(size_t)aTaskStruct.args.clb_data);
                llnReturn = eqCall.set(&doocsAdr,&eqDataIn,&eqDataOut);
                break;
            default:
                llnReturn = -1;
                break;
            }

            __DEBUG__APP__(0,"prop=%s, str=%s, ret=%d, %d ~ %d",
                           aTaskStruct.args.input.c_str(),
                           aTaskStruct.args.input.c_str(),
                           (int)llnReturn,nType,DATA_STRING);
            aTaskDoneStruct1.task = aTaskStruct;
            aTaskDoneStruct1.err = llnReturn;
            emit TaskDoneSig1(aTaskDoneStruct1);

        } // while(m_taskFifo.GetFirstData(&aTaskStruct))

    } // while(m_nWork==1)
}


void pitz::daq::daqinterface::Application::DeviceRegularThreadFunction()
{
    int nIteration ( 0 );
#ifdef _WIN32
#else
    struct sigaction aAction;
    sigemptyset(&aAction.sa_mask);
    aAction.sa_flags = 0;
    aAction.sa_restorer =  nullptr;
    aAction.sa_handler = [](int)->void{};
    sigaction(SIGUSR1,&aAction,NULL);
#endif

    while(m_nWork){
        RegularFunction(nIteration++);
        SleepIntMs(SLEEP_TIME_MS);
        //printf("!!!! iter=%d\n",nIteration);
    }
}


void pitz::daq::daqinterface::Application::RegularFunction(int a_nIteration)
{
    typedef void (Application::*FncType)(SCollemtorItem*& a_pItem);

    //SCollemtorItem *pItem, *pItemNext;
    SCollemtorItem *pItem;
    USTR* pUSTR;
    EqData dataOut;
    EqAdr  dcsAddr;
    EqCall eqCall;
    ::common::NewLockGuard< ::STDN::shared_mutex > aGuard;
    int i,nResult, nLength;
    bool bChangeFound(false);
    auto fncClbkGlb = ::std::bind(&Application::RemoveItemIfNeeded,::std::placeholders::_1,::std::placeholders::_2,a_nIteration);
    FncType fncClbk = *((FncType*)((void*)&fncClbkGlb));

    dataOut.init();

    dcsAddr.adr(COLLECTORS_DEVICE "/*");

    nResult = eqCall.names(&dcsAddr,&dataOut);
    if(nResult){return;}

    nLength = dataOut.length();

    aGuard.SetAndLockMutex(&m_mutexForEntries);

    for(i=0;i<nLength;++i){
        pUSTR = dataOut.get_ustr(i);
        if(!pUSTR){continue;}
        if(strstr(pUSTR->str_data.str_data_val,"SVR.")){continue;}
        if(m_listAndHash.FindEntry(pUSTR->str_data.str_data_val,pUSTR->str_data.str_data_len,&pItem)){
            pItem->iteration = a_nIteration;
            //if((!pItem->slotObject)&&(a_nIteration>5)){emit NewCollectorFoundSignal(pItem,m_listAndHash.index(pItem->pHashItem));}
        }
        else{
            pItem = new SCollemtorItem(a_nIteration,pUSTR->str_data.str_data_val);
            pItem->pHashItem = m_listAndHash.AddData(pItem,pItem->hostName2, pUSTR->str_data.str_data_val,pUSTR->str_data.str_data_len);
            if(a_nIteration>0){emit NewCollectorFoundSignal(pItem,m_listAndHash.index(pItem->pHashItem));}
        }
        bChangeFound = (bChangeFound || pItem->Handle() );

    }

    m_listAndHash.IterateOverEntriesWithPossibleRemove(this,fncClbk);

    aGuard.UnsetAndUnlockMutex();

    if((a_nIteration==0)||bChangeFound){emit InitializationDoneSignal();}
}


void pitz::daq::daqinterface::Application::RemoveItemIfNeeded(SCollemtorItem*& a_pItem, int a_nIteration)
{
    if(a_pItem->iteration == a_nIteration){ emit RemoveSingleCollectorSignal(a_pItem); }
}





//#define CallingArguments1  void* a_clb_data,const std::string& a_input
// struct task_struct{TypeCallback1 fpClbk;void* owner;void* clb_data;std::string input;};

void pitz::daq::daqinterface::Application::SetNewTask(TypeCallback1 a_fpClb,void* a_owner,SCallArgsAll a_args)
{
    STaskStruct aTaskStruct;

    aTaskStruct.fpClbk = a_fpClb;
    aTaskStruct.owner = a_owner;
    aTaskStruct.args = a_args;

    m_taskFifo.AddElement(aTaskStruct);
    m_semaphore.post();
}

//#define CallingArguments1  void* a_clb_data,const std::string& a_input
//#define CallbackArguments1  CallingArguments1, int64_t a_err

void pitz::daq::daqinterface::Application::TaskDoneSlot1(pitz::daq::daqinterface::STaskDoneStruct a_td)
{
    if(a_td.task.fpClbk){(*a_td.task.fpClbk)(a_td.task.owner,a_td.task.args,a_td.err);}
}


/**********************************************************************************************/
pitz::daq::daqinterface::SCollemtorItem::SCollemtorItem(int a_iteration,const std::string& a_serverNm)
    :
    slotObject(0),ownerData(0),iteration(a_iteration)
{
    EqData dataOut, dataIn;
    EqAdr  eqAdr;
    EqCall eqCall;
    size_t i, unStrLen (a_serverNm.length());
    ::std::string watchdogAddition (unStrLen>4 ? a_serverNm.substr(4) : a_serverNm);
    ::std::string callDcsAddress = COLLECTORS_DEVICE "/" + a_serverNm + "/*";
    ::std::string aHostName;


    eqAdr.adr(callDcsAddress);
    dataOut.init();dataIn.init();
    eqCall.names(&eqAdr,&dataOut);

    eqCall.get_option(&eqAdr,&dataIn,&dataOut,EQ_HOSTNAME);
    this->hostName2 = dataOut.get_string();

    unStrLen = this->hostName2.length();
    for(i=0;(i<unStrLen)&&(this->hostName2[i]!='.');++i){
        aHostName.push_back(toupper(this->hostName2[i]));
    }
    //this->hostName.push_back(0);

    dataOut.init();
    eqCall.get_option(&eqAdr,&dataIn,&dataOut,EQ_LIBNO);
    this->rpcNumber = dataOut.get_int();

    this->locationBase = COLLECTORS_DEVICE "/" + a_serverNm + "/";
    this->watchdogAddressBase = WATCHDOG_FACILITY "/" + aHostName + "/SVR." + watchdogAddition + "/";

#if 0
    ::std::cout
          <<"server:"<<a_serverNm
          <<", host:"<<this->hostName
          <<", watchdog:"<<this->watchdogAddressBase
          <<", locationBase:"<<this->locationBase
          <<std::endl;
#endif
}


pitz::daq::daqinterface::SCollemtorItem::~SCollemtorItem()
{
    //
}


bool pitz::daq::daqinterface::SCollemtorItem::Handle()
{
    SCollemtorItem* a_pItem = this;
    EqData dataOut, dataIn;
    EqAdr dcsAdr;
    EqCall eqCall;
    int nResult;
    int nFromDoocs;
    std::string propName2;
    ::std::string strFromDoocs;
    ::std::string propName = a_pItem->locationBase + "ERROR";

    dataOut.init();dataIn.init();
    dcsAdr.adr(propName);
    nResult = eqCall.get(&dcsAdr,&dataIn,&dataOut);

    DEBUG_COL(2,"nResult=%d\n",nResult);
    if(!nResult){
        if((!a_pItem->isRunning)&&a_pItem->slotObject){a_pItem->isRunning=1;emit ChangedSignal((int)collectorChangeCode::startStop);}
        else if(!a_pItem->slotObject){a_pItem->isRunning = 1;}
        nFromDoocs=dataOut.get_int();
    }
    else{
        if(a_pItem->isRunning&&a_pItem->slotObject){a_pItem->isRunning=0;emit ChangedSignal((int)collectorChangeCode::startStop);}
        else if(!a_pItem->slotObject){a_pItem->isRunning = 0;}
        goto watchdgPart;
    }

    if((nFromDoocs != a_pItem->error) && a_pItem->slotObject){
        a_pItem->error = nFromDoocs;
        emit ChangedSignal((int)collectorChangeCode::error);
    }
    else if(!a_pItem->slotObject){a_pItem->error = nFromDoocs;}


    /****************************************************************/
    // error.str
    propName = a_pItem->locationBase + "ERROR.STR";

    dataOut.init();dataIn.init();
    dcsAdr.adr(propName);
    nResult = eqCall.get(&dcsAdr,&dataIn,&dataOut);
    DEBUG_COL(2,"nResult=%d\n",nResult);
    if(nResult){
        if(a_pItem->slotObject){a_pItem->isRunning = 0;emit ChangedSignal((int)collectorChangeCode::startStop);}
        else{a_pItem->isRunning = 0;}
        goto watchdgPart;
    }
    strFromDoocs=dataOut.get_string();

    if((strFromDoocs != a_pItem->errorStr) && a_pItem->slotObject){
        a_pItem->errorStr = strFromDoocs;
        emit ChangedSignal((int)collectorChangeCode::errorStr);
    }
    else if(!a_pItem->slotObject){a_pItem->errorStr = strFromDoocs;}


    /****************************************************************/
    // GEN_EVENT
    propName = a_pItem->locationBase + "GEN_EVENT";

    dataOut.init();dataIn.init();
    dcsAdr.adr(propName);
    nResult = eqCall.get(&dcsAdr,&dataIn,&dataOut);
    DEBUG_COL(2,"nResult=%d\n",nResult);
    if(nResult){
        if(a_pItem->slotObject){a_pItem->isRunning = 0;emit ChangedSignal((int)collectorChangeCode::startStop);}
        else{a_pItem->isRunning = 0;}
        goto watchdgPart;
    }
    nFromDoocs=dataOut.get_int();

    if((nFromDoocs != a_pItem->genEvent) && a_pItem->slotObject){
        a_pItem->genEvent = nFromDoocs;
        emit ChangedSignal((int)collectorChangeCode::genEvent);
    }
    else if(!a_pItem->slotObject){a_pItem->genEvent = nFromDoocs;}


watchdgPart:
    propName2 = a_pItem->watchdogAddressBase + "STS.ONLINE";

    dataOut.init();dataIn.init();
    dcsAdr.adr(propName2);
    nResult = eqCall.get(&dcsAdr,&dataIn,&dataOut);
    DEBUG_COL(2,"a_pItem->watchdogAddressBase:%s, watchdog:%s, nResult=%d\n",a_pItem->watchdogAddressBase.c_str(),propName2.c_str(),nResult);
    if(!nResult){
        if((!a_pItem->isWatchdogOk)&&a_pItem->slotObject){a_pItem->isWatchdogOk=1;emit ChangedSignal((int)collectorChangeCode::watchdogGoodBad);}
        else if(!a_pItem->slotObject){a_pItem->isWatchdogOk = 1;}
        nFromDoocs=dataOut.get_int();
    }
    else{
        if(a_pItem->isWatchdogOk&&a_pItem->slotObject){a_pItem->isWatchdogOk=0;emit ChangedSignal((int)collectorChangeCode::watchdogGoodBad);}
        else if(!a_pItem->slotObject){a_pItem->isWatchdogOk = 0;}
        return false;
    }

    if((nFromDoocs != a_pItem->isOnline) && a_pItem->slotObject){
        a_pItem->isOnline = nFromDoocs;
        emit ChangedSignal((int)collectorChangeCode::onlineOffline);
    }
    else if(!a_pItem->slotObject){a_pItem->isOnline = nFromDoocs;}


    return false;
}
