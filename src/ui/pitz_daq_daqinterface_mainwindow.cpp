/*
 *	File      : pitz_poluxclient_mainwindow.cpp
 *
 *	Created on: 22 Mar, 2017
 *	Author    : Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *
 */

#include "pitz_daq_daqinterface_mainwindow.hpp"
#include "pitz_daq_daqinterface_application.hpp"

pitz::daq::daqinterface::MainWindow::MainWindow()
    :
      m_ActionQuit(QIcon(":/img/quit_pik.png"),tr("Close"),this),
      m_ActionConnect(QIcon(":/img/connect_btn.png"),tr("Connect"),this),
      m_ActionDisconnect(QIcon(":/img/disconnect_btn.png"),tr("Disconnect"),this),
      m_ActionPort(tr("Port"),this),
      m_ActionRescan(QIcon(":/img/rescan_btn.png"),tr("Rescan Port"),this)
{
    // ...
    RETUN_LAYOUT* pWholeLayoutForCentralWidget = m_CentralWidget.GetWholeLayout();

    m_MenuLayout.addWidget(&m_MenuBarLeft);
    m_MenuLayout.addWidget(&m_MenuBarRight);
    m_MenuAndToolbarLayout.addLayout(&m_MenuLayout);
    m_MenuAndToolbarLayout.addWidget(&m_mainToolBar);
    pWholeLayoutForCentralWidget->insertLayout(0,&m_MenuAndToolbarLayout);
    pWholeLayoutForCentralWidget->insertStretch(1);

    m_MenuLayout.setAlignment(&m_MenuBarLeft, Qt::AlignLeft);
    m_MenuLayout.setAlignment(&m_MenuBarRight, Qt::AlignRight);
    m_MenuAndToolbarLayout.setSpacing(0);

    CreateActions();
    CreateMenus();
    setCentralWidget(&m_CentralWidget);
}


pitz::daq::daqinterface::MainWindow::~MainWindow()
{
    RETUN_LAYOUT* pWholeLayoutForCentralWidget = m_CentralWidget.GetWholeLayout();

    pWholeLayoutForCentralWidget->removeItem(&m_MenuAndToolbarLayout);
    m_MenuAndToolbarLayout.removeWidget(&m_mainToolBar);
    m_MenuAndToolbarLayout.removeItem(&m_MenuLayout);
    m_MenuLayout.removeWidget(&m_MenuBarRight);
    m_MenuLayout.removeWidget(&m_MenuBarLeft);
}


void pitz::daq::daqinterface::MainWindow::CreateActions()
{
    //m_pActionLoadIniFile = new QAction( tr("&Load ini"), this );
    //m_pActionLoadIniFile->setIcon( QIcon(":/images/open.png") );
    //m_pActionLoadIniFile->setShortcut( QKeySequence::Open );
    //m_pActionLoadIniFile->setStatusTip( tr("Load ini file") );
    //connect( m_pActionLoadIniFile, SIGNAL(triggered()), this, SLOT(LoadIniFileSlot()) );

    /**************************************************************************/

    m_ActionDisconnect.setDisabled(true);
    m_ActionRescan.setDisabled(true);
    m_ActionPort.setDisabled(true);

    m_ActionQuit.setStatusTip( tr("Exit Program") );
    connect( &m_ActionQuit, SIGNAL(triggered()), this, SLOT(close()) );

    m_ActionConnect.setStatusTip( tr("Connect to the DOOCS server") );
    connect( &m_ActionConnect, SIGNAL(triggered()), this, SLOT(ConnectSlot()) );

    m_ActionDisconnect.setStatusTip( tr("Disconnect from the DOOCS server") );
    connect( &m_ActionDisconnect, SIGNAL(triggered()), this, SLOT(DisconnectSlot()) );

}


void pitz::daq::daqinterface::MainWindow::CreateMenus()
{

    QMenu*    pCurMenu;
    QMenuBar* pMenuBar = &m_MenuBarLeft;

    pCurMenu = pMenuBar->addMenu( tr("&File") );
    pCurMenu->addAction( &m_ActionQuit );

    pCurMenu = pMenuBar->addMenu( tr("Communication") );
    pCurMenu->addAction( &m_ActionPort );
    pCurMenu->addSeparator( );
    pCurMenu->addAction( &m_ActionConnect );
    pCurMenu->addAction( &m_ActionDisconnect );
    pCurMenu->addAction(&m_ActionRescan);

    pCurMenu = pMenuBar->addMenu( tr("&SMC") );

    pCurMenu = pMenuBar->addMenu( tr("Spe&cial") );

    pCurMenu = pMenuBar->addMenu( tr("Config") );

    pCurMenu = pMenuBar->addMenu( tr("Program") );


    pMenuBar = &m_MenuBarRight;

    pCurMenu = pMenuBar->addMenu( tr("Help") );

    m_mainToolBar.addAction(&m_ActionConnect);
    m_mainToolBar.addAction(&m_ActionDisconnect);
    m_mainToolBar.addAction(&m_ActionRescan);
    m_mainToolBar.addAction(&m_ActionQuit);

}


void pitz::daq::daqinterface::MainWindow::ConnectSlot()
{
#if 0
    printf("!!!line:%d, fnc:%s\n",__LINE__,__FUNCTION__);

    DialogRet dlgRet = m_detailsDlg.ExecNew();

    if(dlgRet != DialogRet::OK){return;}

    m_ActionDisconnect.setEnabled(true);
    m_ActionRescan.setEnabled(true);
    m_ActionPort.setEnabled(true);

    m_ActionConnect.setDisabled(true);

#endif

}


void pitz::daq::daqinterface::MainWindow::DisconnectSlot()
{
    printf("!!!line:%d, fnc:%s\n",__LINE__,__FUNCTION__);

    m_ActionDisconnect.setDisabled(true);
    m_ActionRescan.setDisabled(true);
    m_ActionPort.setDisabled(true);

    m_ActionConnect.setEnabled(true);


}
