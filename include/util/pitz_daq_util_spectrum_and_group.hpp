
// pitz_daq_util_spectrum_and_group.hpp
// to include  ->  #include "util/pitz_daq_util_spectrum_and_group.hpp"
// 2018 Feb 08
// if any function return type is int,
// then <0 means error >=0 means result
// if return is void*
// then NULL is error, others are return (for example resource for future calls)


#ifndef __pitz_daq_util_spectrum_and_group_hpp__
#define __pitz_daq_util_spectrum_and_group_hpp__


#include <stddef.h>
#include "util/readwritelock.hpp"
#include "util/mutex.hpp"
#include "util/list.hpp"
#include "util/hashtbl.hpp"

#define	CURRENT_INDEX				-1

namespace pitz{namespace daq{ namespace util{

namespace dataTypes{enum Type{FLOAT,DOUBLE};}
struct SSpectrumKey{int board, channel;char* hostName;};
struct SSpectrumGroupKey{size_t transport;char* hostName;};


class CSpectrumBase
{
	friend class SpectrumCatcher;
public:
	virtual ~CSpectrumBase() {}
	virtual void*	CreateVolumeForData() = 0;
	virtual void	FreeVolumeForData(void* vol) = 0;
	/**/
	int CreateAllVolumes();
protected:
	void**					m_ppVolumes;
	const int				m_ringCount;
	int						m_currentIndex;

};


class CSpectrumGroupBase
{
	friend class SpectrumCatcher;
public:
	virtual ~CSpectrumGroupBase() {}
	virtual CSpectrumBase*	ReceiveAndDesideToKeepOrNot() = 0;
	/**/
	int AddSpectra(CSpectrumBase* spectra);
protected:
	common::HashTbl<CSpectrumBase*>		m_hashSpectra;
	common::List<CSpectrumBase*>		m_listSpectra;
};


SSpectrumKey* CreateSpectrumKey(const char* hostName, int board, int channel, size_t* hashLength);
void DestroySpectrumKey(SSpectrumKey* groupId);

SSpectrumGroupKey* CreateSpectrumGroupKey(const char* hostName, size_t transport, size_t* hashLength);
void DestroySpectrumGroupKey(SSpectrumGroupKey* groupId);

}}}

#endif  // #ifndef __pitz_daq_util_spectrum_and_group_hpp__
