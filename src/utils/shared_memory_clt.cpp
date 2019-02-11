#include "stdafx.h"
#include "shared_memory_clt.h"


#ifdef WIN32
#else
#include <string.h>
#endif


int Shared_Memory_Clt::CreateShrdMem( const char* a_Name )
{

        Shared_Memory_Clt::Close2();

#ifdef WIN32

	size_t unSize;

    m_hMapFile = OpenFileMappingA(
						FILE_MAP_ALL_ACCESS,		// read/write access
						FALSE,						// do not inherit the name
						a_Name	);			// name of mapping object
	
	
	if( m_hMapFile )
	{
		m_ShMemBuffer = (char*) MapViewOfFile(m_hMapFile, // handle to map object
				FILE_MAP_ALL_ACCESS,  // read/write permission
				0,
				0,
				PLACE_FOR_MEM_SIZE );

		if( m_ShMemBuffer )
		{
			if( sizeof(size_t) < PLACE_FOR_MEM_SIZE )
			{
				memcpy( &unSize, m_ShMemBuffer + 4, 4 );
			}
			else
			{
				memcpy( &unSize, m_ShMemBuffer, PLACE_FOR_MEM_SIZE );
			}

			UnmapViewOfFile( m_ShMemBuffer );

			m_ShMemBuffer = 0;

			if( unSize )
			{
				m_ShMemBuffer = (char*) MapViewOfFile(m_hMapFile,	// handle to map object
					FILE_MAP_ALL_ACCESS,							// read/write permission
					0,
					0,
					unSize + PLACE_FOR_MEM_SIZE );

				m_ShMemBuffer += PLACE_FOR_MEM_SIZE ;
			}
			else
			{
				CloseHandle( m_hMapFile );
				m_hMapFile = 0;
			}
		}
	}
	else
	{
	}

   
	return (m_hMapFile && m_ShMemBuffer) ? (int)m_hMapFile : -1;

#else

	size_t unStrLen(strlen(a_Name));
	key_t key = (key_t)hash1_( a_Name, unStrLen );

	m_hMapFile = shmget( key, 0, SVSEM_MODE );

	if( m_hMapFile >= 0 )
	{
		m_ShMemBuffer = (char*)shmat( m_hMapFile,0,0 );
	}
	else
	{
		m_hMapFile = 0;
		m_ShMemBuffer = NULL;
	}

	return m_hMapFile;

#endif

}



void Shared_Memory_Clt::Close2()
{
    Shared_Memory_Base::CloseBase(0);
}
