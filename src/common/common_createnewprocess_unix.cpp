//
// file:        common_createnewprocess_unix.cpp
// created on:  2018 Nov 29
// created by:  D. Kalantaryan
//

#ifndef _WIN32

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

static const char s_cpcTerminatorString[]={' ','\n','\t','\0'};

int CreateProcessToDevNullAndWait(const char* a_cpcExecute, char* a_argv[])
{
    int nPid = fork();

    if(nPid){
        int nWaitResult, status;
        do {
            nWaitResult = waitpid(nPid, &status, WUNTRACED | WCONTINUED);
            if (nWaitResult == -1) {return -1;}

        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        return WEXITSTATUS(status);
    }
    else{
        freopen( "/dev/null", "r", stdin);
        freopen( "/dev/null", "w", stdout);
        freopen( "/dev/null", "w", stderr);
        execvp(a_cpcExecute,a_argv);
        return -1;
    }

    return 0;
}


#if 0
int SystemToDevNull(char* a_cpcExecuteAndCommands)
{
    size_t
    size_t unFoundIndex = strspn(a_cpcExecuteAndCommands,s_cpcTerminatorString);



    int nPid = fork();

    if(nPid){
        int nWaitResult, status;
        do {
            nWaitResult = waitpid(nPid, &status, WUNTRACED | WCONTINUED);
            if (nWaitResult == -1) {return -1;}

        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        return WEXITSTATUS(status);
    }
    else{


        freopen( "/dev/null", "r", stdin);
        freopen( "/dev/null", "w", stdout);
        freopen( "/dev/null", "w", stderr);
        execvp(a_cpcExecute,a_argv);
        return -1;
    }

    return 0;
}
#endif


#endif  // #ifdef _WIN32
