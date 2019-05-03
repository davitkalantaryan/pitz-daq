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
    ~SingleEntryDoocs();

    //const char* specialStringForDoocsProperty()const;
    void ValueStringByKeyInherited(bool bReadAll, const char* request, char* buffer, int bufferLength)const;
    const char* rootFormatString()const;
    data::memory::ForServerBase* CreateMemoryInherit();
    void PermanentDataIntoFile(FILE* fpFile)const;
    //bool ValueStringByKeyInherited(const std::string& a_key, char* a_buffer, int a_bufferLength);

    const char* doocsUrl()const {return m_doocsUrl;}
    int dataType()const{return m_dataType;}
    void setDataType(int a_dataType){m_dataType = a_dataType;}

    void FromDoocsToMemory(data::memory::ForServerBase* pMemory, const EqData* dcsData);

private:
    char* m_doocsUrl;
    char* m_rootFormatStr;
    int m_dataType;
    int m_nSamples;
};
}}

#endif // PITZ_DAQ_SINGLEENTRYDOOCS_HPP
