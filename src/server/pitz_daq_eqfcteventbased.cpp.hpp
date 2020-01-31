/*
 * Remaining questions
 *  1) pitzbpmadc_rpc_server.cc, Lns: 1025-1042 (imast@?)
 */

#ifndef PITZ_DAQ_COLLECTOR_EVENT_BASED_CPP_HPP
#define PITZ_DAQ_COLLECTOR_EVENT_BASED_CPP_HPP

#include "pitz_daq_singleentrydoocs_base.hpp"
#include <string>
#include <stddef.h>
#include <zmq.h>

namespace pitz{namespace daq{

class SingleEntryZmqDoocs final: public SingleEntryDoocsBase
{
public:
    SingleEntryZmqDoocs(entryCreationType::Type creationType,const char* entryLine, TypeConstCharPtr* a_pHelper);
    ~SingleEntryZmqDoocs() OVERRIDE2;

    int zmqPort()const{return m_nPort;}
    const ::std::string& host()const{return m_hostName.value();}
    void* socket()const;

    bool LoadOrValidateData(void* a_pContext);
    DEC_OUT_PD(SingleData)* ReadData();
    void SetMemoryBack( DEC_OUT_PD(SingleData)* );

private:
    void*                           m_pSocket;
    EntryParams::String             m_hostName;
    EntryParams::IntParam<size_t>   m_expectedRead1;
    EntryParams::IntParam<size_t>   m_expectedRead2;
    int                             m_nPort;
    int                             m_nReserved;
    uint64_t                        m_isDataLoaded : 1;
    uint64_t                        m_reserved : 63;
    char                            *m_pBufferForHeader;
};


class SNetworkStructZmqDoocs : public SNetworkStruct
{
public:
    SNetworkStructZmqDoocs( EqFctCollector* pParentCollector );
    ~SNetworkStructZmqDoocs() OVERRIDE2 ;

    void ResizeItemsCount();

public:
    void*                   m_pContext;
    mutable zmq_pollitem_t* m_pItems;
    mutable size_t          m_unCreatedItemsCount;
    mutable time_t          m_lastUpdateTime;
};

}}


#endif  // #ifndef PITZ_DAQ_COLLECTOR_EVENT_BASED_CPP_HPP
