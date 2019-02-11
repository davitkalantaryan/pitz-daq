//
// file:        pitz_daq_daqinterface_collector.cpp
// created on:  2018 Oct 25
//

#include "pitz_daq_daqinterface_collector.hpp"

#define WIDGET_HEIGHT   22



pitz::daq::daqinterface::Collector::Collector(SCollemtorItem* a_pItem)
{
    std::string hostNameShortAndLibno;
    const size_t cunHostNameLen(a_pItem->hostName2.length());
    QFont font1 = m_hostAndRpcNumber.font();

    for(size_t ind(0);(ind<cunHostNameLen)&&(a_pItem->hostName2[ind]!='.');++ind){
        hostNameShortAndLibno.push_back(a_pItem->hostName2[ind]);
    }

    m_detailsDlg.setVisible(false);

    m_pItem = a_pItem;
    m_MainLayout.setSpacing(0);
    m_MainLayout.setMargin(0);

    m_onlineOffline.setStyleSheet("color:white;");

    //m_serverName.setStyleSheet("background-color:rgb(0,0,0);" "color:white;");
    m_serverName.setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    m_serverName.setLineWidth(1);
    //
    m_serverStatusAndGenEvent.setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    m_serverStatusAndGenEvent.setLineWidth(1);
    //  
    font1.setPointSize(6);
    m_hostAndRpcNumber.setFont(font1);
    m_hostAndRpcNumber.setWordWrap(true);
    m_hostAndRpcNumber.setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    m_hostAndRpcNumber.setLineWidth(0);
    hostNameShortAndLibno += ("\n"+std::to_string(a_pItem->rpcNumber));
    //hostNameShortAndLibno
    m_hostAndRpcNumber.setText(hostNameShortAndLibno.c_str());
    //m_hostAndRpcNumber.setText("davit\nkalantaryan");
    //
    //m_startStop.setDisabled(true);
    //m_onlineOffline.setDisabled(true);
    //m_details.setDisabled(true);

    m_serverName.setText((const char*)a_pItem->pHashItem->key);
    m_details.setText("details");

    m_serverName.setFixedSize(140,WIDGET_HEIGHT);
    m_hostAndRpcNumber.setFixedSize(100,WIDGET_HEIGHT);
    m_serverStatusAndGenEvent.setFixedSize(100,WIDGET_HEIGHT);
    m_startStop.setFixedSize(140,WIDGET_HEIGHT);
    m_onlineOffline.setFixedSize(140,WIDGET_HEIGHT);
    m_details.setFixedSize(100,WIDGET_HEIGHT);

    m_MainLayout.addWidget(&m_serverName);
    m_MainLayout.addWidget(&m_hostAndRpcNumber);
    m_MainLayout.addWidget(&m_serverStatusAndGenEvent);
    m_MainLayout.addWidget(&m_startStop);
    m_MainLayout.addWidget(&m_onlineOffline);
    m_MainLayout.addWidget(&m_details);

    ChangedSlot((int)collectorChangeCode::all);

    setFixedHeight(20);

    setLayout(&m_MainLayout);

    //connect(CurrentApp(),SIGNAL(CollectorChangedSignal(SCollemtorItem*,collectorChangeCode)),this,SLOT(ChangedSlot(SCollemtorItem*,collectorChangeCode)));
    connect(a_pItem,SIGNAL(ChangedSignal(int)),this,SLOT(ChangedSlot(int)));
    connect(&m_startStop,SIGNAL(clicked()),this,SLOT(StartStopPushedSlot()));
    connect(&m_onlineOffline,SIGNAL(clicked()),this,SLOT(OnlineOfflinePushedSlot()));
    connect(&m_details,SIGNAL(clicked()),&m_detailsDlg,SLOT(exec()));
}


#if 0
    QLabel  m_serverName;
    QLabel  m_serverStatusAndGenEvent;
    QPushButton m_startStop;
    QPushButton m_onlineOffline;
    QPushButton m_details;
#endif

pitz::daq::daqinterface::Collector::~Collector()
{
    m_MainLayout.removeWidget(&m_details);
    m_MainLayout.removeWidget(&m_onlineOffline);
    m_MainLayout.removeWidget(&m_startStop);
    m_MainLayout.removeWidget(&m_serverStatusAndGenEvent);
    m_MainLayout.removeWidget(&m_serverName);
}


