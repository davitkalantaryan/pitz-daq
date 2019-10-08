
// pitz_daq_eqfctcollector.cpp.hpp
// 2017 Nov 24

#ifndef PITZ_DAQ_EQFCTCOLLECTOR_CPP_HPP
#define PITZ_DAQ_EQFCTCOLLECTOR_CPP_HPP

#include <string>
#include "pitz_daq_singleentry.hpp"


namespace pitz{namespace daq{

class RTree : public ::TTree
{
public:
    RTree( SingleEntry* pEntry );
    ~RTree() OVERRIDE2;

public:
    SingleEntry*    m_pParentEntry;
};

}} // namespace pitz{ namespace daq{

#endif // PITZ_DAQ_EQFCTCOLLECTOR_CPP_HPP
