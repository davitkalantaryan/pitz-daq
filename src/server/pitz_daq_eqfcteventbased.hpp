
// Modified 2017 Oct 23
// pitz_daq_eqfctudpmcast.hpp (old name daqadcreceiver.hpp)

#ifndef PITZ_DAQ_COLLECTOR_EVENT_BASED_HPP
#define PITZ_DAQ_COLLECTOR_EVENT_BASED_HPP

#include "pitz_daq_eqfctcollector.hpp"


#ifndef NEWNULLPTR
#define NEWNULLPTR nullptr
#endif

namespace pitz{ namespace daq{


class EqFctEventBased : public EqFctCollector
{
public:
    EqFctEventBased( );
    virtual ~EqFctEventBased();

protected:
    virtual int  fct_code(void);

    void DataGetterThread(SNetworkStruct* pNet);
    pitz::daq::SingleEntry* CreateNewEntry(entryCreationType::Type creationType,const char* entryLine);

};

}} // namespace pitz, namespace daq

#endif // #ifndef PITZ_DAQ_COLLECTOR_EVENT_BASED_HPP
