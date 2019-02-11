
#ifdef __cplusplus
extern "C"
{
#endif


#include <stdio.h>
#include <signal.h>

#ifdef _WIN32
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <windows.h>
#else
#include <memory.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdlib.h>
static inline int DumpArgs2(int arg1,...){return arg1;}
#endif

void KillProcess2(int a_signal, void* a_processHandle)
{
#ifdef _WIN32
#else
    kill((pid_t)((size_t)a_processHandle), a_signal);
#endif
}


int WaitChildProcess1( void* a_Process, void* a_pTime )
{
#ifdef WIN32

        int lnRet;

        if( a_pTime )
        {
                lnRet = (int)WaitForSingleObject( (HANDLE)a_Process, *((long*)a_pTime) );
        }
        else
        {
                lnRet = (int)WaitForSingleObject( (HANDLE)a_Process, INFINITE );
        }

        return lnRet;

#else

        pid_t w;
        int status;
        do{
            w = waitpid((pid_t)((size_t)a_Process),&status,WUNTRACED|WCONTINUED);
            if (w == -1) {
                perror("waitpid");
                return (EXIT_FAILURE);
            }

           if (WIFEXITED(status)) {
                //printf("exited, status=%d\n", WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                //printf("killed by signal %d\n", WTERMSIG(status));
            } else if (WIFSTOPPED(status)) {
                //printf("stopped by signal %d\n", WSTOPSIG(status));
            } else if (WIFCONTINUED(status)) {
                //printf("continued\n");
            }
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));

        return 0;
        return DumpArgs(1,a_pTime);

#endif
}


#ifdef _WIN32

void CloseHandles1( void* a_lnProc, void* a_lnThread )
{
        if(a_lnProc) CloseHandle( (HANDLE)a_lnProc );
        if(a_lnThread)CloseHandle( (HANDLE)a_lnThread );
}


void* CreateNewProcess1(char* a_argv[], int a_nFD_Inp, int a_nFD_Out, int a_nFD_Err,
                                           char* a_pcErrBuf, int a_nErrBufLen, void* a_pReserved )
{
        int argc = 0;
        char *pcTempBuff, *pcExecute = NULL;
        size_t unStrLen, unOffSet = 0, unBufLen = 0;

        while( a_argv[argc] )
        {
                unStrLen = strlen(a_argv[argc]);
                unBufLen += (unStrLen+1);
                pcTempBuff = (char*)realloc( pcExecute, unBufLen );

                if( !pcTempBuff )
                {
                        free( pcExecute );
                        _snprintf( a_pcErrBuf, a_nErrBufLen, "Not enough memory to create Execute buffer!\n" );
                        return 0L;
                }

                pcExecute = pcTempBuff;

                memcpy( pcExecute + unOffSet, a_argv[argc], unStrLen );
                pcExecute[unBufLen-1] = ' ';
                unOffSet = unBufLen;

                ++argc;

        }

        pcExecute[unBufLen-1] = (char)0;

        return CreateNewProcess2( pcExecute, a_nFD_Inp, a_nFD_Out, a_nFD_Err, a_pcErrBuf, a_nErrBufLen, a_pReserved);
}



