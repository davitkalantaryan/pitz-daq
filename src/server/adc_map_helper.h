#ifndef __adc_map_helper_h__
#define __adc_map_helper_h__

#ifndef LOCK_FOR_MAIN_HED
#define	LOCK_FOR_MAIN_HED	1
#endif

#include <stddef.h>

#ifdef WIN32
#include "adcdma_win.h"
#else
#include <ADCDma.h>
#endif



template <typename Type_Adc_Map>
class ADC_MAP_Helper
{
public:
	ADC_MAP_Helper()
		:	m_pMap(NULL),
			m_nPrevEvent(-1),
			m_nLastBuffer(-1)
	{
	}


	void AttachToAdcMap(Type_Adc_Map* a_pMap)
	{
		m_pMap = a_pMap;

		LockHeaderPrvt(LOCK_FOR_MAIN_HED);
		m_nPrevEvent	= *( (int*)((unsigned char*)m_pMap->shmem_ptr + shm_gen_event) );
		m_nLastBuffer	= *( (int*)((unsigned char*)m_pMap->shmem_ptr + shm_gen_buf_num) );
		UnLockHeaderPrvt(LOCK_FOR_MAIN_HED);

		//printf("m_nLastBuffer = %d, m_nPrevEvent = %d\n\n",m_nLastBuffer,m_nPrevEvent );
	}


	bool GetBufferAndEvent(int*const& a_pnBuffer, int*const& a_pnEvent, bool*const& a_pbContinueLoop=NULL)
	{
		int& nBufferRet = *a_pnBuffer;
		int& nEventRet = *a_pnEvent;

		int nBufferCur, nBuffToTry, nEventCur, nEventToTry, nBunchOffset;
		char* pcMemory = (char*)m_pMap->shmem_ptr;
		int nBuch_size( m_pMap->get_bunch_size() );


		///Locking
		if( LockHeaderPrvt(LOCK_FOR_MAIN_HED) )return false;

		nEventCur	= *( (int*)((unsigned char*)m_pMap->shmem_ptr + shm_gen_event) );
		nBufferCur	= *( (int*)((unsigned char*)m_pMap->shmem_ptr + shm_gen_buf_num) );

		/// Unlocking
		UnLockHeaderPrvt(LOCK_FOR_MAIN_HED);

		if(m_nLastBuffer == nBufferCur)return false;

		nBuffToTry = (m_nLastBuffer+1)%BUF_NUM;

		nBunchOffset = nBuch_size * nBuffToTry;
		
		///Locking
		if( LockHeaderPrvt(nBuffToTry) )return false;
		nEventToTry = *( (int*)(pcMemory + nBunchOffset + shm_off_buf_event) ) ;
		/// Unlocking
		UnLockHeaderPrvt(nBuffToTry);

		//printf("nCurBuffer = %d, nBuffToTry = %d,  m_nLastBuffer = %d, nEventToTry = %d, m_nPrevEvent = %d\n",
		//		nBufferCur,nBuffToTry,m_nLastBuffer,nEventToTry,m_nPrevEvent);


		if( nEventToTry <= m_nPrevEvent )
		{
			if( nBufferCur != nBuffToTry )
			{
				m_nLastBuffer = nBuffToTry;
			}
			return false;
		}

		if( a_pbContinueLoop && nEventToTry==nBufferCur)
			*a_pbContinueLoop = false;

		m_nPrevEvent = nEventRet = nEventToTry;

		m_nLastBuffer = nBufferRet = nBuffToTry;

		return true;
	}



	Type_Adc_Map* operator->()
	{
		return m_pMap ;
	}



private:
	inline int LockHeaderPrvt(const int& a_nBuffer)
	{
#ifdef WIN32
		return 0;
#else
		return m_pMap->cltr_rd_lock(a_nBuffer);
#endif
	}


	inline void UnLockHeaderPrvt(const int& a_nBuffer)
	{
#ifdef WIN32
#else
		m_pMap->cltr_un_lock(a_nBuffer);
#endif
	}

private:
	Type_Adc_Map*	m_pMap;
	int				m_nPrevEvent;
	int				m_nLastBuffer;
};



#endif/* #ifndef __adc_map_helper_h__ */
