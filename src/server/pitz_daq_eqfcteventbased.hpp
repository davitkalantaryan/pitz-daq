
// Modified 2017 Oct 23
// pitz_daq_eqfctudpmcast.hpp (old name daqadcreceiver.hpp)

#ifndef PITZ_DAQ_COLLECTOR_EVENT_BASED_HPP
#define PITZ_DAQ_COLLECTOR_EVENT_BASED_HPP

#include "pitz_daq_eqfctcollector.hpp"


#ifndef NEWNULLPTR
#ifdef nullptr
#undef nullptr
#endif
#define NEWNULLPTR nullptr
#endif

namespace pitz{ namespace daq{


class EqFctEventBased : public EqFctCollector
{
public:
    EqFctEventBased( );
    virtual ~EqFctEventBased() OVERRIDE2;

protected:
    virtual int  fct_code(void) OVERRIDE2;

    //void DataGetterThread(SNetworkStruct* pNet) OVERRIDE2;

    virtual bool DataGetterFunctionWithWait(const SNetworkStruct* pNet, const ::std::vector<SingleEntry*>& pEntries) OVERRIDE2 ;

    pitz::daq::SingleEntry* CreateNewEntry(entryCreationType::Type creationType,const char* entryLine) OVERRIDE2;
    SNetworkStruct* CreateNewNetworkStruct() OVERRIDE2;

};

}} // namespace pitz, namespace daq

#endif // #ifndef PITZ_DAQ_COLLECTOR_EVENT_BASED_HPP
