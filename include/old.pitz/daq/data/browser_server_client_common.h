//
// file:		server_client_common.h
// created on:	2018 Jun 21
//

#ifndef __pitz_daq_data_browser_server_client_common_h__
#define __pitz_daq_data_browser_server_client_common_h__

#include <stdint.h>

#define PITZ_DAQ_BROWSER_SERVER_DEFAULT_PORT	10040

typedef struct ComunicationStruct{
    int64_t   ptr;
    int32_t   pid;
    int32_t   index; // before hashing should be set to 0
}ComunicationStruct;


#endif // #ifndef __pitz_daq_data_browser_server_client_common_h__
