#ifndef __shared_memory_base_h
#define __shared_memory_base_h

#ifdef WIN32
	#include <windows.h>

	#define		PLACE_FOR_MEM_SIZE	8
	#define		TYPE_MAP_FILE		HANDLE
#else
	#include <sys/ipc.h>
	#include <sys/shm.h>
	#include <sys/stat.h>

	#ifdef __USE_GNU
		#define		SVSEM_MODE	(0x100 | 0x80 | 0x20 | 0x10)
	#else
		#define		SVSEM_MODE	(SEM_R | SEM_A | SEM_R>>3 | SEM_R>>6)
					   /* default permissions for new SV */
	#endif

	#define		TYPE_MAP_FILE		int

	#include "roundinteger.h"
#endif

//#include <stddef.h>


class Shared_Memory_Base
{
public:
	Shared_Memory_Base();

	virtual		~Shared_Memory_Base();

        virtual void    Close2(){}

        void		CloseBase(int isServer);

	size_t		Size()const;

	char*		GetMemPtr()const;

	void		setExternall(TYPE_MAP_FILE handle, char* memory);

	static bool	checkExistanceAndKeep(const char* name, char** memoryPtr,TYPE_MAP_FILE* handlePtr );

        static int      createSharedMemory(TYPE_MAP_FILE* phMapFile,void** bufferPtr,size_t size,const char* name,int isServer);

protected:
	TYPE_MAP_FILE	m_hMapFile;
	char*			m_ShMemBuffer;
};


#endif
