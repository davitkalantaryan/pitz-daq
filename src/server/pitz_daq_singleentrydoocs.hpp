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
    SingleEntryDoocs(entryCreationType::Type creationType,const char* entryLine);
    virtual ~SingleEntryDoocs();

    //const char* specialStringForDoocsProperty()const;
    void ValueStringByKeyInherited(bool bReadAll, const char* request, char* buffer, int bufferLength)const;
    const char* rootFormatString()const;
    void PermanentDataIntoFile(FILE* fpFile)const;
    //bool ValueStringByKeyInherited(const std::string& a_key, char* a_buffer, int a_bufferLength);

    const char* doocsUrl()const {return m_doocsUrl;}

    void FromDoocsToMemory(DEC_OUT_PD(SingleData)* pMemory, const EqData* dcsData);

protected:
    bool GetEntryInfoFromServer( DEC_OUT_PD(BranchDataRaw)* a_pEntryInfo ) const;
    void CreateRootFormatString();

protected:
    char* m_doocsUrl;
    char* m_rootFormatStr;
};
}}

#endif // PITZ_DAQ_SINGLEENTRYDOOCS_HPP
