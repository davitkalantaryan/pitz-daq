#include "stdafx.h"
#include "unnamedsemaphore.h"

#include <time.h>

using namespace DAVIT_CLASSES;

UnNamedSemaphore::UnNamedSemaphore( int a_nInitCount )	:
#ifdef WIN32
		m_nSemCount( a_nInitCount ),
#endif
		m_nWaitersCount( -a_nInitCount )
{
#ifdef WIN32
	m_Semaphore = CreateSemaphore( 0, (LONG)a_nInitCount, (LONG)MAX_SEMAPHORE_COUNT, 0 );
#else
	sem_init( &m_Semaphore, SHARING_TYPE, a_nInitCount );
#endif
}


UnNamedSemaphore::~UnNamedSemaphore( )
{
#ifdef WIN32
	CloseHandle( m_Semaphore );
#else
	sem_destroy( &m_Semaphore );
#endif
}


/*
  ERRORS

  EINVAL - sem is not a valid semaphore.

  EOVERFLOW - The maximum allowable value for a semaphore would be exceeded.
 */

int UnNamedSemaphore::Post()
{
	int nRet(-1);

#ifdef WIN32
	if( ReleaseSemaphore( m_Semaphore, 1, 0  ) )
	{
		++m_nSemCount;
		nRet = 0;
	}
#else
	 nRet = sem_post( &m_Semaphore );
#endif

	if( !nRet )
		--m_nWaitersCount;

	return nRet;
}


/*
  ERRORS

  EINVAL - sem is not a valid semaphore.

  EOVERFLOW - The maximum allowable value for a semaphore would be exceeded.
 */

int UnNamedSemaphore::Post( int a_nCounts )
{
	int nRet(-1);

#ifdef WIN32
	if( ReleaseSemaphore( m_Semaphore, (long)a_nCounts, 0  ) )
	{
		m_nSemCount += a_nCounts;
		nRet = 0;
	}
#else
	for( int i(0); i < a_nCounts; ++i )
	{
		if( ( nRet = sem_post( &m_Semaphore ) ) )
		{
			break;
		}
	}
	//nRet = sem_post( &m_Semaphore );
#endif

	if( !nRet )
		m_nWaitersCount -= a_nCounts;

	return nRet;
}


/*
  ERRORS

  EINVAL - sem is not a valid semaphore.

  EOVERFLOW - The maximum allowable value for a semaphore would be exceeded.
 */

int UnNamedSemaphore::OpenAll( )
{
	return Post( m_nWaitersCount );
}


/*
EINTR - The call was interrupted by a signal handler; see signal(7).

EINVAL - sem is not a valid semaphore.
 */
int UnNamedSemaphore::Wait()
{

#ifdef WIN32
	int nRet;

	++m_nWaitersCount;
	--m_nSemCount;
	switch( WaitForSingleObject( m_Semaphore, INFINITE ) )
	{
//	case WAIT_ABANDONED:
//		nRet = 0;
//		break;
	case WAIT_OBJECT_0:
		nRet = 0;
		break;
//	case WAIT_TIMEOUT:
//		nRet = 2;
//		break;
	default:
		++m_nSemCount;
		nRet = -1;
		break;
	}
	
	return nRet;
#else
	++m_nWaitersCount;
	return sem_wait( &m_Semaphore );
#endif
}



int UnNamedSemaphore::TimedWait(long int a_lnMaxWaitTime)
{
#ifdef WIN32
	int nRet;

	++m_nWaitersCount;
	--m_nSemCount;
	switch (WaitForSingleObject(m_Semaphore, a_lnMaxWaitTime))
	{
		//	case WAIT_ABANDONED:
		//		nRet = 0;
		//		break;
	case WAIT_OBJECT_0:
		nRet = 0;
		break;
		//	case WAIT_TIMEOUT:
		//		nRet = 2;
		//		break;
	default:
		++m_nSemCount;
		nRet = -1;
		break;
	}

	return nRet;
#else
	long int lnFinNsec;
	struct timespec aCurTime, aDelta, aFinalTime;

	++m_nWaitersCount;

	aDelta.tv_sec  = a_lnMaxWaitTime/1000;
	aDelta.tv_nsec = (a_lnMaxWaitTime%1000)*1000000;

	clock_gettime(CLOCK_REALTIME, &aCurTime);

	lnFinNsec = aCurTime.tv_nsec + aDelta.tv_nsec;

	aFinalTime.tv_sec  = aCurTime.tv_sec + aDelta.tv_sec + lnFinNsec/1000000000;
	aFinalTime.tv_nsec = lnFinNsec%1000000000;
	
	return sem_timedwait(&m_Semaphore,&aFinalTime);
#endif
}


int UnNamedSemaphore::GetValue( int* a_pnSemValue )
{
#ifdef WIN32
	*a_pnSemValue = m_nSemCount;
	return 0;
#else
	return sem_getvalue( &m_Semaphore, a_pnSemValue );
#endif
}


int UnNamedSemaphore::GetWaitersCount()const
{
	return m_nWaitersCount;
}
