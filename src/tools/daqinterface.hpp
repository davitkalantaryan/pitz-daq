#ifndef DAQINTERFACE_HPP
#define DAQINTERFACE_HPP

#include <QWidget>
#include <QMainWindow>
#include <QScrollArea>
#include <QProcess>
//#include <QColorGroup>
#include <QPalette> 
#include <unistd.h>
#include <pthread.h>
#include <eq_client.h>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QGridLayout>

class	EqCall ;
class	EqAdr  ;
class	EqData ;

class QLineEdit;
class QComboBox;
class QGridLayout;
class QPushButton;
class QString;
class QPalette;
class QColorGroup;

class MainWindow;

class subWindow : public QWidget
{
	Q_OBJECT
public:
	subWindow(QString, QString, QString, QString, QString);	
	
public:
	QLineEdit*     servername;
	QLineEdit*     hostname;
	QLineEdit*     comments;
	QLineEdit*     genevent;
	
	QPushButton*   start;	
	QPushButton*   stop;
    QPushButton*   set_online;
	QComboBox*     comboBox;
	QComboBox*     AcomboBox;
	
	
	char 		statusaddress[80];
	char 		condaddress[80];	
	char 		timeaddress[80];
	
	char     watchdogSTSaddress[80];	
	char     on_off_lineaddress[80];		
	char     startaddress[80];	
	char     stopaddress[80];
	char     alarmaddress[80];	
	char     writeaddress[80];				
		
	int		THREADSTATUS_;
	int      WATCHDOG_;	
	int		CONDITION_S;
	int      ALARM_;		
	
	int      startButton;
	struct timeval tv1; 	
	long           tv1_s;	
	

	QString   Facility;
	QString   Device;
	QString   Location;		
	QString   Hostname;
	QString   RPCNUMBER;	
	
	QStringList	logfile;		
	QStringList	conditionfile;		
	QStringList	configfile;
    QStringList	m_writefile;
		
		
   QProcess*	proc_logfile;
   QProcess*	proc_conditionfile;	
   QProcess*	proc_configfile;	
   QProcess*	proc_writefile;	
	
	QString     command;
		
    QString   string0;
	QString   string1;
	
	QString   string2;
	QString   string3;		
	
	int       station;

	EqAdr  *ea_S;
	EqData *ed_S;
	EqData *result_S;	
	EqCall *eq_S;			
	
public slots:
	void  	 whichfile(int);
	void  	 Awhichfile(int);	
	void  	 stopserver();
	void  	 startserver();
    void SetServerOnlineSlot();
};

class subALL : public QWidget
{
	Q_OBJECT

public:
	subALL(MainWindow* pointer);
	
	subWindow  *sWindow[64];
	MainWindow* p;
	
    //char    host_prop[160];
    //FILE*   host_prop_ptr;
	char*   pn;	
	char    data[160];	
        //char    host_prop_name[80];
	
	
	char   Facility[80];
	char   Device[80];
	char   Location[80];		
	char   Hostname[80];	
	char   RPCNUMBER[80];		
	
	int    i,j;		
	
	QGridLayout *layout;	
};

class MainWindow : public QMainWindow
{
	Q_OBJECT
	
public:
	MainWindow();

private:
	QWidget *centralWidget;		
        QScrollArea *scrollArea;

	QGridLayout *layout;	
	
	subALL* suball;
	
	int    i;
	
    QString	string0;
	
private slots:
    void starttimer();
private:
    void* update_thread();
    static void* update_thread(void*);


private:
signals:
    void DoUpdateSig();
			
public:	
    //QTimer*       timer;
        //QPushButton*  quit;
	
	struct timeval tv2; 	
	long           tv2_s;
	
    int    test_var;
    pthread_t m_updateThread;
    volatile int m_nWork;

    EqAdr  m_ea;
    EqData m_ed;
    EqData m_result;
    EqCall m_eq;
	
			
};

#endif // DAQINTERFACE_HPP

