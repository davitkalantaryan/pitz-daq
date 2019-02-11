//
//
//

#ifndef PITZ_DAQ_DAQINTERFACE_DETAILSBASE_HPP
#define PITZ_DAQ_DAQINTERFACE_DETAILSBASE_HPP

#include <QDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

namespace pitz{ namespace daq{ namespace daqinterface {

class DetailsBase : public QDialog
{
    Q_OBJECT
public:
    DetailsBase();
    virtual ~DetailsBase();

protected:
    QVBoxLayout m_mainLayout;
    QLabel m_dummy;
};

}}}

#endif // PITZ_DAQ_DAQINTERFACE_DETAILSBASE_HPP
