//#include "stdafx.h"
#include "createnewprocess1.h"

#ifdef __cplusplus
extern "C"
{
#endif


#include <stdio.h>

#ifdef WIN32
#include <windows.h>
#else
#include <memory.h>
#include <malloc.h>
#include <sys/wait.h>
#include <stdlib.h>
static inline int DumpArgs(int arg1,...)
{
    return arg1;
}
#endif


long WaitChildProcess1( long a_Process, void* a_pTime )
{
#ifdef WIN32

        long lnRet;

        if( a_pTime )
        {
                lnRet = (long)WaitForSingleObject( (HANDLE)a_Process, *((long*)a_pTime) );
        }
        else
        {
                lnRet = (long)WaitForSingleObject( (HANDLE)a_Process, INFINITE );
        }

        return lnRet;

#else

        /*int nStatus;

        if( a_pTime )
        {
                nStatus = *((int*)a_pTime);
        }
        else
        {
                nStatus = 1;
        }

        return (long)waitpid( (pid_t)a_Process, &nStatus, WEXITED );*/

        pid_t w;
        int status;
        int nIter(0);
        do{++nIter;w = waitpid((pid_t)a_Process, &status, WUNTRACED | WCONTINUED);
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

        return nIter;
        return DumpArgs(1,a_pTime);

#endif
}


#ifdef WIN32

void CloseHandles1( long a_lnProc, long a_lnThread )
{
        CloseHandle( (HANDLE)a_lnProc );
        CloseHandle( (HANDLE)a_lnThread );
}


long CreateNewProcess1(char* a_argv[], int a_nFD_Inp, int a_nFD_Out, int a_nFD_Err,
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



long CreateNewProcess2(const char* a_cpcExecute, int a_nFD_Inp, int a_nFD_Out, int a_nFD_Err,
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
        long lnPipe;

        /////////////////////////////////////////////
        STARTUPINFOA sInfo;
        PROCESS_INFORMATION pInfo;

        //Create input pipe
        if( (lnPipe =  _get_osfhandle(a_nFD_Inp)) == -1 )
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

#if 0

        if( !dwErrCode )
        {
                BOOL bRet = AllocConsole();

                dwErrCode = CreateProcessA(
                                                                "C:\\Windows\\System32\\cmd.exe",
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

                printf("Here\n");

                if( bRet )
                {
                        FreeConsole();
                }
        }

#endif


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
                *((long*)a_pReserved) = (long)pInfo.hThread;
        }


        return (long)pInfo.hProcess;
}

#else/*#ifdef WIN32*/

/*static int DumpArgs(int arg1, ...)
{
    return arg1;
}

void CloseHandles1( long arg1, long arg2)
{
    DumpArgs(arg1,arg2);
}

void AtExitDv()
{
        delete g_pTest;
        g_pTest = NULL;
}*/

void CloseHandles1( long , long  )
{
}


long CreateNewProcess2(const char* a_cpcExecute, int a_nFD_Inp, int a_nFD_Out, int a_nFD_Err,
                                           char* a_pcErrBuf, int a_nErrBufLen, void* a_pReserved )
{


        // Calculating number of arguments
        int i = 0, nStrLen = 0, nNumber = 0, nFirstTray = 0;
        int nFirstTry2 = 0,nOffSet = 0;
        while(a_cpcExecute[i])
        {
                if( a_cpcExecute[i] != ' ' )
                {
                        //++nStrLen;
                        nFirstTray = 0;
                }
                else
                {
                        if( !(nFirstTray++) )
                        {
                                //nStrLen = 0;
                                ++nNumber;
                        }
                }

                ++i;
        }

        char** argv = (char**)malloc( (2+nNumber)*sizeof(char*) );
        nNumber = 0;
        i = 0;


        while(a_cpcExecute[i])
        {
                if( a_cpcExecute[i] != ' ' )
                {
                        nFirstTray = 0;

                        if( !(nFirstTry2++) )
                        {
                                nOffSet = i;
                        }

                        ++nStrLen;
                }
                else
                {
                        nFirstTry2 = 0;

                        if( !(nFirstTray++) )
                        {
                                argv[nNumber] = (char*)malloc( nStrLen + 1 );
                                memcpy( argv[nNumber], a_cpcExecute + nOffSet, nStrLen );
                                (argv[nNumber])[nStrLen] = (char)0;
                                ++nNumber;
                                nStrLen = 0;
                                //
                        }

                }

                ++i;

        }


        argv[nNumber] = (char*)malloc( nStrLen+1 );
        memcpy( argv[nNumber], a_cpcExecute + nOffSet, nStrLen );
        (argv[nNumber])[nStrLen] = (char)0;
        ++nNumber;

        argv[nNumber] = NULL;

        //g_pTest = new Test;

        long lnRet = CreateNewProcess1( argv, a_nFD_Inp, a_nFD_Out, a_nFD_Err, a_pcErrBuf, a_nErrBufLen, a_pReserved );

        for( i = 0; i < nNumber; ++i )
        {
                free( argv[i] );
        }

        free( argv );

        return lnRet;

}



long CreateNewProcess1(char* a_argv[], int a_nFD_Inp, int a_nFD_Out, int a_nFD_Err,
                                           char* a_pcErrBuf, int a_nErrBufLen, void* )
{

        long lnRet;

        // Creating new process (child)
        lnRet = (long)fork();
        if( lnRet )
        {
                return lnRet;
        }

        //atexit( AtExitDv );

        //// Redirectiong standart streams
        int nOldInp, nOldOut, nOldErr;

        nOldInp = dup( 0 );
        dup2( a_nFD_Inp, 0 );

        nOldOut = dup( 1 );
        dup2( a_nFD_Out, 1 );

        nOldErr = dup( 2 );
        dup2( a_nFD_Err, 2 );

        //g_pTest = new Test;

        int nRet = execvp (a_argv[0], a_argv);

        if( nRet == -1 )
        {
                int nStatus;
                snprintf( a_pcErrBuf, a_nErrBufLen, "New process creation error!   errno = %d\n", errno );
                waitpid( (pid_t )lnRet, &nStatus, WEXITED );
                dup2(nOldInp,a_nFD_Inp);
                dup2(nOldOut,a_nFD_Out);
                dup2(nOldErr,a_nFD_Err);
                return 0;
        }

        return (long)0;

}


#endif/*#ifdef WIN32*/

#ifdef __cplusplus
}
#endif
