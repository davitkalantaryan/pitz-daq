#ifndef __shared_memory_svr_h
#define __shared_memory_svr_h


#include "shared_memory_base.h"

class Shared_Memory_Svr : public Shared_Memory_Base
{
public:
	int CreateShrdMem( const size_t& a_unSize, const char* a_Name );

        void		Close2();
};

#endif
