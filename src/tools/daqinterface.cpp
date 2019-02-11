#include <QtGui>

#include "eq_client.h"

#include "daqinterface.hpp"

#include	<ctime>
#include <cstdio>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cerrno>

#include <sys/stat.h>
#include <sys/shm.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <libgen.h>
#include <sys/time.h>
#include <sys/resource.h> 
#include <pthread.h> 
#include <QLabel>

#include <QtGlobal>
//#if QT_VERSION_MAJOR>3 // does not work, strange
#ifdef __toLatin1_SHOULD_BE_DEFINED__
//string0.toAscii()
//string0.toLatin1().data
#define toAscii toLatin1().data
#endif


using namespace std;

const char* LN = "#_QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm1234567890";
int     subNUMBER = 0;
extern int g_nDoDebug;

int     oneALL;

subWindow::subWindow(QString Fac,QString Dev, QString Loc, QString Hos, QString RPCN) 
		: Facility(Fac), Device(Dev), Location(Loc), Hostname(Hos), RPCNUMBER(RPCN) 
{

    QByteArray cbaString0;

	servername = new QLineEdit(this);	
	servername->setFixedWidth(164);
    servername->setReadOnly(true);
	servername->setText( Location );

	hostname = new QLineEdit(this);	
	hostname->setFixedWidth(184);
    hostname->setReadOnly(true);
	hostname->setText( Hostname + "  " + RPCNUMBER );	
	
	comments = new QLineEdit(this);	
        //comments->setFixedWidth(244);
        comments->setFixedWidth(180);
    comments->setReadOnly(true);
	comments->setText("Please, wait.");
    comments->setAutoFillBackground( true );
	
	genevent = new QLineEdit(this);	
	genevent->setFixedWidth(124);
    genevent->setReadOnly(true);
	genevent->setText("         ");	
	
	start = new QPushButton( "START",this);
	start->setFont( QFont( "Sans Serif", 8, QFont::Bold ) );
	
	stop = new QPushButton( "STOP",this);
	stop->setFont( QFont( "Sans Serif", 8, QFont::Bold ) );

    //set_online
    set_online = new QPushButton( "SET.ONLINE",this);
    set_online->setFont( QFont( "Sans Serif", 8, QFont::Bold ) );

        start->setEnabled(false);
        stop->setEnabled(false);
        set_online->setEnabled(false);
	
	
	comboBox = new QComboBox(this);
	comboBox->insertItem(0, "SELECT ITEM");				
	comboBox->insertItem(1, "OPEN LOG FILE");	
	comboBox->insertItem(2, "OPEN CONDITION FILE");	
	comboBox->insertItem(3, "OPEN CONFIG FILE");	
	comboBox->maximumSize();
	comboBox->setCurrentIndex(0);
	
	AcomboBox = new QComboBox(this);
	AcomboBox->insertItem(0, "ALARM");				
	AcomboBox->insertItem(1, "CLEAR ALARM");	
	AcomboBox->insertItem(2, "SHOW FILE");	
	AcomboBox->maximumSize();
	AcomboBox->setCurrentIndex(0);
    AcomboBox->setAutoFillBackground( true );
	
//==	
	if(Dev == "PITZWSADC")
	{
		start->setEnabled( false );
		stop-> setEnabled( false );
        set_online->setEnabled(false);
		comboBox->setEnabled( false );		
		AcomboBox->setEnabled( false );				
	}		
	
	
//==	
			
//	command = "/afs/ifh.de/group/pitz/doocs/develop/levonh/bin/nedit";	
	command = "/afs/ifh.de/group/pitz/doocs/bin/nedit";
	
    string0  = "/afs/ifh.de/group/pitz/doocs/data/DAQdata/log/";
	string1 = Location;
    string0 += string1.toLower().replace('_',".");
    string0 += ".log";
    logfile << string0;
	
    string0  = "/afs/ifh.de/group/pitz/doocs/data/DAQdata/conditions/";
	string1 = Location;
    string0 += string1.toLower().replace('_',".");
    string0 += ".cond";
    conditionfile << string0;
	
    string0  = "/afs/ifh.de/group/pitz/doocs/data/DAQdata/config/";
	string1 = Location;
    string0 += string1.toLower().replace('_',".");
    string0 += ".config";
    configfile << string0;
	
    string0  = Facility;
    string0 += "/";
    string0 += Device;
    string0 += "/";
    string0 += Location;
    string0 += "/";
    string0 += "THREADSTATUS_";
    cbaString0 = string0.toLatin1();
    sprintf(statusaddress,"%s",cbaString0.data());
        if(g_nDoDebug)printf("statusaddress = \"%s\"\n",statusaddress);

    string0  = Facility;
    string0 += "/";
    string0 += Device;
    string0 += "/";
    string0 += Location;
    string0 += "/";
    string0 += "CONDITION_S";
    cbaString0 = string0.toLatin1();
    //cbaString0.data()
    sprintf(condaddress,"%s",cbaString0.data());

    string0  = Facility;
    string0 += "/";
    string0 += Device;
    string0 += "/";
    string0 += Location;
    string0 += "/";
    string0 += "GEN_EVENT";
    cbaString0 = string0.toLatin1();
    //cbaString0.data()
    sprintf(timeaddress,"%s",cbaString0.data());
	
    string0  = "WATCHDOG";
    string0 += "/";
    string0 += Hostname;
    string0 += "/";
	string1 = Device;
    string0 += string1.replace("PITZ","SVR.");
    string0 += "/";
    string0 += "STS.ONLINE";
    sprintf(watchdogSTSaddress,"%s",(const char*)(string0.toAscii()));
	
    string0  = "WATCHDOG";
    string0 += "/";
    string0 += Hostname;
    string0 += "/";
	string1 = Device;
    string0 += string1.replace("PITZ","SVR.");
    string0 += "/";
    string0 += "SET.ONLINE";
        sprintf(on_off_lineaddress,"%s",(const char*)(string0.toAscii()));
        if(g_nDoDebug)printf("on_off_lineaddress = \"%s\"\n",on_off_lineaddress);
	
    string0  = "WATCHDOG";
    string0 += "/";
    string0 += Hostname;
    string0 += "/";
	string1 = Device;
    string0 += string1.replace("PITZ","SVR.");
    string0 += "/";
    string0 += "START";
    sprintf(startaddress,"%s",(const char*)(string0.toAscii()));
        if(g_nDoDebug)printf("startaddress = \"%s\"\n",startaddress);
	
    string0  = Facility;
    string0 += "/";
    string0 += Device;
    string0 += "/";
    string0 += Location;
    string0 += "/";
    string0 += "COMMAND_";
    sprintf(stopaddress,"%s",(const char*)(string0.toAscii()));
        if(g_nDoDebug)printf("stopaddress = \"%s\"\n",stopaddress);
	
    string0  = Facility;
    string0 += "/";
    string0 += Device;
    string0 += "/";
    string0 += Location;
    string0 += "/";
    string0 += "ALARM";
    sprintf(alarmaddress,"%s",(const char*)(string0.toAscii()));
	
    string0  = Facility;
    string0 += "/";
    string0 += Device;
    string0 += "/";
    string0 += Location;
    string0 += "/";
    string0 += "WRITE_FILE";
    sprintf(writeaddress,"%s",(const char*)(string0.toAscii()));
	
	
	proc_logfile       = new QProcess( this );
	proc_conditionfile = new QProcess( this );
	proc_configfile    = new QProcess( this );
	proc_writefile     = new QProcess( this );		
	
	eq_S = new EqCall;
	ea_S = new EqAdr();
	ed_S = new EqData();
	result_S = new EqData();
	
	startButton = 0;
	gettimeofday(&tv1,NULL);
	tv1_s = tv1.tv_sec;

	connect(comboBox,SIGNAL(activated(int)), this,SLOT(whichfile(int)) );
	connect(AcomboBox,SIGNAL(activated(int)),this,SLOT(Awhichfile(int)) );	
    connect(set_online    ,SIGNAL(clicked()),      this,SLOT(SetServerOnlineSlot()) );
    connect(stop    ,SIGNAL(clicked()),      this,SLOT(stopserver()) );
        connect(start   ,SIGNAL(clicked()),      this,SLOT(startserver()) );

        if(g_nDoDebug)printf("\n");
}

