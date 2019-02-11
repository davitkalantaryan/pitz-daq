//
// file:        pitz/daq/data/getter/noindexing.hpp
//

#ifndef __pitz_daq_data_getter_noindexing_hpp__
#define __pitz_daq_data_getter_noindexing_hpp__

#include <pitz/daq/data/getter/base.hpp>

namespace pitz{ namespace daq { namespace data { namespace getter{


class NoIndexing : public getter::Base
{
public:
    NoIndexing();
    virtual ~NoIndexing();

    virtual int  GetMultipleEntriesTI( const ::std::vector< ::std::string >& branchNames, int startTime, int endTime) __OVERRIDE__;

};


}}}} // namespace pitz{ namespace daq { namespace data { namespace getter{


#endif  // #ifndef __pitz_daq_data_getter_noindexing_hpp__