void pitz::daq::daqinterface::Collector::ChangedSlot(int aa_code)
{
    collectorChangeCode a_code = (collectorChangeCode)aa_code;
    SCollemtorItem* a_pItem = m_pItem;
    switch(a_code){
    case collectorChangeCode::no:
        break;
    case collectorChangeCode::all:
    case collectorChangeCode::onlineOffline:
setNormalString:
        if(a_pItem->isOnline){
            m_onlineOffline.setStyleSheet("background-color:rgb(0,255,0);" "color:blue;");
            m_onlineOffline.setText("set offline");
            //m_startStop.setStyleSheet("background-color:rgb(0,255,0);" "color:blue;");
            m_startStop.setStyleSheet("background-color:rgb(246,245,240);" "color:black;");
        }
        else{
            m_onlineOffline.setStyleSheet("background-color:yellow;" "color:blue;");
            m_onlineOffline.setText("set online");
            m_startStop.setStyleSheet("background-color:rgb(246,245,240);" "color:black;");
        }
        if(a_code==collectorChangeCode::watchdogGoodBad){goto setStartStop;}
        if(a_code != collectorChangeCode::all)break;
    case collectorChangeCode::watchdogGoodBad:
        if(a_pItem->isWatchdogOk && (a_code != collectorChangeCode::all)){
            m_onlineOffline.setDisabled(false);
            m_startStop.setDisabled(false);
            goto setNormalString;
        }
        else if(!a_pItem->isWatchdogOk){
            m_onlineOffline.setStyleSheet("background-color:rgb(255,0,0);" "color:blue;");
            m_onlineOffline.setText("watchdog unavailable");
            m_onlineOffline.setDisabled(true);
            m_startStop.setStyleSheet("background-color:rgb(255,0,0);" "color:blue;");
            m_startStop.setText("watchdog unavailable");
            m_startStop.setDisabled(true);
        }
        if(a_code != collectorChangeCode::all)break;
    case collectorChangeCode::genEvent:
        m_serverStatusAndGenEvent.setText(QString::number(a_pItem->genEvent));
        if(a_code != collectorChangeCode::all)break;
    case collectorChangeCode::error:
setErrorStatus:
        if(a_pItem->error){
            m_serverStatusAndGenEvent.setStyleSheet("background-color:rgb(255,0,0);" "color:blue;");
        }
        else{
            //m_serverStatusAndGenEvent.setStyleSheet("background-color:green;" "color:blue;");
            m_serverStatusAndGenEvent.setStyleSheet("background-color:rgb(0,255,0);" "color:blue;");
        }
        if(a_code != collectorChangeCode::all)break;
    case collectorChangeCode::errorStr:
        //m_serverStatusAndGenEvent.setText(QString::number(a_pItem->genEvent));
        if(a_code != collectorChangeCode::all)break;
    case collectorChangeCode::startStop:
setStartStop:
        if(!a_pItem->isRunning){
            m_serverStatusAndGenEvent.setStyleSheet("background-color:grey;" "color:red;");
            m_serverStatusAndGenEvent.setText("not running");
            if(a_pItem->isWatchdogOk){m_startStop.setText("start");}
        }
        else if(a_code != collectorChangeCode::all){
            if(a_pItem->isWatchdogOk){m_startStop.setText("stop");}
            goto setErrorStatus;
        }
        else{
            if(a_pItem->isWatchdogOk){m_startStop.setText("stop");}
        }
        if(a_code != collectorChangeCode::all)break;
    default:
        break;
    }
}


void pitz::daq::daqinterface::Collector::StartStopPushedSlot()
{
    SCallArgsAll aArgs;
    aArgs.type = TaskType::setToDoocsAddressInt;
    aArgs.clb_data = (void*)1;
    aArgs.input=m_pItem->watchdogAddressBase+(m_pItem->isRunning?"STOP":"START");
    CurrentApp()->SetNewTask(NULL,NULL,aArgs);
}


void pitz::daq::daqinterface::Collector::OnlineOfflinePushedSlot()
{
    SCallArgsAll aArgs;
    aArgs.type = TaskType::setToDoocsAddressInt;
    aArgs.clb_data=m_pItem->isOnline?(void*)0:(void*)1;
    aArgs.input=m_pItem->watchdogAddressBase+"SET.ONLINE";
    CurrentApp()->SetNewTask(NULL,NULL,aArgs);
}