void subWindow::whichfile(int index)
{
	station = index;
	switch( station )
	{
		case 0:
		
		break;	
		case 1:
			proc_logfile->start(command, logfile);			
		break;
		case 2:
			proc_conditionfile->start(command, conditionfile);			
		break;
		case 3:
			proc_configfile->start(command, configfile);				
		break;
		default:
		break;		
	}
}

void subWindow::Awhichfile(int index)
{
	station = index;
	switch( station )
	{
		case 0:
		
		break;	
		case 1:
			oneALL = 0;	
			ea_S->adr((char *)(alarmaddress));
			ed_S->init();
			ed_S->set_type(DATA_INT);		
			ed_S->set((int)oneALL);
			eq_S->set(ea_S,ed_S);
			
			AcomboBox->setCurrentIndex(0);						
		break;
		case 2:
			oneALL = 1;	
			ea_S->adr((char *)(writeaddress));
			ed_S->init();
			ed_S->set_type(DATA_INT);		
			ed_S->set((int)oneALL);
			eq_S->set(ea_S,ed_S);
			
			string2  = "/doocs/data/DAQdata/statistic/";
			string3 = Location;
			string2 += string3.toLower().replace('_',".");			
            string2 += ".txt";
            m_writefile << string2;
            proc_writefile->start(command, m_writefile);
					
			AcomboBox->setCurrentIndex(0);				
		break;

		default:
		break;		
	}
}

