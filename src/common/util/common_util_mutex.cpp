

// Modified on 2018 Jan 24

//#include "stdafx.h"
#include "util/mutex.hpp"
#include <errno.h>

#ifdef _WIN32
#else
#include <sys/time.h>
#include <pthread.h>
#include <time.h>
#endif


/************************************************************************/
/****************************  class Mutex  *****************************/
/************************************************************************/

common::util::Mutex::Mutex()
{
#ifdef _WIN32
	m_MutexLock = CreateMutex( NULL, FALSE, NULL );
#else

	pthread_mutexattr_t attr;
	pthread_mutexattr_init( &attr );
	pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_ERRORCHECK );

	pthread_mutex_init( &m_MutexLock, &attr );

	pthread_mutexattr_destroy( &attr );
#endif
}


common::util::Mutex::~Mutex()
{
#ifdef _WIN32
	CloseHandle( m_MutexLock );
#else
	pthread_mutex_destroy( &m_MutexLock );
#endif
}


/*
 * EDEADLK	-	The current thread already owns the mutex.
 *
 */
void common::util::Mutex::lock()
{
#ifdef _WIN32
	WaitForSingleObject( m_MutexLock, INFINITE );
#else
	pthread_mutex_lock( &m_MutexLock );
#endif
}



/*
 * EBUSY	-	The mutex is already locked. 
 * EINVAL	-	Mutex is not an initialized mutex. 
 * EFAULT	-	Mutex is an invalid pointer. 
 *
 */
int common::util::Mutex::tryLock()
{
#ifdef _WIN32
	return WaitForSingleObject( m_MutexLock, 0 ) == WAIT_OBJECT_0 ? 0 : EBUSY;
#else
	return pthread_mutex_trylock( &m_MutexLock );
#endif
}


int common::util::Mutex::timedLock(int a_nTimeMs)
{
#ifdef _WIN32
	return WaitForSingleObject(m_MutexLock,a_nTimeMs) == WAIT_OBJECT_0 ? 0 : EBUSY;
#else
	struct timeval tv;
	struct timespec ts;
	long long int llnTimeNs;
	
	gettimeofday(&tv, NULL);
	llnTimeNs = 1000 * tv.tv_usec + (a_nTimeMs % 1000) * 1000000;
	ts.tv_sec = tv.tv_sec + a_nTimeMs / 1000 + llnTimeNs/1000000000;
	ts.tv_nsec = llnTimeNs%1000000000;
	
	return pthread_mutex_timedlock(&m_MutexLock,&ts);
#endif
}


/*
 * EINVAL	-	Mutex is not an initialized mutex. 
 * EFAULT	-	Mutex is an invalid pointer. 
 * EPERM	-	The calling thread does not own the mutex. 
 *
 */
int common::util::Mutex::unlock()
{
#ifdef _WIN32
	return ReleaseMutex( m_MutexLock ) ? 0 : EPERM;
#else
	return pthread_mutex_unlock( &m_MutexLock );
#endif
}

