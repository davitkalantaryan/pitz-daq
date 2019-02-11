/*
 *	File      : pitz_poluxclient_centralwidget.cpp
 *
 *	Created on: 22 Mar, 2017
 *	Author    : Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *
 */

#include "pitz_daq_daqinterface_centralwidget.hpp"
#include <functional>

pitz::daq::daqinterface::CentralWidget::CentralWidget()
{

    m_MainLayout.setSpacing(0);
    m_MainLayout.setMargin(0);
    //CallInCurApp(IterateOverEntries,this,InitialListIterator);
    connect(CurrentApp(),SIGNAL(NewCollectorFoundSignal(SCollemtorItem*,ptrdiff_t)),this,SLOT(InitializeSingleCollectorSlot(SCollemtorItem*,ptrdiff_t)));
    connect(CurrentApp(),SIGNAL(InitializationDoneSignal()),this,SLOT(InitializeCollectorsSlot()));
    InitializeCollectorsSlot();
    setLayout(&m_MainLayout);

    //connect(&m_executeCommandBtn,SIGNAL(clicked()),this,SLOT(ExecuteCommandSlot()));
}


pitz::daq::daqinterface::CentralWidget::~CentralWidget()
{
    //m_MainLayout.removeItem(&m_MainLayoutDownPart);
    //m_MainLayoutDownPart.removeWidget(&m_anyCommandToExecute);
}


void pitz::daq::daqinterface::CentralWidget::InitializeSingleCollectorSlot(SCollemtorItem*& a_pItem,ptrdiff_t a_index)
{
    a_pItem->slotObject = new Collector(a_pItem);
    if(a_index<0){m_MainLayout.addWidget((Collector*)a_pItem->slotObject);}
    else{m_MainLayout.insertWidget(a_index,(Collector*)a_pItem->slotObject);}
    m_bInited=true;
}


void pitz::daq::daqinterface::CentralWidget::RemoveSingleCollectorSlot(SCollemtorItem*& a_pItem)
{
    m_MainLayout.removeWidget((Collector*)a_pItem->slotObject);
    delete a_pItem->slotObject;
    delete a_pItem;
}


RETUN_LAYOUT* pitz::daq::daqinterface::CentralWidget::GetWholeLayout()
{
    return &m_MainLayout;
}


void pitz::daq::daqinterface::CentralWidget::InitializeCollectorsSlot()
{
    typedef void (CentralWidget::*FncType)(SCollemtorItem*& a_pItem);
    auto fncClbkGlb = ::std::bind(&CentralWidget::InitializeSingleCollectorSlot,::std::placeholders::_1,::std::placeholders::_2,-1);
    FncType fncClbk = *((FncType*)((void*)&fncClbkGlb));
    if(!m_bInited){CurrentApp()->IterateOverEntries(this,fncClbk );}
}


void pitz::daq::daqinterface::CentralWidget::ExecuteCommandSlot()
{
    //std::string ssCommandToExecute = m_anyCommandToExecute.text().toStdString();
    //SCallArgsAll aSCallArgsAll(TaskType::AnyCommand,ssCommandToExecute);

    //SetNewTaskGlbMacro(&CentralWidget::TaskCallbackGUI,this,aSCallArgsAll);
}


void pitz::daq::daqinterface::CentralWidget::TaskCallbackGUI(SCallArgsAll /*a_args*/,int64_t a_err)
{
    if(a_err == 0)
    {
        return;
    }
}