void subWindow::stopserver()
{
	int one;
	
	one = 1;
	ea_S->adr((char *)(stopaddress));
	ed_S->init();
    result_S = eq_S->get(ea_S,ed_S);

	if(result_S->error() == 0) 
	{ 
		ea_S->adr((char *)(stopaddress));	
		ed_S->set((int)one);	
		eq_S->set(ea_S,ed_S);
	}
	
	one = 0;	
	ea_S->adr((char *)(on_off_lineaddress));
	ed_S->init();
	ed_S->set_type(DATA_BOOL);		
	ed_S->set_bool((int)one);
	eq_S->set(ea_S,ed_S);		
}


void subWindow::SetServerOnlineSlot()
{
    EqAdr ea;
    EqData dataIn, dataOut;
    EqCall eqCall;
    QString cqsWatchdogText(set_online->text());
    int oneALL (strcmp(cqsWatchdogText.toAscii(),"SET.ONLINE") ? 0 : 1);

    if(g_nDoDebug){printf("oneAll=%d\n",oneALL);}
    set_online->setEnabled(false);

    ea.adr((char *)(on_off_lineaddress));
    dataIn.init();
    dataOut.init();
    dataIn.set_type(DATA_BOOL);
    dataIn.set_bool((int)oneALL);
    eqCall.set(&ea,&dataIn,&dataOut);
}

void subWindow::startserver()
{
	int one;
	one = 1;
	
	if(startButton == 0)
	{
		startButton = 1;
		gettimeofday(&tv1,NULL);
		tv1_s = tv1.tv_sec;
		
		ea_S->adr((char *)(startaddress));
		ed_S->init();
		ed_S->set_type(DATA_BOOL);		
		ed_S->set_bool((int)one);
		eq_S->set(ea_S,ed_S);

	}
}


static const char* GetStringFromString(const char* a_string, char* a_final_string);

subALL::subALL(MainWindow* pointer) : p(pointer)
{
	layout = new QGridLayout(this);
	layout->setSpacing(1);
	layout->setMargin(1);
    const char* cpcData;

	subNUMBER = 0;
	
    //sprintf(host_prop,"%s","/doocs/data/DAQdata/bin/DAQinterface/config.h");
//	sprintf(host_prop,"%s","./config.h");			
    //host_prop_ptr = fopen(host_prop,"r");

    FILE* host_prop_ptr = fopen("//afs/ifh.de/group/pitz/doocs/data/DAQdata/bin/DAQinterface/config.h","r");

    if(!host_prop_ptr)return;

	while( fgets(data, 160, host_prop_ptr) != NULL)
	{
		pn = strpbrk(data,LN);			
		if( ( pn == 0 ) || ( pn[0] == '#' ) )		continue;
		
        //sscanf(&data[0],"%s  %s %s %s %s",Facility,Device,Location,Hostname,RPCNUMBER);
        cpcData = &data[0];
        cpcData = GetStringFromString(cpcData,Facility);
        cpcData = GetStringFromString(cpcData,Device);
        cpcData = GetStringFromString(cpcData,Location);
        cpcData = GetStringFromString(cpcData,Hostname);
        cpcData = GetStringFromString(cpcData,RPCNUMBER);
		
		sWindow[subNUMBER] = new subWindow(Facility,Device,Location,Hostname,RPCNUMBER);
		
		layout->addWidget(sWindow[subNUMBER]->servername,subNUMBER+2,0);
		layout->addWidget(sWindow[subNUMBER]->hostname,subNUMBER+2,1);
		layout->addWidget(sWindow[subNUMBER]->comments,subNUMBER+2,2);	
		layout->addWidget(sWindow[subNUMBER]->genevent,subNUMBER+2,3);	
		
		layout->addWidget(sWindow[subNUMBER]->start,subNUMBER+2,4);			
		layout->addWidget(sWindow[subNUMBER]->stop,subNUMBER+2,5);
        layout->addWidget(sWindow[subNUMBER]->set_online,subNUMBER+2,6);
        layout->addWidget(sWindow[subNUMBER]->comboBox,subNUMBER+2,7);
        layout->addWidget(sWindow[subNUMBER]->AcomboBox,subNUMBER+2,8);
					
		subNUMBER++;
	}			

	fclose(host_prop_ptr);	
}



