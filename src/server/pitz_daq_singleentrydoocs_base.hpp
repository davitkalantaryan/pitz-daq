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
#ifndef PITZ_DAQ_SINGLEENTRYDOOCS_BASE_HPP
#define PITZ_DAQ_SINGLEENTRYDOOCS_BASE_HPP

#include "pitz_daq_singleentry.hpp"


namespace pitz{namespace daq{

class SingleEntryDoocsBase : public SingleEntry
{
public:
    SingleEntryDoocsBase(entryCreationType::Type creationType,const char* entryLine, TypeConstCharPtr* a_pHelper);
    virtual ~SingleEntryDoocsBase() OVERRIDE2;

    const char* rootFormatString()const OVERRIDE2;
    const ::std::string& doocsUrl()const ;

protected:
    EntryParams::String             m_doocsUrl;
    char*                           m_rootFormatStr;
};
}}

#endif // PITZ_DAQ_SINGLEENTRYDOOCS_BASE_HPP
