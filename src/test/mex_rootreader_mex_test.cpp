//
// file:        mex_simple_root_reader.cpp
//
// info:
//              https://stackoverflow.com/questions/19409573/qt-creator-stops-on-linux-signal
//              handle SIGSEGV nostop noprint pass   # this works
//


#include <mex.h>
#include <iostream>
#include <stdio.h>
#include <TFile.h>
#include <TTree.h>
#include <stddef.h>
#include <stdlib.h>
#include <fstream>
#include <vector>
#include <string>
#include "TROOT.h"
#include "TPluginManager.h"

typedef struct struct19
{
        int       time;
        int       buffer;
        char      any_value[32768];
}struct19;

static int s_nLibraryStarted = 0;


void mexFunction(int a_nNumOuts, mxArray *a_Outputs[],
    int a_nNumInps, const mxArray*a_Inputs[])
{

    if(!s_nLibraryStarted){
        mexPrintf("version 4.1.0. pid=%d\n",static_cast<int>(getpid()));
        //mexAtExit(&MexCleanupFunction);
        gROOT->GetPluginManager()->AddHandler("TVirtualStreamerInfo",
                                              "*",
                                              "TStreamerInfo",
                                              "RIO",
                                              "TStreamerInfo()");
        s_nLibraryStarted = 1;
    }

    mexPrintf("inputsNumber=%d, inp0=%p; outputsNumber=%d, out0=%p\n",a_nNumInps,a_Inputs,a_nNumOuts,a_Outputs);



}

