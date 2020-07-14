//
// file:        pitz/daq/data/bypipe/base.hpp
//

#ifndef __pitz_daq_data_bypipe_base_hpp__
#define __pitz_daq_data_bypipe_base_hpp__

#include <pitz/daq/base.hpp>
#include <stdint.h>
#ifdef _WIN32
#ifndef ssize_t_defined
#define ssize_t_defined
typedef int ssize_t;
#endif
#else
#include <unistd.h>
#include <sys/select.h>
#endif


namespace pitz{ namespace daq { namespace data { namespace byPipe{

namespace pipePurpose { enum Type { Data, Cntr, Info, Rep, Err, Count }; }
namespace pipeType { enum Type { NoPipe,Unknown, Pipe, Socket }; }

// todo: proper rombic ingeritance should be implemented
//class Base : public pitz::daq::Base //
class Base 
{
public:
	Base(pipeType::Type pipeType );
	virtual ~Base();

	pipeType::Type pipeType()const {return m_pipeType;}
	void SetPipes(const int pipes[]);
	void StopEngine();
	bool IsPipesSet()const;
	const int*	pipes()const;

public:
	static const int	sm_cnPipesBufferSize;
protected:
	pipeType::Type		m_pipeType;
	int					m_pipes[pipePurpose::Count];
	int					m_nMaxPipePlus1;
	volatile uint32_t	m_nWork : 1;
	
};


}}}}  // namespace pitz{ namespace daq { namespace data { namespace engine{


#endif  // #ifndef __pitz_daq_data_bypipe_base_hpp__


