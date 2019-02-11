//
// file:        pitz_daq_daqinterface_collector.hpp
// created on:  2018 Oct 25
//

#ifndef PITZ_DAQ_DAQINTERFACE_COLLECTOR_HPP
#define PITZ_DAQ_DAQINTERFACE_COLLECTOR_HPP

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFrame>
#include "pitz_daq_daqinterface_application.hpp"
#include "pitz_daq_daqinterface_detailsbase.hpp"

namespace pitz {namespace daq{ namespace daqinterface {

class Collector : public QWidget
{
    Q_OBJECT

public:
    Collector(SCollemtorItem* pItem);
    ~Collector();

public slots:
    void ChangedSlot(int code);
    void StartStopPushedSlot();
    void OnlineOfflinePushedSlot();


private:
    QHBoxLayout     m_MainLayout;
    QLabel  m_serverName;
    QLabel  m_hostAndRpcNumber;
    QLabel  m_serverStatusAndGenEvent;
    QPushButton m_startStop;
    QPushButton m_onlineOffline;
    QPushButton m_details;
    SCollemtorItem* m_pItem;
    DetailsBase m_detailsDlg;
};

}}}

#endif // PITZ_DAQ_DAQINTERFACE_COLLECTOR_H
