/*
 *	File      : pitz_poluxclient_centralwidget.hpp
 *
 *	Created on: 22 Mar, 2017
 *	Author    : Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *
 */
#ifndef PITZ_DAQ_DAQINTERFACE_CENTRALWIDGET_HPP
#define PITZ_DAQ_DAQINTERFACE_CENTRALWIDGET_HPP

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QGridLayout>
#include <QDoubleSpinBox>
#include "pitz_daq_daqinterface_application.hpp"
#include "pitz_daq_daqinterface_collector.hpp"
#include <common/lists.hpp>

#define RETUN_LAYOUT   QVBoxLayout

namespace pitz{ namespace daq{ namespace daqinterface{

class CentralWidget : public QWidget
{
    Q_OBJECT

public:
    CentralWidget();
    ~CentralWidget();

    RETUN_LAYOUT* GetWholeLayout();

private:
    void TaskCallbackGUI(SCallArgsAll a_args,int64_t a_err);

private slots:
    void ExecuteCommandSlot();
    void InitializeCollectorsSlot();
    void InitializeSingleCollectorSlot(SCollemtorItem*& a_pItem,ptrdiff_t a_index);
    void RemoveSingleCollectorSlot(SCollemtorItem*& pItem);

private:
    QVBoxLayout     m_MainLayout;
    //::common::List<Collector*> m_listOfCollectors;
    bool    m_bInited = false;


};

}}}

#endif // PITZ_DAQ_DAQINTERFACE_CENTRALWIDGET_HPP
