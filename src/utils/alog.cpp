/*****************************************************************************
 * File		  alog.cpp
 * created on 2012-06-26
 *****************************************************************************
 * Author:	D.Kalantaryan, Tel:033762/77552 kalantar
 * Email:	davit.kalantaryan@desy.de
 * Mail:	DESY, Platanenallee 6, 15738 Zeuthen
 *****************************************************************************
 * Description
 *   ...
 ****************************************************************************/
#include "alog.h"

#include <sys/timeb.h>
#include <time.h>
#include <sys/stat.h>

#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif


/*
 FILE*			m_pFile;
	size_t			m_unMaxSize;
	int				m_cBgColor;
	int				m_cTxColor;
 */


ALog::ALog(size_t a_unMaxSize)
	:	m_pFile(NULL),
		m_unMaxSize(a_unMaxSize),
		m_cBgColor(9),
		m_cTxColor(9),
		m_lock()
{
}


ALog::ALog(const char *a_szFileName)
        :	m_pFile(NULL),
                m_unMaxSize(DEFAULT_LOG_MAX_SIZE),
                m_cBgColor(9),
                m_cTxColor(9),
                m_lock()
{
    this->Open(a_szFileName);
}



void ALog::SetOutputPtr( FILE* a_pFile )
{
	m_pFile = a_pFile;
}



bool ALog::Open( const char* a_szFileName )
{
	if( m_pFile == 0 && a_szFileName && a_szFileName[0] != 0 )
	{
		m_pFile = fopen( a_szFileName, "a+" );

		if( m_pFile )
		{
			return true;
		}
	}

	return false;
}



bool ALog::Open( const char* a_szFileName, const char* a_pcFormat )
{
	if( m_pFile == 0 && a_szFileName && a_szFileName[0] != 0 )
	{
		m_pFile = fopen( a_szFileName, a_pcFormat );

		if( m_pFile )
		{
			return true;
		}
	}

	return false;
}


void ALog::Close()
{
    if(m_pFile)
    {
        fclose( m_pFile );
        m_pFile = 0;
    }
}


long ALog::FilePos()
{
    if(m_pFile){return ftell(m_pFile);}
    return 0;
}


int ALog::SetFilePos(long a_lnOffset)
{
    if(m_pFile){return fseek(m_pFile,a_lnOffset,SEEK_SET);}
    return -1;
}


size_t ALog::FWrite(const void* a_buffer, size_t a_buf_len)
{
    if(m_pFile){return ::fwrite(a_buffer,1,a_buf_len,m_pFile);}
    return -1;
}

int ALog::TruncateLog(long a_nLen)
{
    int nRet( -1 );

        if( m_pFile )
        {
                int nHandle = fileno( m_pFile );

#ifdef WIN32
                nRet = chsize( nHandle, (long)a_nLen );
#else
                nRet = ftruncate( nHandle, (off_t)a_nLen );
#endif

        }

        return nRet;
}


int ALog::WritePrvt2( const char* a_callerFile, int a_nLine, const char* a_callerFnc,
                      const char *a_szFmt, va_list& a_args )
{

	int nRet(0);
	
	if( m_pFile )
	{
		timeb	aCurrentTime;
		char* pcTimeline;

//#if defined(__sun)
//	ctime_r( &t, date, sizeof(date) );
//#else
		ftime( &aCurrentTime );
//		ctime_r( &t, date );
//#endif
		pcTimeline = ctime( & ( aCurrentTime.time ) );
                nRet = fprintf( m_pFile, "[%.19s.%.3hu %.4s]:", pcTimeline, aCurrentTime.millitm, &pcTimeline[20] );
                nRet += fprintf( m_pFile, "[fl:\"%s\",ln:%d,fnc:%s][", a_callerFile,a_nLine,a_callerFnc );
		nRet += vfprintf( m_pFile, a_szFmt, a_args );
                nRet += fprintf( m_pFile, "]");
		fflush( m_pFile );
	}

	return nRet;
}


