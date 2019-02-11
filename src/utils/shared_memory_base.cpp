#include "stdafx.h"
#include "shared_memory_base.h"

#ifdef WIN32
#else
#include <string.h>
#endif

#include <errno.h>


Shared_Memory_Base::Shared_Memory_Base()
	:	m_hMapFile( 0 ),
#ifdef WIN32
//		m_unShMemSize(0),
#endif
		m_ShMemBuffer(0)
{
}



Shared_Memory_Base::~Shared_Memory_Base()
{
    Close2();
}


void Shared_Memory_Base::CloseBase(int a_nIsServer)
{
	if( m_hMapFile )
	{
#ifdef WIN32
		if( m_ShMemBuffer )
		{
			UnmapViewOfFile( m_ShMemBuffer ); 
		}

		CloseHandle( m_hMapFile );
#else
                shmdt(m_ShMemBuffer);
                if(a_nIsServer)shmctl( m_hMapFile,  IPC_RMID, 0 );
#endif
		m_hMapFile = 0;
		m_ShMemBuffer = NULL;
	}
}


size_t Shared_Memory_Base::Size()const
{

	size_t unSize;

#ifdef WIN32
	
	char* pRealPtr = m_ShMemBuffer - PLACE_FOR_MEM_SIZE ;

	if( sizeof(size_t) < PLACE_FOR_MEM_SIZE )
	{
		memcpy( &unSize, pRealPtr + 4, 4 );
	}
	else
	{
		memcpy( &unSize, pRealPtr, PLACE_FOR_MEM_SIZE );
	}

#else
	struct shmid_ds shminfo;

	if( shmctl( m_hMapFile, IPC_STAT, &shminfo ) != -1 )
	{
		unSize = shminfo.shm_segsz;
	}
	else
	{
//		m_hMapFile = 0;
		unSize = 0;
	}

#endif

	return unSize ;

}


char* Shared_Memory_Base::GetMemPtr()const
{
	return m_ShMemBuffer;
}



bool Shared_Memory_Base::checkExistanceAndKeep( const char* a_Name, char** a_ppShm,TYPE_MAP_FILE* a_pHandle  )
{

	bool bRet(false);
	char*& pcShMemBuffer = *a_ppShm;
	TYPE_MAP_FILE& hMapFile = *a_pHandle;

#ifdef WIN32

    hMapFile = OpenFileMappingA(
						FILE_MAP_ALL_ACCESS,		// read/write access
						FALSE,						// do not inherit the name
						a_Name	);			// name of mapping object
	
	
	if( hMapFile )
	{
		pcShMemBuffer = (char*)MapViewOfFile(hMapFile, // handle to map object
				FILE_MAP_ALL_ACCESS,  // read/write permission
				0,
				0,
				PLACE_FOR_MEM_SIZE );

		if( pcShMemBuffer )
		{
			bRet = true;
			//UnmapViewOfFile( pRet );
		}

		//CloseHandle( hMapFile );

	} // if( hMapFile )


#else

	size_t unStrLen(strlen(a_Name));
	key_t key = (key_t)hash1_( a_Name, unStrLen );

	hMapFile = shmget( key, 0, SVSEM_MODE );

	if( hMapFile >= 0 )
	{
		pcShMemBuffer = (char*)shmat( hMapFile,0,0 );
		if(pcShMemBuffer){bRet = true;}
		//shmctl( hMapFile,  IPC_RMID, 0 );
	}

#endif

	return bRet;

}



void Shared_Memory_Base::setExternall(TYPE_MAP_FILE a_Handle, char* a_pcMemory)
{
	m_hMapFile = a_Handle;
	m_ShMemBuffer = a_pcMemory;
}



int Shared_Memory_Base::createSharedMemory(TYPE_MAP_FILE* a_phMapFile,void** a_pBuffer,size_t a_unSize,const char* a_cpcName,int a_nIsServer)
{
    TYPE_MAP_FILE& hMapFile = *a_phMapFile;
    void*& pBuffer = *a_pBuffer;


#ifdef WIN32
        hMapFile = CreateFileMappingA(
                 (HANDLE)INVALID_HANDLE_VALUE,		// use paging file
                 NULL,								// default security
                 PAGE_READWRITE,					// read/write access
                 0,									// maximum object size (high-order DWORD)
                 (DWORD)a_unSize + 8,				// maximum object size (low-order DWORD)
                 a_cpcName );

        if( hMapFile )
        {

//		m_unShMemSize = a_unSize ;

                pBuffer = (void*) MapViewOfFile( hMapFile,   // handle to map object
                        FILE_MAP_ALL_ACCESS, // read/write permission
                        0,
                        0,
                        (DWORD)a_unSize + PLACE_FOR_MEM_SIZE );

                if( pBuffer )
                {
                        memset( pBuffer, 0, PLACE_FOR_MEM_SIZE );

                        if( sizeof(size_t) < PLACE_FOR_MEM_SIZE )
                        {
                            memcpy( ((char*)pBuffer) + 4, &a_unSize, 4 );
                        }
                        else
                        {
                                memcpy( ((char*)pBuffer), &a_unSize, PLACE_FOR_MEM_SIZE );
                        }

                        char* pcBuffer = (char*)pBuffer + PLACE_FOR_MEM_SIZE;
                        pBuffer = pcBuffer ;
                }
        }
        else
        {
                pBuffer = 0;
        }

        return (hMapFile && pBuffer) ? (int)hMapFile : -1;

#else

        int nRet;

        size_t unStrLen(strlen(a_cpcName));
        key_t key = (key_t)hash1_( a_cpcName, unStrLen );

        if(a_nIsServer)
        {

            hMapFile = shmget( key, a_unSize, IPC_CREAT | 0666 );

            if( hMapFile >= 0 )
            {
                    nRet = (int)hMapFile;
                    pBuffer = shmat( hMapFile,NULL,0 );
            }
            else // if( hMapFile >= 0 )
            {
                    pBuffer = NULL;

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

                    hMapFile = 0;
                    nRet = -nRet;

            } // else for if( m_hMapFile >= 0 )

        }// end  if(a_nIsServer)
        else
        {

            hMapFile = shmget( key, 0, SVSEM_MODE );

            if( hMapFile >= 0 )
            {
                    nRet = (int)hMapFile;
                    pBuffer = shmat( hMapFile,0,0 );
            }
            else
            {
                    hMapFile = 0;
                    nRet = -1;
                    pBuffer = NULL;
            }

            //return m_hMapFile;
        }

        return nRet;

#endif
}
