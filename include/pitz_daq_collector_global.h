//
// file:			pitz_daq_collector_global.h
// created on:		2020 Jul 09
// created by:		D. Kalantaryan
//

#ifndef PITZ_DAQ_COLLECTOR_GLOBAL_H
#define PITZ_DAQ_COLLECTOR_GLOBAL_H

#include <pitz_daq_data_handling_internal.h>
#include <stddef.h>
#include <stdint.h>

#define PDD_SIGNATURE_VALUE		0x0123456789abcdef
#define PDD_MEMORY_OFFSET		( sizeof(::pitz::daq::data::SMemoryHeader) + sizeof(DEC_OUT_PD(Header))  )

#define HEADER_FROM_HEADER(_header)	\
	reinterpret_cast< ::pitz::daq::data::SMemoryHeader* >(  reinterpret_cast<char*>(_header)-sizeof(::pitz::daq::data::SMemoryHeader)  )
#define HEADER_FROM_MEM(_memory)	\
	reinterpret_cast< ::pitz::daq::data::SMemoryHeader* >(  reinterpret_cast<char*>(_memory)-PDD_MEMORY_OFFSET  )
#define HAS_HEADER_RAW(_memory)		(  (*reinterpret_cast<uint64_t*>(_memory)) ==  PDD_SIGNATURE_VALUE  )
#define HAS_HEADER(_memory)			( (_memory) && HAS_HEADER_RAW( HEADER_FROM_MEM(_memory) )  )
#define PD_HEADER(_memory)			reinterpret_cast< DEC_OUT_PD(Header)* >( reinterpret_cast<char*>(_memory)-sizeof(DEC_OUT_PD(Header))  )

namespace pitz { namespace daq { namespace data {

struct SMemoryHeader{
	uint64_t	signature;
	void*		priv;
	PADDIN32(0)
};

//class EqDataPD final : public ::EqData
//{
//	static void* operator new ( ::std::size_t a_size );
//	static void  operator delete (void* a_ptr);
//};

}}} // namespace pitz { namespace daq { namespace data {


#endif  // #ifndef PITZ_DAQ_COLLECTOR_GLOBAL_H