int ALog::WritePrvt( const char *a_szFmt, va_list& a_args )
{

        int nRet(0);

        if( m_pFile )
        {

                 size_t unLogSize = this->GetLogSize();

                 if(unLogSize>m_unMaxSize)
                 {
                     ALog::CopyBufIntoFilePrvt("temporal.log",NULL, 0 );
                     ALog::ClearLogPrvt();
                 }

                // m_unMaxSize
                //int len = 0;
                //char date[80];
                timeb	aCurrentTime;
                char* pcTimeline;

//#if defined(__sun)
//	ctime_r( &t, date, sizeof(date) );
//#else
                ftime( &aCurrentTime );
//		ctime_r( &t, date );
//#endif
                pcTimeline = ctime( & ( aCurrentTime.time ) );

                nRet = fprintf( m_pFile, "[%.19s.%.3hu %.4s][", pcTimeline, aCurrentTime.millitm, &pcTimeline[20] );

//			fprintf( m_pFile, "[%9lu][", (unsigned long)pthread_self() );

                nRet += vfprintf( m_pFile, a_szFmt, a_args );

                nRet += fprintf( m_pFile, "]\n");

                fflush( m_pFile );
        }

        return nRet;
}



/*
 * Writing log without any time stamp
 *
 */
int ALog::WriteWTSPrvt( const char *a_szFmt, va_list& a_args )
{

	int nRet(0);
	
	if( m_pFile )
	{
		nRet = vfprintf( m_pFile, a_szFmt, a_args );
		fflush( m_pFile );
	}

	return nRet;
}



int ALog::WriteOnlyTSPrvt( )
{

	int nRet(0);
	
	if( m_pFile )
	{
		timeb	aCurrentTime;
		char* pcTimeline;

		ftime( &aCurrentTime );

		pcTimeline = ctime( & ( aCurrentTime.time ) );

		nRet = fprintf( m_pFile, "[%.19s.%.3hu %.4s]", pcTimeline, aCurrentTime.millitm, &pcTimeline[20] );

		fflush( m_pFile );
	}

	return nRet;
}




///////////////////////////////////////////////////////////////////////////////////
int ALog::Write1( const char *a_szFmt, ... )
{

	int nRet(0);

	if( m_lock.Lock() == 0 )
	{
		
		va_list args;
		va_start( args, a_szFmt );
		nRet = WritePrvt( a_szFmt, args );
		va_end( args );

		m_lock.UnLock();
	}

	return nRet;
}



/*
 * Writing log without any time stamp
 *
 */
int ALog::WriteWTS1( const char *a_szFmt, ... )
{

	int nRet(0);

	if( m_lock.Lock() == 0 )
	{
		va_list args;
		va_start( args, a_szFmt );
		nRet = WriteWTSPrvt( a_szFmt, args );
		va_end( args );

		m_lock.UnLock();
	}

	return nRet;

}



int ALog::WriteOnlyTS1( )
{

	int nRet(0);

	if( m_lock.Lock() == 0 )
	{
		nRet = WriteOnlyTSPrvt();
		m_lock.UnLock();
	}

	return nRet;
}
//////////////////////////////////////////////////////////////////////////////////



int ALog::Write2(const COLORTP* a_pBgColor, const COLORTP* a_pTxtColor, 
				 char* a_pcBuffer, size_t a_unBufSize,
				 const char *a_szFmt, ... )
{


	int nRet(0);

	if( m_lock.Lock() == 0 )
	{
		////////////////////////////////////////////
		//// Copying and Clearing log if it is needed
		if(m_unMaxSize && a_unBufSize)
		{
			size_t unLogSize = ALog::GetLogSize();

			if(unLogSize>m_unMaxSize)
			{
				ALog::CopyBufIntoFilePrvt("temporal.log",a_pcBuffer, a_unBufSize );
				ALog::ClearLogPrvt();
			}
		}


		////////////////////////////////////////////
		//// Setting bg. Color
		if( a_pBgColor )
		{
			SetBgColor1(*a_pBgColor);
		}

		////////////////////////////////////////////
		//// Setting text Color
		if( a_pTxtColor )
		{
			SetTextColor1(*a_pTxtColor);
		}

		////////////////////////////////////////////
		//// Writing log
		va_list args;
		va_start( args, a_szFmt );
		nRet = WritePrvt( a_szFmt, args );
		va_end( args );

		m_lock.UnLock();
	}

	return nRet;
}



