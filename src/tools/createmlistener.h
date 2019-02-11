#ifndef __createmlistener_h__
#define __createmlistener_h__

typedef void (*TYPE_CALLBACK)(void*);

extern void* CreateMListener(TYPE_CALLBACK a_fpCallBack);
extern void SendToListener(void* Listener, void* Info);
extern void DestroyMListener(void* pListener);


#ifdef _WIN64
	#ifndef WIN32
		#define WIN32
	#endif
#endif

#ifdef _MSC_VER
#if(_MSC_VER >= 1400)
//#define		_CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)
#endif
#endif

#include <mex.h>

#ifdef WIN32
#include <windows.h>
#else  /*#ifdef WIN32*/
#include <pthread.h>
#include <string.h>
#endif  /*#ifdef WIN32*/


#ifdef WIN32
#define		_CLASS_NAME2_	"CreateMListener"
#define		_CLASS_NAME_L_	L"CreateMListener"
#ifndef     _COMMON_CODE_
#define		_COMMON_CODE_		WM_USER+2
#endif/* #ifndef _COMMON_CODE_ */

LRESULT CALLBACK WndProcStat3(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
#else
#ifndef     _COMMON_CODE_
#define		_COMMON_CODE_		SIGRTMIN+2
//#define CODE_TO_SEND SIGUSR1
#endif/* #ifndef _COMMON_CODE_ */
#endif



typedef struct MListener
{
	TYPE_CALLBACK	m_fpCallBack;
#ifdef WIN32
	HWND			m_hWnd;
#else
	pthread_t		m_ThreadID;
	void*			m_pData;
#endif
}MListener;



#ifdef WIN32
	typedef HANDLE			TYPE_MUT;
#else
	typedef pthread_mutex_t	TYPE_MUT;
#endif

inline void	SilentArgs(...){}


class SimpleMutex
{

public:
	SimpleMutex()
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

	~SimpleMutex()
	{
#ifdef WIN32
		CloseHandle( m_MutexLock );
#else
		pthread_mutex_destroy( &m_MutexLock );
#endif
	}

	int	Lock()
	{
#ifdef WIN32
		return WaitForSingleObject( m_MutexLock, INFINITE ) ? 0 : (int)GetLastError();
#else
		return pthread_mutex_lock( &m_MutexLock );
#endif
	}


        int TryLock()
        {

#ifdef WIN32
                return WaitForSingleObject( m_MutexLock, 0 ) == WAIT_OBJECT_0 ? 0 : -5;
#else
                return pthread_mutex_trylock( &m_MutexLock );
#endif

        }



        int InsTryLock(volatile int* a_pnLoop)
        {
#ifdef WIN32
#else
            struct timespec waitspec;
#endif
            int nRet(1);

            while(nRet && *a_pnLoop)
            {
                nRet = SimpleMutex::TryLock();
                if(nRet)
                {
#ifdef WIN32
                    Sleep(1L);

#else
                    waitspec.tv_sec = 0;
                    waitspec.tv_nsec = 1000000;
                    nanosleep(&waitspec, NULL);
#endif
                }
            }


            return nRet;

        }


	void	UnLock()
	{
#ifdef WIN32
		ReleaseMutex( m_MutexLock ) ;
#else
		pthread_mutex_unlock( &m_MutexLock );
#endif
	}

private:
	TYPE_MUT			m_MutexLock;
};

template <typename Type,int _SIZE_>
class FastFifo
{
public:
	FastFifo():m_nInStack(0)
	{
	}


	void AddToTheEnd(const Type& a_tNew)
	{
		if( !m_Mutex.Lock() )
		{
			if(m_nInStack == _SIZE_ )
			{
				memmove(m_pElemets,m_pElemets+1,sizeof(Type)*(--m_nInStack) );
			}

			m_pElemets[m_nInStack++] = a_tNew;
			m_Mutex.UnLock();
		}
	}


	bool AddToTheEndIfFree(const Type& a_tNew)
	{
		bool bRet(false);

		if( !m_Mutex.Lock() )
		{
			if(m_nInStack < _SIZE_ )
			{
				m_pElemets[m_nInStack++] = a_tNew;
				bRet = true;
			}

			m_Mutex.UnLock();
			
		}

		return bRet;
	}


	bool GetFirst(Type* a_pElm)
	{
		bool bRet(false);

		if( !m_Mutex.Lock() )
		{
			if(m_nInStack)
			{
				bRet = true;
				*a_pElm = m_pElemets[0];
				memmove( m_pElemets,m_pElemets+1,sizeof(Type)*(--m_nInStack) );
			}
			m_Mutex.UnLock();
		}

		return bRet;
	}


	int size()const
	{
		int nSize(0);

		if( !m_Mutex.Lock() )
		{
			nSize = m_nInStack;
			m_Mutex.UnLock();
		}

		return nSize;
	}


	bool GetByKeyFunc2(Type* a_pRet, bool (*a_fpKeyFunc)(Type,const void*),const void* a_pOut)
	{
		bool bRet(false);
		int i(0);

		if( !m_Mutex.Lock() )
		{
			for(;i<m_nInStack;++i)
			{
				if( (*a_fpKeyFunc)(m_pElemets[i],a_pOut) )
				{
					bRet = true;
					*a_pRet = m_pElemets[i];
					break;
				}
			}
			m_Mutex.UnLock();
		}

		return bRet;
	}

	bool GetByKeyFuncAndDelete(Type* a_pRet, bool (*a_fpKeyFunc)(Type,const void*),const void* a_pOut)
	{
		bool bRet(false);
		int i(0);

		if( !m_Mutex.Lock() )
		{
			for(;i<m_nInStack;++i)
			{
				if( (*a_fpKeyFunc)(m_pElemets[i],a_pOut) )
				{
					bRet = true;
					*a_pRet = m_pElemets[i];
					memmove( m_pElemets+i,m_pElemets+i+1,sizeof(Type)*(--m_nInStack - i) );
					break;
				}
			}
			m_Mutex.UnLock();
		}

		return bRet;
	}
private:
	int					m_nInStack;
	Type				m_pElemets[_SIZE_];
	mutable SimpleMutex	m_Mutex;
};


enum RET_Types
{
	NO_ERROR__		= 0,
	NOT_STRING		= 1,
	LOW_MEMORY		= 2,
	UNABLE_GET_STR	= 3,

};

const char*const	g_vcpcErrors2[]={
	"No error",
	"Argument is not string!",
	"Not enougth memory!",
	"Couldn't get string"
};


inline int CreateAndCopy3a( const mxArray* a_Source, char** a_ppcBuffer, int* a_pnInitBufLen, bool a_bThrow )
{
	
	int nBufLen;
	char*& pcBuffer = *a_ppcBuffer;
	int& nInitBufLen = *a_pnInitBufLen;
	char* pcTempBuffer = NULL;

	if(!mxIsChar(a_Source))
	{
		if(a_bThrow)
		{
			mexErrMsgTxt(g_vcpcErrors2[NOT_STRING]);
		}

		return NOT_STRING;
	}
	
	nBufLen = 1 + (int)( mxGetM( a_Source ) * mxGetN( a_Source ) );

	if(nBufLen>nInitBufLen)
	{
		pcTempBuffer = (char*)realloc(pcBuffer,nBufLen);
		if(!pcTempBuffer)
		{
			if(a_bThrow)
				mexErrMsgTxt(g_vcpcErrors2[LOW_MEMORY]);

			return LOW_MEMORY;
		}

		nInitBufLen = nBufLen;
		pcBuffer = pcTempBuffer;
	}
		
	if( mxGetString( a_Source, pcBuffer, nBufLen )  )
	{
		if(a_bThrow)
			mexErrMsgTxt(g_vcpcErrors2[UNABLE_GET_STR]);
		return UNABLE_GET_STR;
	}
	
	return NO_ERROR__;
}



inline int CreateAndCopy4( const mxArray* a_Source, char** a_ppcBuffer, bool a_bThrow )
{
	
	int nBufLen;
	char*& pcBuffer = *a_ppcBuffer;
	char* pcTempBuffer = NULL;

	if(!mxIsChar(a_Source))
	{
		if(a_bThrow)
		{
			mexErrMsgTxt(g_vcpcErrors2[NOT_STRING]);
		}

		return NOT_STRING;
	}
	
	nBufLen = 1 + (int)( mxGetM( a_Source ) * mxGetN( a_Source ) );

	pcTempBuffer = (char*)realloc(pcBuffer,nBufLen);
	if(!pcTempBuffer)
	{
		if(a_bThrow)mexErrMsgTxt(g_vcpcErrors2[LOW_MEMORY]);
		return LOW_MEMORY;
	}

	pcBuffer = pcTempBuffer;
		
	if( mxGetString( a_Source, pcBuffer, nBufLen )  )
	{
		if(a_bThrow)mexErrMsgTxt(g_vcpcErrors2[UNABLE_GET_STR]);
		return UNABLE_GET_STR;
	}
	
	return NO_ERROR__;
}


#endif  /* #ifndef __createmlistener_h__ */