MainWindow::MainWindow()
{
	subNUMBER = 0;

	centralWidget = new QWidget;
	setCentralWidget(centralWidget);
	
        //quit = new QPushButton( "QUIT",this);
        //quit->setFont( QFont( "Sans Serif", 8, QFont::Bold ) );
		 
        //connect( quit, SIGNAL(clicked()), qApp, SLOT(quit()) );
	
   QLabel *infoLabel = new QLabel(tr("     Server (location)           Host name (RPC_NUMBER)    "
          "COMMENT                        GENEVENT                COMMANDS        (log,config,condition)"));
	
	layout = new QGridLayout(centralWidget);
	layout->setSpacing(1);
	layout->setMargin(1);	
	
	suball = new subALL(this);

        scrollArea = new QScrollArea(centralWidget);
	scrollArea->setWidget(suball);	

        //layout->addWidget(quit,       0,9, 1, 1);
        //layout->addWidget(quit,       0,6, 1, 1);
	layout->addWidget(infoLabel,  1,0, 1,10);		
	layout->addWidget(scrollArea, 3,0,12,10);
	
	centralWidget->setLayout(layout);	
	
    //timer = new QTimer(this);
    //connect(timer, SIGNAL(timeout()), this, SLOT(starttimer()));
	
    test_var = 0;

    connect(this,SIGNAL(DoUpdateSig()),this,SLOT(starttimer()));

    m_nWork = 1;
    //timer->start(1000);
    pthread_create(&m_updateThread,NULL,update_thread,this);
	
}


void* MainWindow::update_thread(void* a_pObj)
{
    return ((MainWindow*)a_pObj)->update_thread();
}


void* MainWindow::update_thread()
{
    while(m_nWork)
    {
#ifdef WIN32
        Sleep(500);
#else
        usleep(500000);
#endif
        emit  DoUpdateSig();
    }

    return NULL;
}


