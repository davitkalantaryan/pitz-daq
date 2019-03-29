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
#include <thread>
#include <stdarg.h>

#define ROOT_FILE_NAME_BASE_ACS "dcap://dcap:22125/pnfs/ifh.de"
#define ROOT_FILE_NAME_BASE_LOC ""

#define TEST_FILE_NAME          "/doocs/data/DAQdata/daqL/pitznoadc0/PITZ_DATA.pitznoadc0.2019-02-25-0803.root"
#define TEST_BRANCH_NAME        "CATH__IGP3_P"


typedef struct struct19
{
        int       time;
        int       buffer;
        char      any_value[32768];
}struct19;

static int s_nLibraryStarted = 0;

static const char * argv[] = {
    "",
    TEST_FILE_NAME ,
    TEST_BRANCH_NAME
};
//static int argc = sizeof(argv)/sizeof(const char*) ;

void MexCleanupFunction(void);
void RootPrintf(const char* a_cpcFormat, ...);
void RootThreadFunction(void);

void mexFunction(int a_nNumOuts, mxArray *a_Outputs[],
    int a_nNumInps, const mxArray*a_Inputs[])
{

    if(!s_nLibraryStarted){
        mexPrintf("version 4.1.0. pid=%d\n",static_cast<int>(getpid()));
        mexAtExit(&MexCleanupFunction);
        //gROOT->GetPluginManager()->AddHandler()
        s_nLibraryStarted = 1;
    }

    mexPrintf("inputsNumber=%d, inp0=%p; outputsNumber=%d, out0=%p\n",a_nNumInps,a_Inputs,a_nNumOuts,a_Outputs);
    if(a_nNumInps>5){
        ::std::thread rootThread(RootThreadFunction);
        rootThread.join();
    }

}



void MexCleanupFunction(void)
{
    mexPrintf("Cleaning mex file!\n");
}


void RootThreadFunction(void)
{
#if 0
    const char* cpcErrorString="Unknown error";
    const char* cpcRootFileName;
    int nReturn = -1;
    TFile* tFile = nullptr;
    TTree* pTree;
    char vcBuffer2[1024];

    gROOT->GetPluginManager()->AddHandler("TVirtualStreamerInfo","*",
                                          "TStreamerInfo",
                                          "RIO",
                                          "TStreamerInfo()");

    if(strncmp(argv[1],"/acs/",5)==0){
        snprintf(vcBuffer2,1023,ROOT_FILE_NAME_BASE_ACS "%s",argv[1]);
        cpcRootFileName = vcBuffer2;
    }
    else{
        cpcRootFileName = argv[1];
    }


    tFile = TFile::Open(cpcRootFileName);
    RootPrintf("tFile=%p\n",tFile);
    if(!tFile || !tFile->IsOpen()){
        goto returnPoint;
    }
    RootPrintf("Root file %s opened succes....!\n",argv[1]);

    pTree = static_cast<TTree *>(tFile->Get(argv[2]));
    RootPrintf("pTree = %p\n",pTree);

    nReturn = 0;
returnPoint:
    //if(fpFileOut){fclose(fpFileOut);}
    //if(indexFileIn.is_open()){indexFileIn.close();}
    if(tFile){tFile->Close();}

    gROOT->GetPluginManager()->RemoveHandler("TVirtualStreamerInfo","*");

#if 0
void   AddHandler(const char *base, const char *regexp,
                  const char *className, const char *pluginName,
                  const char *ctor = 0, const char *origin = 0);
void   RemoveHandler(const char *base, const char *regexp = 0);
#endif

    if(nReturn){
        RootPrintf("Error accured (%s)\n",cpcErrorString);
    }
#endif
}


void RootPrintf(const char* a_cpcFormat, ...)
{
    va_list argList;

    va_start(argList,a_cpcFormat);
    vdprintf(STDOUT_FILENO,a_cpcFormat,argList);
    va_end(argList);
}
