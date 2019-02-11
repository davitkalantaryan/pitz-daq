
// Modified 2017 Oct 23
// pitz_daq_eqfctudpmcast.hpp (old name daqadcreceiver.hpp)

#ifndef __pitz_daq_eqfctudpmcast_hpp__
#define __pitz_daq_eqfctudpmcast_hpp__

#include "pitz_daq_eqfctcollector.hpp"

#define     MAX_CHANNELS_NUM    128
#define     CODE_UDP_MCAST_DAQ  300	// eq_fct_type number for the .conf file

namespace pitz{ namespace daq{


class EqFctUdpMcast : public EqFctCollector
{

    friend class SingleEntryUdp;
public:
    EqFctUdpMcast( );
    virtual ~EqFctUdpMcast();

protected:
    virtual int  fct_code(void);

    void DataGetterThread(SNetworkStruct* pNet);
    pitz::daq::SingleEntry* CreateNewEntry(entryCreationType::Type creationType,const char* entryLine);

protected:
    D_string        m_hostName;
    SingleEntry*    m_vMapper[MAX_CHANNELS_NUM];

};

}} // namespace pitz, namespace daq

#endif // #ifndef __pitz_daq_eqfctudpmcast_hpp__
