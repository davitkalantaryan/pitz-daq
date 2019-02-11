


// pitz_daq_util_spectrumcatcher.cpp
// created on :    2018 Jan 22

#include "util/pitz_daq_util_spectrumcatcher.hpp"
#include "mclistener.hpp"
#include "udpmcastdaq_commonheader.h"
#include <stdint.h>

#define MAX_NUM_OF_BOARDS			8
#define CHANNEL_NUMBERS_ON_BOARD	8


static const int MAX_NUM_UDP_CHANNELS = MAX_NUM_OF_BOARDS*CHANNEL_NUMBERS_ON_BOARD;

class MClistener;
static const int s_cnUdpReceiveSize = sizeof(DATA_struct);
static const size_t s_cunIterCount = s_cnUdpReceiveSize / 4;

static ReturnAndLink ReceiverThreadStatic(void* a_this);
static ReturnAndLink AdministrativeThreadStatic(void* a_this);

namespace pitz{namespace daq{ namespace util{

template <typename Int4Bytes>
static inline void Swap4Bytes(Int4Bytes* a_pData);

class CUdpSystem : public CSpectrumGroupBase
{
public:
	CUdpSystem();
	~CUdpSystem();
	DATA_struct		m_dataForReceive;
};

}}}


pitz::daq::util::SpectrumCatcher::SpectrumCatcher()
	:
	m_ringBufferCount(0)
{
}


pitz::daq::util::SpectrumCatcher::~SpectrumCatcher()
{
}


int pitz::daq::util::SpectrumCatcher::Initilize(int a_ringBufferCount)
{
	if(m_ringBufferCount>0){return 1;}
	if(a_ringBufferCount<=0){return -2;}
	m_ringBufferCount = a_ringBufferCount;

	return 0;
}


void* pitz::daq::util::SpectrumCatcher::AddData(const char* a_name, const char* a_hostName, int a_adcBoard, int a_channelNumberOnBoard)
{
	// Some logic to see which type of receiver is used
	return AddUdpMcastData(a_name,a_hostName,a_adcBoard, a_channelNumberOnBoard);
}


void pitz::daq::util::SpectrumCatcher::GetDataHeader(const char* spectrName, int index)
{
}


void pitz::daq::util::SpectrumCatcher::StartThreads()
{

	//if(m_threadAdmin==)

#ifdef _WIN32
	DWORD dwThreadId;
	m_threadAdmin = CreateThread(NULL, 0, AdministrativeThreadStatic, this, 0, &dwThreadId);
	m_threadReceiver = CreateThread(NULL,0, ReceiverThreadStatic,this,0,&dwThreadId);
#else
	pthread_create(&m_threadAdmin, NULL, AdministrativeThreadStatic, this);
	pthread_create(&m_threadReceiver, NULL, ReceiverThreadStatic, this);
#endif
}


void pitz::daq::util::SpectrumCatcher::StopThreads()
{
	// to be done
}


void* pitz::daq::util::SpectrumCatcher::AddUdpMcastData(const char* a_name, const char* a_hostName, int a_adcBoard, int a_channelNumberOnBoard)
{
	size_t unKeySize;
	CSpectrumGroupSystem* pSystem = NULL;
	CUdpSystem* pSystemUdp;
	CDaqSpectra* pSpectra = NULL;
	SGroupIdentifier* pIdentifier = CreateSpectrumIdentifier(a_hostName, UDP_DAQ_BOARD_AND_CHANNEL, UDP_DAQ_BOARD_AND_CHANNEL, &unKeySize);
	int nChannelNumAll = a_adcBoard * 8 + a_channelNumberOnBoard;

	try {

		if (pIdentifier) {
			if (!m_hashSystems.FindEntry(pIdentifier, unKeySize, &pSystem)) {
				pSystem = new CUdpSystem;
				if (pSystem) {
					if (0==m_hashSystems.AddEntry(pIdentifier, unKeySize, pSystem)) {
						if (m_listSystem.AddToTheEnd(pSystem)) {
							pSystemUdp = (CUdpSystem*)pSystem;
							if (pSystemUdp->m_socket.ConnectToTheMGroup(a_hostName)) {
								delete pSystem;
								pSystem = NULL;
							}
						}
						else {
							delete pSystem;
							pSystem = NULL;
						}
					}
					else {
						delete pSystem;
						pSystem = NULL;
					}
				}
			}  // if(!m_hashSystems.FindEntry(pIdentifier,unKeySize,&pSystem)){
			DestroySpectrumIdentifier(pIdentifier);
		}  // if(pIdentifier){

		if (pSystem) {
			pSystemUdp = (CUdpSystem*)pSystem;
			nChannelNumAll %= MAX_NUM_UDP_CHANNELS;
			pSpectra = pSystemUdp->m_vSpectras[nChannelNumAll];
			if (!pSpectra) {
				pSpectra = new CDaqSpectra(m_ringBufferCount, pSystem);
				if (pSpectra) {pSystemUdp->m_vSpectras[nChannelNumAll] = pSpectra;}
			}
		}
	} // try
	catch (...)
	{
		if(pSpectra){delete pSpectra;}
		pSpectra = NULL;
	}

	return pSpectra;
}


