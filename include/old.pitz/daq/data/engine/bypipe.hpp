//
// file:        pitz/daq/data/engine/bypipe.hpp
//

#ifndef __pitz_daq_data_engine_bypipe_hpp__
#define __pitz_daq_data_engine_bypipe_hpp__

#include <common_daq_definations.h>
#include <pitz/daq/data/engine/base.hpp>
#include <pitz/daq/data/bypipe/base.hpp>


namespace pitz{ namespace daq { namespace data { namespace engine{

typedef ssize_t (*TypeReadPipe)(int,void*,size_t);

class ByPipe : public engine::Base, public byPipe::Base
{
public:
	ByPipe(TypeReadPipe a_fpReader );
	virtual ~ByPipe();

    void Stop();
    uint32_t isRunning()const{return m_isRunning;}

	virtual int  Initialize() __OVERRIDE__ { return 0; }
	virtual void Cleanup() __OVERRIDE__  {}
	int  GetEntriesInfo( const char* rootFileName) __OVERRIDE__ __FINAL__;
	int  GetMultipleEntries( const char* rootFileName, ::common::List<TBranchItemPrivate*>* pBranches) __OVERRIDE__ __FINAL__;

protected:
	
	pitz::daq::callbackN::retType::Type CheckReportPipes( ::fd_set* pipesSet, int* pReturn);


protected:
	TypeReadPipe		m_fpReader;
	
	volatile uint32_t	m_isRunning : 1;
};


}}}}  // namespace pitz{ namespace daq { namespace data { namespace engine{


#endif  // #ifndef __pitz_daq_data_engine_bypipe_hpp__


