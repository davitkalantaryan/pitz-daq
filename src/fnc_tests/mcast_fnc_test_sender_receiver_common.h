//
// file:            mcast_fnc_test_sender_receiver_common.h
// created on:      2020 March 04
// created by:      D. Kalantaryan
//

#ifndef FNC_TESTS_MCAST_FNC_TEST_SENDER_RECEIVER_COMMON_H
#define FNC_TESTS_MCAST_FNC_TEST_SENDER_RECEIVER_COMMON_H

#include <pitz_daq_data_handling_internal.h>
#include <stddef.h>

#define TEST_SPECTRUM_NUMBER    0

//struct DATA_struct
//{
//    int              endian;
//    int              branch_num;
//    int              seconds;
//    int              gen_event;
//    int              samples;
//    float            f[2048];
//};

struct DATA_structBase
{
    int              endian;
    int              branch_num;
    int              seconds;
    int              gen_event;
    int              samples;
};



struct DATA_struct
{
    struct DATA_structBase  base;
    float                   f[2048];
};


#define DATA_FROM_DATA_STRUCT(_dataStruct)  \
    STATIC_CAST2( void*,REINTERPRET_CAST2(char*,_dataStruct) + sizeof(struct DATA_structBase) )


#ifdef __cplusplus
extern "C"{
#endif


struct DATA_structBase* CreateDataStructWithSize(size_t designedSize, size_t* pRealSize);
void FreeNetworkDataBuffer(struct DATA_structBase* netDataBuffer);


#ifdef __cplusplus
}
#endif


#endif  // #ifndef FNC_TESTS_MCAST_FNC_TEST_SENDER_RECEIVER_COMMON_H
