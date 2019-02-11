#ifndef COMMONHEADER_ROOT_H
#define COMMONHEADER_ROOT_H

#define SHARED_MEM_NAME     "shared_for_root_transfer"
#define SHARED_MEM_NAME2    "shared_for_root_transfer2"
#define _NUMBER_OF_FLOATS_  2048

#define _GET_INDEX_(spectIndex,entryIndex,totalNumbOfEntries)       \
                    (totalNumbOfEntries)*(spectIndex)+(entryIndex)
#define _SIZE_OF_SHARED_(nNumberOfSpectra,cnNumberOfEvents)         \
                    (sizeof(struct19)*(nNumberOfSpectra)*(cnNumberOfEvents)+sizeof(int))

#include <string.h>
#define _FILE_PATH(__full_path) (  strrchr((__full_path),'/') ? (strrchr((__full_path),'/')+1) : \
                    ( strrchr((__full_path),'\\') ? (strrchr((__full_path),'\\')+1) : (__full_path) )  )

extern int MakeLogging(const char* callerFile, int line, const char* callerFnc,const char* format,...); // printf formatted
extern long GetLogCurrentPos();
extern int SetLogCurrentPos(long offset);
extern int fwriteToLog(const void* buffer, int buf_len);
extern int TruncateLog(long a_size);
extern int g_nLoglevel;
#if 1
#define _DEBUG_APP_(__log_level__,...) \
    do{ \
        if((__log_level__)<=g_nLoglevel) { \
            printf("fl:\"%s\",ln:%d,fnc:%s:  ",_FILE_PATH(__FILE__),__LINE__,__FUNCTION__); \
            printf(__VA_ARGS__); printf("\n"); \
        } \
    }while(0)
#else  // #if 0/1
#define _DEBUG_APP_(__log_level__,...) \
    do{ \
        if((__log_level__)<=g_nLoglevel) { \
            MakeLogging(_FILE_PATH(__FILE__),__LINE__,__FUNCTION__,":  "); \
            MakeLogging(NULL,0,NULL,__VA_ARGS__); MakeLogging(NULL,0,NULL,"\n"); \
        } \
    }while(0)
#endif // #if 0/1
#define MAKE_LOGGING(...) \
    do{ \
        MakeLogging(_FILE_PATH(__FILE__),__LINE__,__FUNCTION__,__VA_ARGS__);}while(0)

#include <time.h>
#define _MAX_NORMAL_TIME_SEC_   200
//#define _FNC_START(...) time_t tStartTime; time(&tStartTime)
#define _FNC_START(...) \
    time_t tStartTime = time(NULL);\
    long _lnFilePos = GetLogCurrentPos();\
    int _nWritten = MakeLogging(_FILE_PATH(__FILE__),__LINE__,__FUNCTION__,__VA_ARGS__)
#define _FNC_END(...) \
    do{ \
        time_t tEndTime; time(&tEndTime);\
        if( (tEndTime-tStartTime)>_MAX_NORMAL_TIME_SEC_ ){ \
            MakeLogging(NULL,0,NULL,"\n"); \
            MAKE_LOGGING(__VA_ARGS__);MakeLogging(NULL,0,NULL,"  elapsedTime=%d\n",(int)(tEndTime-tStartTime));\
        }else{\
            /*for(int __i(0);__i<_nWritten;++__i){MakeLogging(NULL,0,NULL,"\b");}*/\
            /*char __cWrite(0);SetLogCurrentPos(_lnFilePos);*/\
            /*for(int __i(0);__i<_nWritten;++__i){fwriteToLog(&__cWrite,1);}*/\
            /*SetLogCurrentPos(_lnFilePos);*/TruncateLog(_lnFilePos);\
        } \
    }while(0)

typedef struct struct19
{
        int       time;
        int       buffer;
        float     array_value[2048];
}struct19;

typedef struct BranchItem
{
    int m_nIndex;
    int m_nBegFounded;
    const char* m_cpcBranchName;
    const char* m_cpcTreeName;
}BranchItem;


#endif // COMMONHEADER_ROOT_H
