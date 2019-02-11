/*
 *	File: pitz_daq_collectorbase.hpp
 *
 *	Created on: 30 Jan 2017
 *	Created by: Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *  This file implements ...
 *
 */
#ifndef PITZ_DAQ_COLLECTORBASE_HPP
#define PITZ_DAQ_COLLECTORBASE_HPP

#define     CODECOLLECTORBASE  299	// eq_fct_type number for the .conf file

#include	<eq_fct.h>

namespace pitz{ namespace daq{

class CollectorBase : public EqFct
{    
public:
    CollectorBase();
    virtual ~CollectorBase();

}; // class CollectorBase

}// namespace daq
}  // namespace pitz

#endif // PITZ_DAQ_COLLECTORBASE_HPP
