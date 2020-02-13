/*****************************************************************************
 * File:    pitz_daq_eqfctrr.hpp
 * created: 2017 May 30
 *****************************************************************************
 * Author:	D.Kalantaryan, Tel:+49(0)33762/77552 kalantar
 * Email:	davit.kalantaryan@desy.de
 * Mail:	DESY, Platanenallee 6, 15738 Zeuthen
 *****************************************************************************
 * Description
 *   ...
 ****************************************************************************/
#ifndef PITZ_DAQ_EQFCTRR_HPP
#define PITZ_DAQ_EQFCTRR_HPP

#include "eq_fct.h"
#include "pitz_daq_eqfctcollector.hpp"
#include <eq_client.h>

#define EqFctRR_code        310

namespace pitz{namespace daq{


class EqFctRR : public EqFctCollector
{
public:
    EqFctRR();
    ~EqFctRR() OVERRIDE2;

protected:
    int  fct_code(void) OVERRIDE2;

    //void DataGetterThread(SNetworkStruct* pNet);
    pitz::daq::SingleEntry* CreateNewEntry(entryCreationType::Type creationType,const char* entryLine) OVERRIDE2;
    void DataGetterFunctionWithWait(const SNetworkStruct* pNet, const ::std::vector<SingleEntry*>& pEntries) OVERRIDE2;


protected:
    D_int           m_pollingPeriod;

};

}}

#endif // PITZ_DAQ_EQFCTRR_HPP
