
// Modified 2017 Oct 23
// pitz_daq_eqfctudpmcast.hpp (old name daqadcreceiver.hpp)

#ifndef pitz_daq_eqfctudpmcast_hpp
#define pitz_daq_eqfctudpmcast_hpp

#include "pitz_daq_eqfctcollector.hpp"

#define     MAX_CHANNELS_NUM    128
#define     CODE_UDP_MCAST_DAQ  300	// eq_fct_type number for the .conf file

namespace pitz{ namespace daq{


class EqFctUdpMcast : public EqFctCollector
{

    friend class SingleEntryUdp;
public:
    EqFctUdpMcast( );
    virtual ~EqFctUdpMcast() OVERRIDE2;

protected:
    int  fct_code(void) OVERRIDE2;

    pitz::daq::SingleEntry* CreateNewEntry(entryCreationType::Type creationType,const char* entryLine) OVERRIDE2;
    void DataGetterFunctionWithWait(const SNetworkStruct* pNet, const ::std::vector<SingleEntry*>& pEntries) OVERRIDE2;
    pitz::daq::SNetworkStruct* CreateNewNetworkStruct() OVERRIDE2;

protected:
    D_string        m_hostName;
    SingleEntry*    m_vMapper[MAX_CHANNELS_NUM];

};

}} // namespace pitz, namespace daq

#endif // #ifndef pitz_daq_eqfctudpmcast_hpp
