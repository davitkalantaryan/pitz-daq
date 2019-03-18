//
// file:        mex_simple_root_reader.cpp
//
// info:
//              https://stackoverflow.com/questions/19409573/qt-creator-stops-on-linux-signal
//              handle SIGSEGV nostop noprint pass   # this works
//


#include <mex.h>
#include <unistd.h>

static int s_nLibraryStarted = 0;


void mexFunction(int a_nNumOuts, mxArray *a_Outputs[],
    int a_nNumInps, const mxArray*a_Inputs[])
{

    if(!s_nLibraryStarted){
        mexPrintf("version 4.1.0. pid=%d\n",static_cast<int>(getpid()));
        //mexAtExit(&MexCleanupFunction);
        s_nLibraryStarted = 1;
    }

    mexPrintf("inputsNumber=%d, inp0=%p; outputsNumber=%d, out0=%p\n",a_nNumInps,a_Inputs,a_nNumOuts,a_Outputs);

}