int ALog::WriteWTS2(const COLORTP* a_pBgColor, const COLORTP* a_pTxtColor,
					char* a_pcBuffer, size_t a_unBufSize,
					const char *a_szFmt, ... )
{

	int nRet(0);

	if( m_lock.Lock() == 0 )
	{
		////////////////////////////////////////////
		//// Copying and Clearing log if it is needed
		if(m_unMaxSize && a_unBufSize)
		{
			size_t unLogSize = ALog::GetLogSize();

			if(unLogSize>m_unMaxSize)
			{
				ALog::CopyBufIntoFilePrvt("temporal.log",a_pcBuffer, a_unBufSize );
				ALog::ClearLogPrvt();
			}
		}


		////////////////////////////////////////////
		//// Setting bg. Color
		if( a_pBgColor )
		{
			SetBgColor1(*a_pBgColor);
		}

		////////////////////////////////////////////
		//// Setting text Color
		if( a_pTxtColor )
		{
			SetTextColor1(*a_pTxtColor);
		}

		////////////////////////////////////////////
		//// Writing log
		va_list args;
		va_start( args, a_szFmt );
		nRet = WriteWTSPrvt( a_szFmt, args );
		va_end( args );

		m_lock.UnLock();
	}

	return nRet;
}



int ALog::WriteOnlyTS2(const COLORTP* a_pBgColor, const COLORTP* a_pTxtColor, 
					   char* a_pcBuffer,size_t a_unBufSize)
{
	int nRet(0);

	if( m_lock.Lock() == 0 )
	{
		////////////////////////////////////////////
		//// Copying and Clearing log if it is needed
		if(m_unMaxSize && a_unBufSize)
		{
			size_t unLogSize = ALog::GetLogSize();

			if(unLogSize>m_unMaxSize)
			{
				ALog::CopyBufIntoFilePrvt("temporal.log",a_pcBuffer, a_unBufSize );
				ALog::ClearLogPrvt();
			}
		}


		////////////////////////////////////////////
		//// Setting bg. Color
		if( a_pBgColor )
		{
			SetBgColor1(*a_pBgColor);
		}

		////////////////////////////////////////////
		//// Setting text Color
		if( a_pTxtColor )
		{
			SetTextColor1(*a_pTxtColor);
		}

		////////////////////////////////////////////
		//// Writing log
		nRet = WriteOnlyTSPrvt();

		m_lock.UnLock();
	}

	return nRet;
}



int ALog::ClearLog()
{
	int nRet(-1);

	if( m_lock.Lock() == 0 )
	{
		nRet = ClearLogPrvt();
		m_lock.UnLock();
	}

	return nRet;
}



int ALog::ClearLogPrvt()
{

	int nRet( -1 );

	if( m_pFile )
	{
		int nHandle = fileno( m_pFile );

#ifdef WIN32
		nRet = chsize( nHandle, 0L );
#else
		nRet = ftruncate( nHandle, (off_t)0 );
#endif
		COLORTP aColor(m_cBgColor);
		m_cBgColor = 9;
		ALog::SetBgColor1(aColor);

		aColor = m_cTxColor;
		m_cTxColor = 9;
		ALog::SetTextColor1(aColor);

	}

	return nRet;
}



bool ALog::LockLog()
{
	bool bRet;

	if( m_pFile )
	{
		bRet = ( m_lock.Lock() == 0 ) ;
	}
	else
		bRet = false;

	return bRet;

}


void ALog::UnLockLog()
{

	if( m_pFile )
		m_lock.UnLock();

}


/*
 * Before calling this function Lock Log
 */

size_t ALog::GetLogSize()const
{

	size_t unRet(0);

	
	if( m_pFile )
	{

		struct stat fStat;

		if( !fstat( fileno( m_pFile ), &fStat ) )
		{
			unRet = (size_t)fStat.st_size;
		}

		
	}

	return unRet ;
}


