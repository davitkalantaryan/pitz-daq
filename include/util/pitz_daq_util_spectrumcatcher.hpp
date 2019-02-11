
// pitz_daq_util_spectrumcatcher.hpp
// in order to include  ->  #include "util/pitz_daq_util_spectrumcatcher.hpp"
// created on :    2018 Jan 22
// 
// if any function return type is int,
// then <0 means error >=0 means result
// if return is void*
// then NULL is error, others are return (for example resource for future calls)

#ifndef __pitz_daq_util_spectrumcatcher_hpp__
#define __pitz_daq_util_spectrumcatcher_hpp__

#include "util/pitz_daq_util_spectrum_and_group.hpp"
#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
typedef HANDLE	TypeThread;
typedef DWORD	TypeThreadReturn;
#define	ReturnAndLink	TypeThreadReturn WINAPI
#else
#include <pthread.h>
typedef pthread_t	TypeThread;
typedef void*		TypeThreadReturn;
#define	ReturnAndLink	TypeThreadReturn
#endif

#define SOCKET_RECEIVE_TIMEOUT		2000
#define UDP_TRANSPORT_INDEX			-1


namespace pitz{namespace daq{ namespace util{

class SpectrumCatcher
{
public:
	SpectrumCatcher();
	virtual ~SpectrumCatcher();

	int		Initilize(int ringBufferCount);
	void	StartThreads();
	void	StopThreads();
	void*	AddData(const char* a_name, const char* hostName, int adcBoard, int channelNumberOnBoard);
	void	GetDataHeader(const char* spectrName, int index=CURRENT_INDEX);

protected:
	void*	AddUdpMcastData(const char* a_name,const char* hostName, int adcBoard, int channelNumberOnBoard);

public:
	void	ReceiverThread();
	void	AdministrativeThread();

protected:
	common::util::Mutex						m_lockForSystems2;
	common::util::ReadWriteLock				m_rwLockForSpectras;
	common::List<CSpectrumGroupBase*>		m_listSystem;
	common::HashTbl<CSpectrumGroupBase*>	m_hashSystems;
	int										m_ringBufferCount;

	TypeThread								m_threadReceiver;
	TypeThread								m_threadAdmin;

};


#if 0
struct SpectrumHeader
{
	int		eventNumber;
	int		seconds;
	int		samples;
	int		dataType;
};

#endif


}}}



#endif  // #ifndef __pitz_daq_util_spectrumcatcher_hpp__
