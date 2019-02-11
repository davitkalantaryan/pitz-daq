//
//
//

#include "pitz_daq_daqinterface_detailsbase.hpp"

pitz::daq::daqinterface::DetailsBase::DetailsBase()
{
    m_dummy.setText("To be implemented");
    QFont font1 = m_dummy.font();
    font1.setPointSize(60);
    font1.setBold(true);
    m_dummy.setFont(font1);
    m_mainLayout.addWidget(&m_dummy);
    //m_dummy.setVisible(true);
    setLayout(&m_mainLayout);
    setMinimumSize(500,300);
}


pitz::daq::daqinterface::DetailsBase::~DetailsBase()
{
    //
}
