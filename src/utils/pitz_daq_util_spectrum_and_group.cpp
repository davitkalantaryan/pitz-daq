
// pitz_daq_util_spectrum_and_group.cpp
// 2018 Feb 08


#include "util/pitz_daq_util_spectrum_and_group.hpp"




/*//////////////////////////////////////////////////////*/
// Global functions

namespace pitz{namespace daq{ namespace util{

SSpectrumKey* CreateSpectrumKey(const char* a_hostName, int a_board, int a_channel, size_t* a_hashLength)
{
	SSpectrumKey* pRet;
	size_t unHostNameLenPlus1 = strlen(a_hostName) + 1;

	*a_hashLength = sizeof(SSpectrumKey) + unHostNameLenPlus1;
	pRet = (SSpectrumKey*)malloc(*a_hashLength);
	if (pRet) {
		pRet->board = a_board;
		pRet->channel = a_channel;
		pRet->hostName = ((char*)pRet) + sizeof(SSpectrumKey);
		memcpy(pRet->hostName, a_hostName, unHostNameLenPlus1);
	}

	return pRet;
}


void DestroySpectrumKey(SSpectrumKey* a_pGroupId)
{
	free(a_pGroupId);
}


SSpectrumGroupKey* CreateSpectrumGroupKey(const char* a_hostName, size_t a_transpot, size_t* a_hashLength)
{
	SSpectrumGroupKey* pRet;
	size_t unHostNameLenPlus1 = strlen(a_hostName) + 1;

	*a_hashLength = sizeof(SSpectrumGroupKey) + unHostNameLenPlus1;
	pRet = (SSpectrumGroupKey*)malloc(*a_hashLength);
	if (pRet) {
		pRet->transport = a_transpot;
		pRet->hostName = ((char*)pRet) + sizeof(SSpectrumGroupKey);
		memcpy(pRet->hostName, a_hostName, unHostNameLenPlus1);
	}

	return pRet;
}


void DestroySpectrumGroupKey(SSpectrumGroupKey* a_pGroupId)
{
	free(a_pGroupId);
}

}}}
