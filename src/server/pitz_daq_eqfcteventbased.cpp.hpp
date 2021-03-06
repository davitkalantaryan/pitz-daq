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
    SingleEntryZmqDoocs(EqFctCollector* a_pParent,entryCreationType::Type creationType,const char* entryLine, TypeConstCharPtr* a_pHelper);
    ~SingleEntryZmqDoocs() OVERRIDE2;

    void* socket()const;

    bool LoadOrValidateData(void* a_pContext);
	DEC_OUT_PD(Header)* ReadData();
	void SetMemoryBack( DEC_OUT_PD(Header)* );

private:
	void RecalculateSamples();

private:
	void		InitializeRootTreeVirt() OVERRIDE2;
	void		FinalizeRootTreeVirt() OVERRIDE2;
	void		FreeUsedMemory(DEC_OUT_PD(Header)* a_usedMemory) OVERRIDE2;

private:
    void*                           m_pSocket;
    EntryParams::String             m_zmqEndpoint;
	int32_t                        m_secondHeaderLength;
	int32_t                        m_expectedDataLength;
	//uint32_t                        m_nextExpectedMaxDataLength;
    char                            *m_pBufferForSecondHeader;
};


class SNetworkStructZmqDoocs : public SNetworkStruct
{
public:
    SNetworkStructZmqDoocs( EqFctCollector* pParentCollector );
    ~SNetworkStructZmqDoocs() OVERRIDE2 ;

    bool ResizeItemsCount(size_t a_unNewCount)const;

public:
    void*                   m_pContext;
    mutable zmq_pollitem_t* m_pItems;
private:
    mutable size_t          m_unCreatedItemsCount;
};

}}


#endif  // #ifndef PITZ_DAQ_COLLECTOR_EVENT_BASED_CPP_HPP
