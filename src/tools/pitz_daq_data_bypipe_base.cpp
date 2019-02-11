//
// file:        mex_simple_root_reader.cpp
//

#include "common_daq_definations.h"
#include "pitz/daq/data/bypipe/base.hpp"
#include <string.h>


using namespace pitz::daq;



/***********************************************************************************************************************/

static const int s_cnPipesCount((int)data::byPipe::pipePurpose::Count);
const int data::byPipe::Base::sm_cnPipesBufferSize = s_cnPipesCount*sizeof(int);

data::byPipe::Base::Base(pipeType::Type a_pipeType)
    :
      m_pipeType(a_pipeType)
{
	memset(m_pipes, -1, sm_cnPipesBufferSize);
	m_nMaxPipePlus1 = 0;
	m_nWork = 0;
}


data::byPipe::Base::~Base()
{
}


void data::byPipe::Base::SetPipes(const int a_pipes[])
{
	int nMaxPipe = -1;

	for (int i(0); i < s_cnPipesCount; ++i) {
		m_pipes[i] = a_pipes[i];
		if (a_pipes[i] > nMaxPipe) { nMaxPipe = a_pipes[i]; }
	}

	m_nMaxPipePlus1 = nMaxPipe + 1;
}


void data::byPipe::Base::StopEngine()
{
	m_nWork = 0;
	// kill select
}


bool data::byPipe::Base::IsPipesSet()const
{
	return (m_pipes[pipePurpose::Data] > 0);
}


const int* data::byPipe::Base::pipes()const
{
	return m_pipes;
}
