
#include <stdio.h>
#include <stdlib.h>
#include "browsing_funcs.h"
#include "shared_memory_clt.h"
#ifdef WIN32
#include <conio.h>
#endif

#include <time.h>
#include <stdarg.h>

#include <alog.h>

static ALog s_logFile(1000000000);


int main(int argc, const char* argv[])
{
#if 0
    //s_logFile.SetOutputPtr(stdout);
    s_logFile.Open("/doocs/data/lililog/lililog.txt");
    _FNC_START("argc=%d argv[0]=%s, argv[1]=%s, argv[2]=%s",argc,argv[0],argv[1]?argv[1]:"null",argv[2]?argv[2]:"null");
    usleep(6000000);
    if(argc>2){usleep(5000000);}
    //MAKE_LOGGING("argc=%d argv[0]=%s, argv[1]=%s, argv[2]=%s",argc,argv[0],argv[1]?argv[1]:"null",argv[2]?argv[2]:"null");
    _FNC_END("argc=%d argv[0]=%s, argv[1]=%s, argv[2]=%s",argc,argv[0],argv[1]?argv[1]:"null",argv[2]?argv[2]:"null");
    return 0;
#endif
#if 0
    time_t aTime = time(NULL);
    printf("time1=%d\t",(int)aTime);
    usleep(5000000);
    time(&aTime);
    printf("time2=%d\n",(int)aTime);
    return 0;
#endif

    if( argc<5 )
    {
        fprintf(stderr,"Input arguments number is to less!\n");
        return 1;
    }

#ifdef WIN32
    _getch();
#endif

    char* pcBuffer;
    int* pnReaded;
    int i;
    //int nCreatedInside;

    int nOption = atoi(argv[1]);
    int nFrom = atoi(argv[2]);
    int nTo = atoi(argv[3]);
    //int nNumbOfEvents = toEvent - fromEvent + 1;
    int nNumbOfEvents2;
    int nNumberOfSpectra = argc - 4;

    s_logFile.Open("/doocs/data/lililog/lililog.txt");

    char* pcDbgEnv = getenv("LILI_DEBUG");
    g_nLoglevel = pcDbgEnv ? atoi(pcDbgEnv) : 0;

    BranchItem* pBranchItems = (BranchItem*)malloc(sizeof(BranchItem)*nNumberOfSpectra);
    for(i = 0; i<nNumberOfSpectra;++i)
    {
        pBranchItems[i].m_nIndex = i;
        pBranchItems[i].m_nBegFounded = 0;
        pBranchItems[i].m_cpcBranchName = argv[4+i];
        pBranchItems[i].m_cpcTreeName = pBranchItems[i].m_cpcBranchName;
    }

    //struct19* pBufferForData = (struct19*)malloc(sizeof(struct19)*nNumberOfSpectra*nNumbOfEvents);
    struct19* pBufferForData;
    Shared_Memory_Clt aShared;
    if(aShared.CreateShrdMem(SHARED_MEM_NAME)<=0)
    {

#ifdef WIN32
        HANDLE nMapFile;
#else
        int nMapFile;
#endif
        void* pBuffer;

        nNumbOfEvents2 = nOption ? ((nTo-nFrom)*10 + 1) : (nTo - nFrom + 1);

        size_t unSize = _SIZE_OF_SHARED_(nNumberOfSpectra,nNumbOfEvents2);
        fprintf(stdout,"Shared memory doesn't exist! New one will be created\n");
        if(Shared_Memory_Base::createSharedMemory(&nMapFile,&pBuffer,unSize,SHARED_MEM_NAME2,1)<=0)
        {
            fprintf(stderr,"Couldn't create shared memory!\n");
            return 1;
        }
        //pBufferForData = (struct19*)pBuffer;
        pcBuffer = (char*)pBuffer;
        //nCreatedOutside = 0;
    }
    else
    {
        //pBufferForData = (struct19*)aShared.GetMemPtr();
        pcBuffer = (char*)aShared.GetMemPtr();
        //nCreatedOutside = 1;
    }

    pnReaded = (int*)pcBuffer;
    pBufferForData = (struct19*)(pcBuffer + sizeof(int));

    switch(nOption)
    {
    case 0:
    {
        TmEvPointers aPointers(SEARCH_TYPE_BY_EVENT,nFrom,nTo);
        nNumbOfEvents2 = nTo - nFrom + 1;
        *pnReaded = ReadDataFromDCacheByEv2(nNumbOfEvents2,pBufferForData,printf,&aPointers,
                                            nNumberOfSpectra,pBranchItems,NULL,NULL,0);
        break;
    }

    case 1:
    {
        nNumbOfEvents2 = (nTo-nFrom)*10 + 1;
        *pnReaded = ReadDataFromDCache(nNumbOfEvents2,pBufferForData,printf,nFrom,nTo,nNumberOfSpectra,pBranchItems);
        break;
    }

    default:
        break;
    }


    free(pBranchItems);
    aShared.Close2();
    //printf("nReaded = %d\n\n",nReaded);
    //close(STDOUT_FILENO);
    //fprintf(stderr,"STDOUT_FILENO = %d\n",STDOUT_FILENO);
    return 0;
}


int MakeLogging(const char* a_callerFile, int a_nLine, const char* a_callerFnc,
                const char* a_format,...)
{
    int nRet;
    va_list args;
    va_start(args,a_format);
    if(a_callerFile){nRet = s_logFile.WritePrvt2(a_callerFile,a_nLine,a_callerFnc, a_format,args);}
    else{nRet = s_logFile.WriteWTSPrvt(a_format,args);}
    va_end(args);

    return nRet;
}

//long GetLogCurrentPos(){return s_logFile.FilePos();}
long GetLogCurrentPos(){return s_logFile.GetLogSize();}
int SetLogCurrentPos(long a_offset){return s_logFile.SetFilePos(a_offset);}
int fwriteToLog(const void* buffer, int buf_len){return (int)s_logFile.FWrite(buffer,buf_len);}

int TruncateLog(long a_size){return s_logFile.TruncateLog(a_size);}
