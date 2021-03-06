
// pitz_daq_singleentry.cpp.hpp
// 2017 Nov 24

#ifndef PITZ_DAQ_SINGLEENTRY_CPP_HPP
#define PITZ_DAQ_SINGLEENTRY_CPP_HPP

#include <TTree.h>
#include <pitz_daq_data_handling_internal.h>


namespace pitz{namespace daq{

class SingleEntry;
class NewTFile;

class TreeForSingleEntry : public ::TTree
{
public:
    TreeForSingleEntry( SingleEntry* pEntry );
    ~TreeForSingleEntry() OVERRIDE2;

	void Finalize();	

private:
    SingleEntry*    m_pParentEntry;
};

}} // namespace pitz{ namespace daq{

#endif // PITZ_DAQ_SINGLEENTRY_CPP_HPP
