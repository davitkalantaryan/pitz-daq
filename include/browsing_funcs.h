#ifndef BROWSING_FUNCS_H
#define BROWSING_FUNCS_H

#define _INCORRECT_BRANCH_NAME_ -1
#define _INCORRECT_TREE_NAME_ -1
#define _INCORRECT_TIME_ -2
#define _UNABLE_TO_OPEN_ROOT_ -3

#define _DEF_BUFFER_LEN_IND_FILES_  512

#include <stddef.h>

#include "commonheader_root.h"



//typedef struct19* TPONEEVSPEC;
typedef const char*   TPCHPTR;

#ifdef __cplusplus
extern "C"
{
#endif

enum SEARCH_TYPE_Type{SEARCH_TYPE_BY_EVENT,SEARCH_TYPE_BY_TIME};

typedef struct STmEvPointers
{
    // Members
    SEARCH_TYPE_Type m_type;
    const int *m_cpnBegBufIn,*m_cpnEndBufIn,*m_cpnBegTimeIn,*m_cpnEndTimeIn;
    mutable int m_nBegTimeFile,m_nBegBufFile,m_nEndTimeFile,m_nEndBufFile;
    int *m_pnBegFile, *m_pnEndFile;
    int m_nFromIn, m_nToIn;
}STmEvPointers;


int ReadDataFromDCacheByEv2(
                       int a_nEventsMaxCount, struct19* a_pBufferForData,
                       int (*a_fpErrFunc)(const char*,...),
                       STmEvPointers* a_pPointers,
                       int a_nNumberOfSpectra, const BranchItem* a_pBranchItems,
                       void* a_pNotFoundedBranches = NULL,
                       char* a_pcBufferForNames = NULL,
                       int a_nBuffLen = 0,
                       void* a_pRootFiles = NULL);

int ReadDataFromDCache(int a_nEventsMaxCount, struct19* a_pBufferForData,
                       int (*a_fpErrFunc)(const char*,...),
                       int a_fromTime, int a_toTime,
                       int a_nNumberOfSpectra, const BranchItem* a_pBranchItems);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

class TmEvPointers : public STmEvPointers
{
    // Functions
public:
    TmEvPointers(SEARCH_TYPE_Type a_type, const int& a_beg, const int& a_end){
        m_cpnBegBufIn=NULL;m_cpnEndBufIn=NULL;m_cpnBegTimeIn=NULL;m_cpnEndTimeIn=NULL;
        SetType(a_type, a_beg, a_end);
    }

    void SetType(SEARCH_TYPE_Type a_type, const int& a_beg, const int& a_end){
        m_type = a_type;m_nFromIn = a_beg;m_nToIn=a_end;
        switch(a_type)
        {

        case SEARCH_TYPE_BY_EVENT:
            m_cpnBegBufIn = &a_beg;
            m_cpnEndBufIn = &a_end;
            m_nBegBufFile=a_beg;m_pnBegFile = &m_nBegBufFile; // Shold be redirected
            m_nEndBufFile=a_end;m_pnEndFile = &m_nEndBufFile; // Shold be redirected
            break;

        case SEARCH_TYPE_BY_TIME:
            m_cpnBegTimeIn = &a_beg;
            m_cpnEndTimeIn = &a_end;
            m_nBegTimeFile=a_beg; m_pnBegFile = &m_nBegTimeFile; // Shold be redirected
            m_nEndTimeFile=a_end;m_pnEndFile = &m_nEndTimeFile; // Shold be redirected
            break;

        default:
            break;
        }
    }

};

#endif  // #ifdef __cplusplus



#endif // BROWSING_FUNCS_H