void* CreateNewProcess2(const char* a_cpcExecute, int a_nFD_Inp, int a_nFD_Out, int a_nFD_Err,
                                           char* a_pcErrBuf, int a_nErrBufLen, void* a_pReserved )
{

        /*SECURITY_ATTRIBUTES secattr;
        ZeroMemory(&secattr,sizeof(secattr));
        secattr.nLength = sizeof(secattr);
        secattr.bInheritHandle = TRUE;

        HANDLE rPipeInp, wPipeInp, rPipeOut, wPipeOut, rPipeErr, wPipeErr;
        //HANDLE rPipe, wPipe;
        DWORD dwErrCode;

        //Create input pipes
        if( !CreatePipe(&rPipeInp,&wPipeInp,&secattr,0) )
        {
                dwErrCode = GetLastError();
                //fprintf( stderr, "Input pipes creation error!   Error = %d\n", (int)dwErrCode );

                FormatMessageA(	FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                                NULL, dwErrCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                                                a_pcErrBuf, a_nErrBufLen,
                                                NULL);

                return 0L;
        }*/

        DWORD dwErrCode;

        //////////////////////////////////////////////////////////////////////
        ////// Creating system pipes from input file descriptors  ////////////
        //////////////////////////////////////////////////////////////////////
        HANDLE rPipeInp, wPipeOut, wPipeErr;
		u_int64_ttt lnPipe;

        /////////////////////////////////////////////
        STARTUPINFOA sInfo;
        PROCESS_INFORMATION pInfo;

        //Create input pipe
        if( (lnPipe =  (u_int64_ttt)_get_osfhandle(a_nFD_Inp)) == -1 )
        {

                _snprintf( a_pcErrBuf, a_nErrBufLen, "Input file handle is invalid!   errno = %d\n", (int)errno );

                /*dwErrCode = GetLastError();
                FormatMessageA(	FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                                NULL, dwErrCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                                                a_pcErrBuf, a_nErrBufLen,
                                                NULL);*/

                return 0L;
        }
        rPipeInp = (HANDLE)lnPipe;


        //Create output pipe
        if( (lnPipe =  _get_osfhandle(a_nFD_Out)) == -1 )
        {
                _snprintf( a_pcErrBuf, a_nErrBufLen, "Output file handle is invalid!   errno = %d\n", (int)errno );
                return 0L;
        }
        wPipeOut = (HANDLE)lnPipe;


        //Create error pipe
        if( (lnPipe =  _get_osfhandle(a_nFD_Err)) == -1 )
        {
                _snprintf( a_pcErrBuf, a_nErrBufLen, "Error file handle is invalid!   errno = %d\n", (int)errno );
                return 0L;
        }
        wPipeErr = (HANDLE)lnPipe;


        //STARTUPINFOA sInfo;
        ZeroMemory(&sInfo,sizeof(sInfo));
        sInfo.dwFlags=STARTF_USESTDHANDLES;
        sInfo.hStdInput=rPipeInp;
        sInfo.hStdOutput=wPipeOut;
        sInfo.hStdError=wPipeErr;

        //PROCESS_INFORMATION pInfo;
        ZeroMemory(&pInfo,sizeof(pInfo));

        dwErrCode = CreateProcessA(
                                                                NULL,
                                                                //const_cast<char*>(a_cpcExecute),
                                                                (char*)a_cpcExecute,
                                                                NULL,
                                                                NULL,
                                                                TRUE,
                                                                NORMAL_PRIORITY_CLASS|CREATE_NO_WINDOW,
                                                                NULL,
                                                                NULL,
                                                                &sInfo,
                                                                &pInfo
                                                                );



        if( !dwErrCode )
        {
                dwErrCode = GetLastError();
                //fprintf( stderr, "Process creation error!   Error = %d\n", dwErrCode );

                FormatMessageA(	FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                                NULL, dwErrCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                                                a_pcErrBuf, a_nErrBufLen,
                                                NULL);

                return 0L;
        }


        if( a_pReserved )
        {
                *((u_int64_ttt*)a_pReserved) = (u_int64_ttt)pInfo.hThread;
        }


        return (void*)pInfo.hProcess;
}

#else/*#ifdef WIN32*/

void CloseHandles1( void* , void*  )
{
}


struct SResourceForCrtPrc{
    size_t maxNumberOfArgs;
    char** ppArgv;
    int*   sizes;
};


void* GetResourceForCreateProcess(void)
{
    struct SResourceForCrtPrc* pResource = (struct SResourceForCrtPrc*)malloc(sizeof(struct SResourceForCrtPrc));
    if(!pResource){return NULL;}
    pResource->maxNumberOfArgs = 0;
    pResource->ppArgv = NULL;
    pResource->sizes = NULL;
    return pResource;
}

void InitCreateProcessResourse(void* a_pResourse)
{
    size_t i=0;
    struct SResourceForCrtPrc* pResource=(struct SResourceForCrtPrc*)a_pResourse;
    free(pResource->sizes);pResource->sizes=NULL;
    for(;i<pResource->maxNumberOfArgs;++i){free(pResource->ppArgv[i]);}
    free(pResource->ppArgv);pResource->ppArgv=NULL;
    pResource->maxNumberOfArgs = 0;
}


void FreeCreateProcessResource(void* a_pResource)
{
    InitCreateProcessResourse(a_pResource);
    free(a_pResource);
}

