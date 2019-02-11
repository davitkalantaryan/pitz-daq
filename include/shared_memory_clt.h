#ifndef __shared_memory_clt_h
#define __shared_memory_clt_h

#include "shared_memory_base.h"

class Shared_Memory_Clt : public Shared_Memory_Base
{
public:
	int CreateShrdMem( const char* a_Name );

        void		Close2();
};

#endif
