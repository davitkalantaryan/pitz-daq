//
// file:        pitz/daq/dataclientpipe.hpp
//

#ifndef __pitz_daq_data_getter_tosocketpipe_hpp__
#define __pitz_daq_data_getter_tosocketpipe_hpp__

#include <pitz/daq/data/getter/topipe.hpp>


namespace pitz{ namespace daq { namespace data{ namespace getter{


class ToSocketPipe : public getter::ToPipe
{
public:
    ToSocketPipe();
    virtual ~ToSocketPipe();
    callbackN::retType::Type DoNextStep(SPipeCallArgs*) __OVERRIDE__ ;
};


}}}}  // namespace pitz{ namespace daq { namespace data{ namespace getter{


#endif  // #ifndef __pitz_daq_data_getter_topipebase_hpp__


