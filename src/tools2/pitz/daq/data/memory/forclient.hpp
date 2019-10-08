//
// file:        pitz/daq/data/memory/base.hpp
// created on:  2018 Nov 08
//
#ifndef PITZ_DAQ_DATA_MEMORY_FORSERVER_HPP
#define PITZ_DAQ_DATA_MEMORY_FORSERVER_HPP

#include <pitz/daq/data/memory/base.hpp>

#ifdef ROOT_APP
//#include "Rtypes.h"
#else   // #ifdef ROOT_APP
typedef int Int_t;
typedef float Float_t;
#endif  // #ifdef ROOT_APP
#include <pitz/daq/data/entryinfo.hpp>


namespace pitz{ namespace daq{

bool copyString(char** a_dst, const char* a_str);

namespace data{ namespace memory{

class ForClient : public Base
{
public:
    ForClient( const EntryInfoBase& aInfo, void* pParent );
    ForClient(ForClient&cM,void* pParent);
    virtual ~ForClient();
#ifdef CPP11_DEFINED2
#endif

};


}}}}  // namespace pitz{ namespace daq{ namespace data{ namespace memory{


#endif // PITZ_DAQ_DATA_MEMORY_FORSERVER_HPP
