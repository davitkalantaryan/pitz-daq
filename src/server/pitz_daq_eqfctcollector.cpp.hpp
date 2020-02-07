
// pitz_daq_eqfctcollector.cpp.hpp
// 2020 Feb 06

#ifndef PITZ_DAQ_EQFCTCOLLECTOR_CPP_HPP
#define PITZ_DAQ_EQFCTCOLLECTOR_CPP_HPP

/*//////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
#include <pitz_daq_data_handling_internal.h>
#include <TFile.h>

class NewTFile : public TFile
{
public:
    NewTFile(const char* filePath);
    ~NewTFile() OVERRIDE2;

    void newDataAdded(Int_t newDataSize);
    Long64_t meanDataSize()const;

private:
    Long64_t    m_uncompressedDataSize;
};

#endif  // #ifndef PITZ_DAQ_EQFCTCOLLECTOR_CPP_HPP