void pitz::daq::util::SpectrumCatcher::ReceiverThread()
{
	// to be done
}


void pitz::daq::util::SpectrumCatcher::AdministrativeThread()
{
	// to be done
}



/*///////////////////////////////////////////////////////////////*/

pitz::daq::util::CUdpSystem::CUdpSystem()
{
	memset(m_vSpectras, 0, sizeof(m_vSpectras));
}


pitz::daq::util::CUdpSystem::~CUdpSystem()
{
}


void* pitz::daq::util::CUdpSystem::CreateVolumeForData()
{
	return malloc(s_cnUdpReceiveSize);
}


void pitz::daq::util::CUdpSystem::FreeVolumeForData(void* a_pVolume)
{
	free(a_pVolume);
}


int pitz::daq::util::CUdpSystem::ReceiveAndDesideToKeepOrNot()
{
	int nReceived = m_socket.recvC(&m_dataForReceive, s_cnUdpReceiveSize);

	if(nReceived== s_cnUdpReceiveSize){
		if(m_dataForReceive.endian!=1){
			Swap4Bytes(&m_dataForReceive.branch_num);
			m_dataForReceive.branch_num %= MAX_NUM_UDP_CHANNELS;
			CDaqSpectra* pSpectra = m_vSpectras[m_dataForReceive.branch_num];
			if(pSpectra){
				uint32_t* pDataToSwap = (uint32_t*)&m_dataForReceive;
				for (int i(0); i < s_cunIterCount; ++i) {
					Swap4Bytes(&pDataToSwap[i]);
				}
				int nNextIndex = (pSpectra->m_currentIndex + 1) % MAX_NUM_UDP_CHANNELS;
				memcpy(pSpectra->m_ppVolumes[nNextIndex],&m_dataForReceive,s_cnUdpReceiveSize);
				pSpectra->m_currentIndex = nNextIndex;
			}
		}  // if(m_dataForReceive.endian!=1){
	}  // if(nReceived== s_cnUdpReceiveSize){

	return 0;
}


/*//////////////////////////////////*/



/*//////////////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////////////////////*/


static ReturnAndLink ReceiverThreadStatic(void* a_this)
{
	pitz::daq::util::SpectrumCatcher* pCatcher = (pitz::daq::util::SpectrumCatcher*)a_this;
	pCatcher->ReceiverThread();
	return (TypeThreadReturn)0;
}


static ReturnAndLink AdministrativeThreadStatic(void* a_this)
{
	pitz::daq::util::SpectrumCatcher* pCatcher = (pitz::daq::util::SpectrumCatcher*)a_this;
	pCatcher->AdministrativeThread();
	return (TypeThreadReturn)0;
}



/*/////////////////////////////////////////////////////////////////////////////////////////*/
// static functions

namespace pitz{namespace daq{ namespace util{


template <typename Int4Bytes>
static inline void Swap4Bytes(Int4Bytes* a_pData)
{
	char* pcBuff = (char*)a_pData;
	
	char cTemp = pcBuff[0];
	pcBuff[0] = pcBuff[3];
	pcBuff[3] = cTemp;

	cTemp = pcBuff[1];
	pcBuff[1] = pcBuff[2];
	pcBuff[2] = cTemp;
}

}}}