void MainWindow::starttimer()
{
	if(test_var == 1) { return; }

    int nEventNum;
    //QPalette Palette;

	test_var = 1;

	for(int i=0; i < subNUMBER; i++)
	{
		if( suball->sWindow[i]->startButton == 1 )
		{
			gettimeofday(&tv2,NULL);
			tv2_s = tv2.tv_sec;
			
			if( (tv2_s - suball->sWindow[i]->tv1_s) > 35 )
			{
				suball->sWindow[i]->startButton = 0;
			}
			else
			{
                //Palette.setColor(QPalette::Base, QColor(Qt::yellow));
                // suball->sWindow[i]->comments->setPalette(Palette);
                suball->sWindow[i]->comments->setStyleSheet("color: black;""background-color:yellow;");

                                suball->sWindow[i]->comments->setText("Please, wait.");

                                suball->sWindow[i]->start->setEnabled(false);
                                suball->sWindow[i]->stop->setEnabled(false);
                                suball->sWindow[i]->set_online->setEnabled(false);
			}			
		}
		else
		{
            m_ea.adr( (char *)(suball->sWindow[i]->statusaddress));
            m_ed.init();
            m_result.init();
            if( m_eq.get(&m_ea,&m_ed,&m_result)==0)
			{
                suball->sWindow[i]->THREADSTATUS_ = m_result.get_int();
			}
			else
			{
				suball->sWindow[i]->THREADSTATUS_ = -1;
			}
			
            m_ea.adr( (char *)(suball->sWindow[i]->watchdogSTSaddress));
            m_ed.init();
            m_result.init();
            if(m_eq.get(&m_ea,&m_ed,&m_result)==0)
			{
                suball->sWindow[i]->set_online->setEnabled(true);
                suball->sWindow[i]->WATCHDOG_ = m_result.get_int();
                if(suball->sWindow[i]->WATCHDOG_ )
                {
                    //printf("!!!!!! ofline!\n");
                    suball->sWindow[i]->set_online->setStyleSheet("background-color:white;");
                    suball->sWindow[i]->set_online->setText(tr("SET.OFLINE"));
                }
                else
                {
                    //printf("!!!!!! online!\n");
                    suball->sWindow[i]->set_online->setStyleSheet("background-color:green;");
                    suball->sWindow[i]->set_online->setText(tr("SET.ONLINE"));
                }
			}
			else
			{
				suball->sWindow[i]->WATCHDOG_ = -1;
                suball->sWindow[i]->set_online->setEnabled(false);
                suball->sWindow[i]->set_online->setText(tr("WATCHDOG.ERR"));
            }

			
            if( (suball->sWindow[i]->THREADSTATUS_ == 1) && (suball->sWindow[i]->WATCHDOG_ == 0) )
			{
#if 1
                suball->sWindow[i]->comments->setStyleSheet("color: black;""background-color:white;");
                //suball->sWindow[i]->comments->setStyleSheet("QLineEdit { background: rgb(255, 255, 255); selection-background-color: rgb(233, 233, 233); }");
                //suball->sWindow[i]->comments->update();

                suball->sWindow[i]->comments->setText( tr( "Condition is O.k."));
                suball->sWindow[i]->start->setEnabled(false);
                suball->sWindow[i]->stop->setEnabled(true);
#else // if 1
				oneALL = 1;
                m_ea.adr((char *)(suball->sWindow[i]->on_off_lineaddress));
                m_ed.init();
                m_ed.set_type(DATA_BOOL);
                m_ed.set_bool((int)oneALL);
                m_eq.set(&m_ea,&m_ed);
#endif // if 1
            }
			
            m_ea.adr( (char *)(suball->sWindow[i]->condaddress));
            m_ed.init();
            m_result.init();
            if(m_eq.get(&m_ea,&m_ed,&m_result)==0)
			{
                suball->sWindow[i]->CONDITION_S = m_result.get_int();
			}
			else
			{
				suball->sWindow[i]->CONDITION_S = -1;	
			}

			if( (suball->sWindow[i]->THREADSTATUS_ == -1) || (suball->sWindow[i]->CONDITION_S == -1) )
			{
                //Palette.setColor(QPalette::Base, QColor(255,178,175));
                //suball->sWindow[i]->comments->setPalette(Palette);
                suball->sWindow[i]->comments->setStyleSheet("color: white;""background-color:red;");
				suball->sWindow[i]->comments->setText("The server is stoped !");

                                suball->sWindow[i]->start->setEnabled(true);
                                suball->sWindow[i]->stop->setEnabled(false);
			}	

			if( suball->sWindow[i]->THREADSTATUS_ == 0 )
			{
                //Palette.setColor(QPalette::Base, QColor(Qt::yellow));
                //suball->sWindow[i]->comments->setPalette(Palette);
                suball->sWindow[i]->comments->setStyleSheet("color: black;""background-color:yellow;");
				suball->sWindow[i]->comments->setText("Please, wait for a 61 seconds.");

                                suball->sWindow[i]->start->setEnabled(false);
                                suball->sWindow[i]->stop->setEnabled(false);
			}	

            if( (suball->sWindow[i]->THREADSTATUS_ == 1) && (suball->sWindow[i]->CONDITION_S == 1) && (suball->sWindow[i]->WATCHDOG_ != 0) )
			{
                //Palette.setColor(QPalette::Base, QColor(178, 255, 175));
                //suball->sWindow[i]->comments->setPalette(Palette);
                suball->sWindow[i]->comments->setStyleSheet("color: white;""background-color:green;");
				suball->sWindow[i]->comments->setText("Condition is O.k.");

                                suball->sWindow[i]->start->setEnabled(false);
                                suball->sWindow[i]->stop->setEnabled(true);
			}

			if( (suball->sWindow[i]->THREADSTATUS_ == 1) && (suball->sWindow[i]->CONDITION_S == 0) )
			{
                //Palette.setColor(QPalette::Base, QColor(Qt::yellow));
                //suball->sWindow[i]->comments->setPalette(Palette);
                suball->sWindow[i]->comments->setStyleSheet("color: black;""background-color:yellow;");
				suball->sWindow[i]->comments->setText("Bad condition");

                                suball->sWindow[i]->start->setEnabled(false);
                                suball->sWindow[i]->stop->setEnabled(true);
			}	
			
            m_ea.adr( (char *)(suball->sWindow[i]->alarmaddress));
            m_ed.init();
            m_result.init();
            if(m_eq.get(&m_ea,&m_ed,&m_result)==0)
			{
                suball->sWindow[i]->ALARM_ = m_result.get_int();
			}
			else
			{
				suball->sWindow[i]->ALARM_ = -1;
			}
			
			if(suball->sWindow[i]->ALARM_ == 1)
			{
                //suball->sWindow[i]->AcomboBox->setPalette( QPalette( QColor(255,178,175) ) );
                suball->sWindow[i]->AcomboBox->setStyleSheet("color: white;""background-color:red;");
				if( (suball->sWindow[i]->AcomboBox->count()) != 3)
				{
					suball->sWindow[i]->AcomboBox->clear();
					suball->sWindow[i]->AcomboBox->insertItem(0, "ALARM");				
                                        suball->sWindow[i]->AcomboBox->insertItem(1, "CLEAR ALARM");
					suball->sWindow[i]->AcomboBox->insertItem(2, "SHOW FILE");	
					suball->sWindow[i]->AcomboBox->setCurrentIndex(0);
				}
			}
			
			if(suball->sWindow[i]->ALARM_ == 2)
			{
                //suball->sWindow[i]->AcomboBox->setPalette( QPalette( QColor(Qt::yellow) ) );
                suball->sWindow[i]->AcomboBox->setStyleSheet("color: black;""background-color:yellow;");
				if( (suball->sWindow[i]->AcomboBox->count()) != 3)
				{
					suball->sWindow[i]->AcomboBox->clear();
					suball->sWindow[i]->AcomboBox->insertItem(0, "ALARM");				
	   			suball->sWindow[i]->AcomboBox->insertItem(1, "CLEAR ALARM");	
					suball->sWindow[i]->AcomboBox->insertItem(2, "SHOW FILE");	
					suball->sWindow[i]->AcomboBox->setCurrentIndex(0);
				}
			}			
			
			if(suball->sWindow[i]->ALARM_ == 0)
			{
                //suball->sWindow[i]->AcomboBox->setPalette( QPalette( QColor(178, 255, 175) ) );
                suball->sWindow[i]->AcomboBox->setStyleSheet("color: white;""background-color:green;");
				if( (suball->sWindow[i]->AcomboBox->count()) != 1)
				{
					suball->sWindow[i]->AcomboBox->clear();
					suball->sWindow[i]->AcomboBox->insertItem(0, "ALARM");				
					suball->sWindow[i]->AcomboBox->setCurrentIndex(0);
				}				
			}				
			if(suball->sWindow[i]->ALARM_ == -1)
			{
                //suball->sWindow[i]->AcomboBox->setPalette( QPalette( QColor(Qt::yellow) ) );
                suball->sWindow[i]->AcomboBox->setStyleSheet("color: black;""background-color:yellow;");
				if( (suball->sWindow[i]->AcomboBox->count()) != 1)
				{
					suball->sWindow[i]->AcomboBox->clear();
					suball->sWindow[i]->AcomboBox->insertItem(0, "ALARM");				
					suball->sWindow[i]->AcomboBox->setCurrentIndex(0);
				}					
			}
		}
	
        m_ea.adr( (char *)(suball->sWindow[i]->timeaddress));
        m_ed.init();
        m_result.init();
        if(m_eq.get(&m_ea,&m_ed,&m_result)==0)
		{
            nEventNum = m_result.get_int();
            string0.setNum(nEventNum);
            suball->sWindow[i]->genevent->setText( string0 );
		}
		else
		{
			suball->sWindow[i]->genevent->setText("         ");		
		}
	}
	
	test_var = 0;
	
	return;		
}



//sscanf(&data[0],"%s  %s %s %s %s",Facility,Device,Location,Hostname,RPCNUMBER);
static const char* GetStringFromString(const char* a_string, char* a_final_string)
{
    size_t unBytes;
    const char *pcStringIn = a_string,*pcString;
    if(!a_string) return NULL;
    if(!strlen(a_string)) {a_final_string = 0;return a_string;}

    pcString = strchr(pcStringIn,' ');

    while(pcString == pcStringIn)
    {
        ++pcStringIn;
        pcString = strchr(pcStringIn,' ');
        //if(!pcString){return pcString;}
    }

    if(!pcString)
    {
        strcpy(a_final_string,pcStringIn);
        return pcString;
    }

    unBytes = pcString-pcStringIn;
    memcpy(a_final_string,pcStringIn,unBytes);
    a_final_string[unBytes] = 0;
    return pcStringIn + unBytes;
}













