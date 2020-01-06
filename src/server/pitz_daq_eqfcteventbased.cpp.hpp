/*
 * Remaining questions
 *  1) pitzbpmadc_rpc_server.cc, Lns: 1025-1042 (imast@?)
 */

#ifndef PITZ_DAQ_COLLECTOR_EVENT_BASED_CPP_HPP
#define PITZ_DAQ_COLLECTOR_EVENT_BASED_CPP_HPP

#include "pitz_daq_singleentrydoocs.hpp"
#include <string>
#include <stddef.h>
#include <zmq.h>

namespace pitz{namespace daq{

class SingleEntryZmqDoocs final: public SingleEntryDoocs
{
public:
    SingleEntryZmqDoocs(entryCreationType::Type creationType,const char* entryLine, TypeConstCharPtr* a_pHelper);
    ~SingleEntryZmqDoocs() OVERRIDE2;

    int zmqPort()const{return m_nPort;}
    const ::std::string& host()const{return m_hostName;}
    void* socket()const;

    bool LoadOrValidateData(void* a_pContext);
    DEC_OUT_PD(SingleData)* ReadData();
    void SetMemoryBack( DEC_OUT_PD(SingleData)* );

private:
    ::std::string   m_hostName;
    void*           m_pSocket;
    size_t          m_expectedReadHeader2;
    int             m_nPort;
    uint64_t        m_isValid : 1;
    uint64_t        m_isDataLoaded : 1;
    uint64_t        m_reserved : 62;
    char            *m_pBufferForHeader2;
};


class SNetworkStructZmqDoocs : public SNetworkStruct
{
public:
    SNetworkStructZmqDoocs( EqFctCollector* pParentCollector );
    ~SNetworkStructZmqDoocs() OVERRIDE2 ;

    void ResizeItemsCount();

public:
    void*           m_pContext;
    zmq_pollitem_t* m_pItems;
    size_t          m_unCreatedItemsCount;
};

}}


#endif  // #ifndef PITZ_DAQ_COLLECTOR_EVENT_BASED_CPP_HPP
