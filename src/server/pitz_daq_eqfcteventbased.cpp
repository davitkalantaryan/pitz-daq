/*
 * Remaining questions
 *  1) pitzbpmadc_rpc_server.cc, Lns: 1025-1042 (imast@?)
 */

#include "pitz_daq_eqfcteventbased.hpp"
#include <zmq.h>
#include "pitz_daq_singleentrydoocs.hpp"

using namespace pitz::daq;

namespace pitz{namespace daq{

class SingleEntryZmqDoocs final: public SingleEntryDoocs
{
public:
    //using SingleEntryDoocs::SingleEntryDoocs;
    SingleEntryZmqDoocs(entryCreationType::Type creationType,const char* entryLine);

    int zmqPort()const{return m_nPort;}
    const ::std::string& host()const{return m_hostName;}

private:
    void*           m_pSocket;
    int             m_nPort;
    int             m_nReserved;
    ::std::string   m_hostName;
};
}}


EqFct* eq_create(int a_eq_code, void* /*a_arg*/)
{
    ::EqFct* pRet = NEWNULLPTR;
    //getchar();

    switch (a_eq_code)
    {
    case CODE_EVENT_BASED_DAQ:
        pRet = new pitz::daq::EqFctEventBased;
        break;
    default: break;
    }
    return pRet;
}


/*////////////////////////////////////////////////////////////////////*/


pitz::daq::EqFctEventBased::EqFctEventBased()
{
}



pitz::daq::EqFctEventBased::~EqFctEventBased()
{
}


int pitz::daq::EqFctEventBased::fct_code()
{
    return CODE_EVENT_BASED_DAQ;
}


pitz::daq::SingleEntry* pitz::daq::EqFctEventBased::CreateNewEntry(entryCreationType::Type a_creationType,const char* a_entryLine)
{
    SingleEntryZmqDoocs* pEntry = new SingleEntryZmqDoocs(a_creationType,a_entryLine);

    if(!pEntry){return pEntry;}

    return pEntry;
}


void pitz::daq::EqFctEventBased::DataGetterThread(SNetworkStruct* a_pNet)
{
    zmq_pollitem_t* pItems = NEWNULLPTR;
    //a_pNet->
}


/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
pitz::daq::SingleEntryZmqDoocs::SingleEntryZmqDoocs(entryCreationType::Type a_creationType,const char* a_entryLine)
    :
      SingleEntryDoocs(a_creationType,a_entryLine)
{
    //
}
