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
#ifndef PITZ_DAQ_SINGLEENTRYDOOCS_HPP
#define PITZ_DAQ_SINGLEENTRYDOOCS_HPP

#include "pitz_daq_singleentry.hpp"


namespace pitz{namespace daq{

class SingleEntryDoocs : public SingleEntry
{
public:
    SingleEntryDoocs(entryCreationType::Type creationType,const char* entryLine, TypeConstCharPtr* a_pHelper);
    virtual ~SingleEntryDoocs() OVERRIDE2;

    virtual void ValueStringByKeyInherited(bool bReadAll, const char* request, char* buffer, int bufferLength)const OVERRIDE2;
    const char* rootFormatString()const OVERRIDE2;
    void PermanentDataIntoFile(FILE* fpFile)const OVERRIDE2;

    const char* doocsUrl()const {return m_doocsUrl;}

    void FromDoocsToMemory(DEC_OUT_PD(SingleData)* pMemory, const EqData* dcsData);

protected:
    bool GetEntryInfoFromServer( DEC_OUT_PD(BranchDataRaw)* a_pEntryInfo ) const;

protected:
    char* m_doocsUrl;
    char* m_rootFormatStr;
};
}}

#endif // PITZ_DAQ_SINGLEENTRYDOOCS_HPP