static int ResizeResourse(
        struct SResourceForCrtPrc* a_pResourse, size_t a_newSize)
{
    if(a_newSize>a_pResourse->maxNumberOfArgs){
        size_t i=a_pResourse->maxNumberOfArgs;
        a_pResourse->ppArgv = (char**)realloc(a_pResourse->ppArgv,a_newSize*sizeof(char*));
        if(!a_pResourse->ppArgv){InitCreateProcessResourse(a_pResourse);return -1;}
        a_pResourse->sizes = (int*)realloc(a_pResourse->sizes,a_newSize*sizeof(int));
        if(!a_pResourse->sizes){InitCreateProcessResourse(a_pResourse);return -2;}
        for(;i<a_newSize;++i){a_pResourse->ppArgv[i] = NULL;}
        a_pResourse->maxNumberOfArgs = a_newSize;
    }
    return 0;
}


static int ResizeArgument(struct SResourceForCrtPrc* a_pResourse,
                          size_t a_index,int a_bufLenPlus1)
{
    if(a_pResourse->sizes[a_index]<a_bufLenPlus1){
        a_pResourse->ppArgv[a_index]=(char*)realloc(a_pResourse->ppArgv[a_index],a_bufLenPlus1);
        if(!a_pResourse->ppArgv[a_index]){a_pResourse->sizes[a_index]=0;return -1;}
    }
    return 0;
}


void* CreateNewProcess4(const char* a_cpcExecute, int a_nFD_Inp, int a_nFD_Out, int a_nFD_Err,
                        char* a_pcErrBuf, int a_nErrBufLen, void* a_pResource)
{
    // '`'
    const char *cpcNext(a_cpcExecute), *cpcNextTmp;
    struct SResourceForCrtPrc* pResourse = (struct SResourceForCrtPrc*)a_pResource;
    int i, nStrLen1, nNumber(0), nNumberMinus1;

    while((*cpcNext == ' ')||(*cpcNext == '\t'))++cpcNext;

    while(cpcNext && *cpcNext){
        ++nNumber;
        // cpcNextTmp=strpbrk(cpcNext," \t");  // which one is fast
        cpcNextTmp=strchr(cpcNext,' ');
        if(!cpcNextTmp){cpcNextTmp=strchr(cpcNext,'\t');if(!cpcNextTmp){break;}}
        cpcNext = cpcNextTmp;
        while((*cpcNext == ' ')||(*cpcNext == '\t'))++cpcNext;
    }

    if(!nNumber){return NULL;}

    if(ResizeResourse( pResourse,2+nNumber)){return NULL;}
    nNumberMinus1 = nNumber-1;

    for(i=0,cpcNext=a_cpcExecute; i<nNumberMinus1;++i){
        while((*cpcNext == ' ')||(*cpcNext == '\t'))++cpcNext;
        cpcNextTmp=strchr(cpcNext,' ');if(!cpcNextTmp){cpcNextTmp=strchr(cpcNext,'\t');}
        nStrLen1 = (int)((size_t)cpcNextTmp - (size_t)cpcNext);
        if(ResizeArgument(pResourse,i,nStrLen1+1)){return NULL;}
        memcpy(pResourse->ppArgv[i],cpcNext,nStrLen1);
        (pResourse->ppArgv[i])[nStrLen1]=0;
        cpcNext = cpcNextTmp;
    }

    while((*cpcNext == ' ')||(*cpcNext == '\t'))++cpcNext;
    nStrLen1=strlen(cpcNext);
    if(ResizeArgument(pResourse,nNumberMinus1,nStrLen1+1)){return NULL;}
    memcpy(pResourse->ppArgv[nNumberMinus1],cpcNext,nStrLen1);
    (pResourse->ppArgv[nNumberMinus1])[nStrLen1]=0;

    pResourse->ppArgv[nNumber]=NULL;pResourse->ppArgv[nNumber+1]=NULL;

    void* lnRet = CreateNewProcess1( pResourse->ppArgv, a_nFD_Inp, a_nFD_Out, a_nFD_Err, a_pcErrBuf, a_nErrBufLen, NULL );

    return lnRet;
}