/*
 * Before calling this function Lock Log
 */

size_t ALog::GetLogBuffer( char* a_Buffer, const size_t& a_unBufferSize )const
{

	size_t unRet = 0;

	if( m_pFile )
	{

		//fpos_t Pos0 = (fpos_t)0;

		//fsetpos( m_pFile, &Pos0 );

		fseek( m_pFile, 0L, SEEK_SET );
		
		unRet = (size_t)fread( a_Buffer, sizeof(char), a_unBufferSize, m_pFile );	

		
	}


	return unRet;
}



int ALog::SetTextColor1( const COLORTP& a_cColor, COLORTP* a_pcOldColor )
{

#ifdef WIN32

	//m_pFile = _fdopen(nFD);
	int nFD = _fileno( m_pFile );

	//int nFD = _open_osfhandle( (long)hConsole,_O_WRONLY);
	HANDLE hConsole = (HANDLE)_get_osfhandle(nFD);

	//HANDLE hConsole = CreateFile("test.txt", GENERIC_READ|GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS ,FILE_ATTRIBUTE_NORMAL, NULL);
	//HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	//PCONSOLE_SCREEN_BUFFER_INFO lpConsoleScreenBufferInfo = new CONSOLE_SCREEN_BUFFER_INFO;
	CONSOLE_SCREEN_BUFFER_INFO aConsoleScreenBufferInfo;
	
	BOOL bRet = GetConsoleScreenBufferInfo(hConsole, &aConsoleScreenBufferInfo);

	WORD atrbt = aConsoleScreenBufferInfo.wAttributes;

	WORD bkCol = atrbt / 16;

	if( a_pcOldColor )
	{
		*a_pcOldColor = atrbt % 16;
	}

	if( a_cColor == atrbt % 16 )
		return 0;


	atrbt = bkCol * 16 + a_cColor;

	bRet = SetConsoleTextAttribute(hConsole, atrbt);

	m_cTxColor = a_cColor;

	return 0;

#else
	
	// Command is the control command to the terminal 
	if( a_pcOldColor ){*a_pcOldColor = m_cTxColor;}

	if( a_cColor == m_cTxColor )return 0;

	char command[16];

	snprintf(command, 15, "\x1b[%dm", a_cColor + 30);
	fprintf(m_pFile, "%s", command);
	m_cTxColor = a_cColor;

	return 0;

#endif
}



int ALog::SetBgColor1( const COLORTP& a_cColor, COLORTP* a_pcOldColor )
{

#ifdef WIN32

	//m_pFile = _fdopen(nFD);
	int nFD = _fileno( m_pFile );

	//int nFD = _open_osfhandle( (long)hConsole,_O_WRONLY);
	HANDLE hConsole = (HANDLE)_get_osfhandle(nFD);

	//HANDLE hConsole = CreateFile("test.txt", GENERIC_READ|GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS ,FILE_ATTRIBUTE_NORMAL, NULL);
	//HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	//PCONSOLE_SCREEN_BUFFER_INFO lpConsoleScreenBufferInfo = new CONSOLE_SCREEN_BUFFER_INFO;
	CONSOLE_SCREEN_BUFFER_INFO aConsoleScreenBufferInfo;
	
	BOOL bRet = GetConsoleScreenBufferInfo(hConsole, &aConsoleScreenBufferInfo);

	WORD atrbt = aConsoleScreenBufferInfo.wAttributes;

	WORD bkCol = atrbt / 16;
	WORD fgCol = atrbt % 16;

	if( a_pcOldColor )
	{
		*a_pcOldColor = bkCol;
	}

	if( a_cColor == bkCol )
		return 0;

	atrbt = a_cColor * 16 + fgCol;

	bRet = SetConsoleTextAttribute(hConsole, atrbt);

	m_cBgColor = a_cColor;

	return 0;

#else
	
	// Command is the control command to the terminal 
	if( a_pcOldColor )
	{
		*a_pcOldColor = m_cBgColor;
	}

	if( a_cColor == m_cBgColor )
		return 0;

	char command[16];

	snprintf(command, 15, "\x1b[%dm", a_cColor + 40);
		
	fprintf(m_pFile, "%s", command);

	m_cBgColor = a_cColor;

	return 0;

#endif
}



