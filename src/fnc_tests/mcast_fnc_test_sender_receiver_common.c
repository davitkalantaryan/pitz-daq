//
// file:            mcast_sender_receiver_common.h
// created on:      2020 March 04
// created by:      D. Kalantaryan
//

#include "mcast_fnc_test_sender_receiver_common.h"
#include <stdlib.h>

#ifdef __cplusplus
extern "C"{
#endif


struct DATA_structBase* CreateDataStructWithSize(size_t a_designedSize, size_t* a_pRealSize)
{
    size_t unStructSize = (a_designedSize>sizeof(struct DATA_structBase)) ? a_designedSize : sizeof(struct DATA_structBase);

    if(a_pRealSize){*a_pRealSize=unStructSize;}
    return STATIC_CAST2(struct DATA_structBase*,malloc(unStructSize));
}


void FreeNetworkDataBuffer(struct DATA_structBase* a_netDataBuffer)
{
    free(a_netDataBuffer);
}


#ifdef __cplusplus
}
#endif
