
// pitz_daq_eqfctcollector.cpp.hpp
// 2020 Feb 06

#ifndef PITZ_DAQ_EQFCTCOLLECTOR_CPP_HPP
#define PITZ_DAQ_EQFCTCOLLECTOR_CPP_HPP

/*//////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
#include <pitz_daq_data_handling_internal.h>
#include <TFile.h>
#include <vector>


namespace pitz{namespace daq{

class TreeForSingleEntry;
class SingleEntry;

// if we will decide to rewrite tfile
class NewTFile : public ::TFile
{
public:
	NewTFile(const char* filePath, const ::std::vector< SingleEntry* >& a_list);
    ~NewTFile() OVERRIDE2;

	void FinalizeAndSaveAllTrees();
	void AddNewTree(TreeForSingleEntry* a_pNewTree);

private:
	::std::vector< TreeForSingleEntry* > m_trees;
};

}}  // namespace pitz{namespace daq{

#endif  // #ifndef PITZ_DAQ_EQFCTCOLLECTOR_CPP_HPP
