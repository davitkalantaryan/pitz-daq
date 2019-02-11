#include "browsing_funcs.h"


static inline int DumpArgs(int arg0,...)
{
    return arg0;
}

int ReadDataFromDCacheByEv2(
                       int a_nEventsMaxCount, struct19* a_pBufferForData,
                       int (*a_fpErrFunc)(const char*,...),
                       int a_fromEvent, int a_toEvent,
                       int a_nNumberOfSpectra, const BranchItem* a_pBranchItems,
                       void* a_pNotFoundedBranches ,
                       char* a_pcBufferForNames ,
                       int a_nBuffLen )
{
    return DumpArgs(a_nEventsMaxCount,a_pBufferForData,a_fpErrFunc,a_fromEvent,
                    a_toEvent,a_nNumberOfSpectra,a_pBranchItems,
                    a_pNotFoundedBranches,a_pcBufferForNames,a_nBuffLen);
}