void* CreateNewProcess3(const char* a_cpcExecute, int a_nFD_Inp, int a_nFD_Out, int a_nFD_Err,
                        char* a_pcErrBuf, int a_nErrBufLen, void* a_pResource )
{
        // Calculating number of arguments
        const char *cpcNext(a_cpcExecute), *cpcNextTmp;
        struct SResourceForCrtPrc* pResourse = (struct SResourceForCrtPrc*)a_pResource;
        int i, nStrLen1, nNumber(0), nNumberMinus1;

        while((*cpcNext == ' ')||(*cpcNext == '\t'))++cpcNext;

        while(cpcNext && *cpcNext){
            ++nNumber;
            // cpcNextTmp=strpbrk(cpcNext," \t");  // which one is fast
            cpcNextTmp=strchr(cpcNext,' ');
            if(!cpcNextTmp){cpcNextTmp=strchr(cpcNext,'\t');if(!cpcNextTmp){break;}}
            cpcNext = cpcNextTmp;
            while((*cpcNext == ' ')||(*cpcNext == '\t'))++cpcNext;
        }

        if(!nNumber){return NULL;}

        if(ResizeResourse( pResourse,2+nNumber)){return NULL;}
        nNumberMinus1 = nNumber-1;

        for(i=0,cpcNext=a_cpcExecute; i<nNumberMinus1;++i){
            while((*cpcNext == ' ')||(*cpcNext == '\t'))++cpcNext;
            cpcNextTmp=strchr(cpcNext,' ');if(!cpcNextTmp){cpcNextTmp=strchr(cpcNext,'\t');}
            nStrLen1 = (int)((size_t)cpcNextTmp - (size_t)cpcNext);
            if(ResizeArgument(pResourse,i,nStrLen1+1)){return NULL;}
            memcpy(pResourse->ppArgv[i],cpcNext,nStrLen1);
            (pResourse->ppArgv[i])[nStrLen1]=0;
            cpcNext = cpcNextTmp;
        }

        while((*cpcNext == ' ')||(*cpcNext == '\t'))++cpcNext;
        nStrLen1=strlen(cpcNext);
        if(ResizeArgument(pResourse,nNumberMinus1,nStrLen1+1)){return NULL;}
        memcpy(pResourse->ppArgv[nNumberMinus1],cpcNext,nStrLen1);
        (pResourse->ppArgv[nNumberMinus1])[nStrLen1]=0;

        pResourse->ppArgv[nNumber]=NULL;pResourse->ppArgv[nNumber+1]=NULL;

        void* lnRet = CreateNewProcess1( pResourse->ppArgv, a_nFD_Inp, a_nFD_Out, a_nFD_Err, a_pcErrBuf, a_nErrBufLen, NULL );

        return lnRet;

}



void* CreateNewProcess1(char* a_argv[], int a_nFD_Inp, int a_nFD_Out, int a_nFD_Err,
                                           char* a_pcErrBuf, int a_nErrBufLen, void* )
{

        void* lnRet;
        int nRet;

        lnRet = (void*)((size_t)fork());
        if( lnRet ){return lnRet;}

        //// Redirectiong standart streams
        int nOldInp, nOldOut, nOldErr;

        nOldInp = dup( 0 );
        dup2( a_nFD_Inp, 0 );

        nOldOut = dup( 1 );
        dup2( a_nFD_Out, 1 );

        nOldErr = dup( 2 );
        dup2( a_nFD_Err, 2 );

        nRet = execvp (a_argv[0], a_argv);

        if( nRet == -1 ){
                int nStatus;
                snprintf( a_pcErrBuf, a_nErrBufLen, "New process creation error!   errno = %d\n", errno );
                waitpid( (pid_t )((size_t)lnRet), &nStatus, WEXITED );
                dup2(nOldInp,a_nFD_Inp);
                dup2(nOldOut,a_nFD_Out);
                dup2(nOldErr,a_nFD_Err);
                return 0;
        }

        return (void*)0;

}


#include <unistd.h>
#include <stdio.h>

int NewSystemToDevNULL(const char* a_cpcExecute, const char* a_argv[])
{
    int nPid = fork();

    if(nPid){
        int nWaitResult;
        do {
            nWaitResult = waitpid(cpid, &status, WUNTRACED | WCONTINUED);
            if (nWaitResult == -1) {return -1;}

        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        return WEXITSTATUS(status);
    }
    else{
        freopen( "/dev/null", "r", stdin);
        freopen( "/dev/null", "w", stdout);
        freopen( "/dev/null", "w", stderr);
        execvp(a_cpcExecute,argv);
        return -1;
    }

    return 0;
}


#endif/*#ifdef _WIN32*/

#ifdef __cplusplus
}
#endif
