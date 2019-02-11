#include "stdafx.h"
#include "shared_memory_svr.h"


#ifdef WIN32

#else

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#define  FILE_MODE	(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

#endif

#include <errno.h>


int Shared_Memory_Svr::CreateShrdMem( const size_t& a_unSize, const char* a_Name )
{

        Shared_Memory_Svr::Close2();

#ifdef WIN32
    m_hMapFile = CreateFileMappingA(
                 (HANDLE)INVALID_HANDLE_VALUE,		// use paging file
                 NULL,								// default security
                 PAGE_READWRITE,					// read/write access
                 0,									// maximum object size (high-order DWORD)
                 (DWORD)a_unSize + 8,				// maximum object size (low-order DWORD)
                 a_Name );

	if( m_hMapFile )
	{

//		m_unShMemSize = a_unSize ;

		m_ShMemBuffer = (char*) MapViewOfFile( m_hMapFile,   // handle to map object
                        FILE_MAP_ALL_ACCESS, // read/write permission
                        0,
                        0,
                        (DWORD)a_unSize + PLACE_FOR_MEM_SIZE );

		if( m_ShMemBuffer )
		{
			memset( m_ShMemBuffer, 0, PLACE_FOR_MEM_SIZE );

			if( sizeof(size_t) < PLACE_FOR_MEM_SIZE )
			{
				memcpy( m_ShMemBuffer + 4, &a_unSize, 4 );
			}
			else
			{
				memcpy( m_ShMemBuffer, &a_unSize, PLACE_FOR_MEM_SIZE );
			}

			m_ShMemBuffer += PLACE_FOR_MEM_SIZE ;
		}
	}
	else
	{
		m_ShMemBuffer = 0;
	}

	return (m_hMapFile && m_ShMemBuffer) ? (int)m_hMapFile : -1;

#else

	int nRet;

	size_t unStrLen(strlen(a_Name));

	key_t key = (key_t)hash1_( a_Name, unStrLen );
	m_hMapFile = shmget( key, a_unSize, IPC_CREAT | 0666 );

	if( m_hMapFile >= 0 )
	{
		nRet = (int)m_hMapFile;
		m_ShMemBuffer = (char*)shmat( m_hMapFile,NULL,0 );
	}
	else // if( m_hMapFile >= 0 )
	{
		m_ShMemBuffer = NULL;

		switch( errno )
		{
		case EACCES:
			nRet = 1;
			break;

		case EINVAL:
			nRet = 2;
			break;

		case ENFILE:
			nRet = 3;
			break;

		case ENOENT:
			nRet = 4;
			break;

		case ENOMEM:
			nRet = 5;
			break;

		case ENOSPC:
			nRet = 6;
			break;

		case EPERM:
			nRet = 7;
			break;

		default:
			nRet = 8;
			break;
		} // switch( errno )

		m_hMapFile = 0;
		nRet = -nRet;

	} // else for if( m_hMapFile >= 0 )

	return nRet;

#endif

}



void Shared_Memory_Svr::Close2()
{
    Shared_Memory_Base::CloseBase(1);
}
