
// pitz_daq_eqfctcollector.cpp.hpp
// 2020 Feb 06

#ifndef PITZ_DAQ_EQFCTCOLLECTOR_CPP_HPP
#define PITZ_DAQ_EQFCTCOLLECTOR_CPP_HPP

/*//////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
#include <pitz_daq_data_handling_internal.h>
#include <TFile.h>
#include <vector>

class TTree;

// if we will decide to rewrite tfile
class NewTFile : public TFile
{
public:
    NewTFile(const char* filePath);
    ~NewTFile() OVERRIDE2;

    void SaveAllTrees();
    void AddNewTree(TTree* a_pNewTree);

private:
    ::std::vector< TTree* > m_trees;
};

#endif  // #ifndef PITZ_DAQ_EQFCTCOLLECTOR_CPP_HPP
