//
// file:			pitz_daq_collector_global.h
// created on:		2020 Jul 09
// created by:		D. Kalantaryan
//

#ifndef PITZ_DAQ_COLLECTOR_GLOBAL_H
#define PITZ_DAQ_COLLECTOR_GLOBAL_H

#include <eq_data.h>
#include <stddef.h>

#define SIGNATURE_OFFSET		8
#define HAS_HEADER(_memory)		(   (_memory) && \
	( (*reinterpret_cast< size_t** >(  reinterpret_cast<char*>(_memory)-pitz::daq::data::g_cunOffsetToData  ))==(&pitz::daq::data::g_cunOffsetToData) )   )
#define PD_HEADER(_memory)		reinterpret_cast< DEC_OUT_PD(Header)* >( reinterpret_cast<char*>(_memory)-sizeof( sizeof(DEC_OUT_PD(Header)) )   )

namespace pitz { namespace daq { namespace data {

extern const size_t		g_cunOffsetToData;

//class EqDataPD final : public ::EqData
//{
//	static void* operator new ( ::std::size_t a_size );
//	static void  operator delete (void* a_ptr);
//};

}}} // namespace pitz { namespace daq { namespace data {


#endif  // #ifndef PITZ_DAQ_COLLECTOR_GLOBAL_H
