//
// file:        pitz/daq/data/getter/withindexer.hpp
//

#ifndef __pitz_daq_data_getter_withindexer_hpp__
#define __pitz_daq_data_getter_withindexer_hpp__

#include <pitz/daq/data/getter/base.hpp>

namespace pitz{ namespace daq { namespace data { namespace getter{


class WithIndexer : public getter::Base
{
public:
    WithIndexer();
    virtual ~WithIndexer();

    virtual int  GetMultipleEntriesTI( const ::std::vector< ::std::string >& branchNames, int startTime, int endTime);

};


}}}} // namespace pitz{ namespace daq { namespace data { namespace getter{


#endif  // #ifndef __pitz_daq_data_getter_base_hpp__