void ALog::SetBold()
{
	fprintf(m_pFile, "\x1b[1m");
}



void ALog::UnBold()
{
	fprintf(m_pFile, "\x1b[0m");
}

#include <malloc.h>

void ALog::CopyBufIntoFilePrvt(const char* a_cpcFileName, char* a_pcBuffer, size_t a_unBufSize)
{
    size_t unBufSize = a_unBufSize;

    if(!a_unBufSize)
    {
        unBufSize = ALog::GetLogSize();
        a_pcBuffer = (char*)malloc(unBufSize);
        if(!a_pcBuffer) {return;}
    }

    size_t unLogSize = ALog::GetLogSize();

    unLogSize = (unLogSize<(a_unBufSize-1)) ? unLogSize : a_unBufSize-1;
    size_t unReaded = ALog::GetLogBuffer( a_pcBuffer, unLogSize);
    a_pcBuffer[unReaded] = 0;

    FILE* fpTempLog = fopen( a_cpcFileName, "w" );

    fprintf( fpTempLog,"%s",a_pcBuffer);
    fclose( fpTempLog);

    if(!a_unBufSize){free(a_pcBuffer);}
}



void ALog::SetMaxLogSize( const size_t& a_unMaxLogSize)
{
	m_unMaxSize = a_unMaxLogSize;
}













///////////////////////////////////////////////////////////////

/************************************************************************/
/****************************  class MutexDv  *************************/
/************************************************************************/

ALog::mut::mut()
{
#ifdef WIN32
	m_MutexLock = CreateMutex( NULL, FALSE, NULL );
#else

	pthread_mutexattr_t attr;
	pthread_mutexattr_init( &attr );
	pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_ERRORCHECK );

	pthread_mutex_init( &m_MutexLock, &attr );

	pthread_mutexattr_destroy( &attr );
#endif
}



ALog::mut::~mut()
{

#ifdef WIN32
	CloseHandle( m_MutexLock );
#else
	pthread_mutex_destroy( &m_MutexLock );
#endif

}



/*
 * EDEADLK	-	The current thread already owns the mutex.
 *
 */
int ALog::mut::Lock()
{

#ifdef WIN32
	/*
	 * Here is question that needs to be solved
	int nRet;
	switch( WaitForSingleObject( m_MutexLock, INFINITE ) )
	{
	case WAIT_ABANDONED:
		nRet = 0;
		break;
	case WAIT_OBJECT_0:
		nRet = 1;
		break;
	case WAIT_TIMEOUT:
		nRet = 2;
		break;
	default:
		nRet = 3;
		break;
	}
	return nRet;
	*/
	return WaitForSingleObject( m_MutexLock, INFINITE ) == WAIT_OBJECT_0 ? 0 : EDEADLK ;
#else
	return pthread_mutex_lock( &m_MutexLock );
#endif

}



/*
 * EBUSY	-	The mutex is already locked. 
 *
 * EINVAL	-	Mutex is not an initialized mutex. 
 *
 * EFAULT	-	Mutex is an invalid pointer. 
 *
 */
/*LINKAGE_SRC int MutexDv::TryLock()
{

#ifdef WIN32
	return WaitForSingleObject( m_MutexLock, 0 ) == WAIT_OBJECT_0 ? 0 : EBUSY;
#else
	return pthread_mutex_trylock( &m_MutexLock );
#endif

}*/


/*
 * EINVAL	-	Mutex is not an initialized mutex. 
 *
 * EFAULT	-	Mutex is an invalid pointer. 
 *
 * EPERM	-	The calling thread does not own the mutex. 
 *
 */
int ALog::mut::UnLock()
{

#ifdef WIN32
	return ReleaseMutex( m_MutexLock ) ? 0 : EPERM;
#else
	return pthread_mutex_unlock( &m_MutexLock );
#endif

}


