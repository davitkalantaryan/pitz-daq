//#include "main_simple_root_reader_test.h"


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

#define ROOT_FILE_NAME_BASE_ACS "dcap://dcap:22125/pnfs/ifh.de"
#define ROOT_FILE_NAME_BASE_LOC ""

//#define DAQ_ENTRY_NAME  "GUN__COUPLER__PMT_20140905"
//#define ROOT_FILE_NAME  "/acs/pitz/daq/2017/10/pitzdiag.adc/PITZ_DATA.pitzdiag.adc.2017-10-01-0100.root","READ"


int main(int argc, char* argv[])
{
    //char vcFileNameBuffer[1024];
    //snprintf(a_pcFinalRootFileName,a_nFinalRtFlNmLen,"dcap://dcap:22125/pnfs/ifh.de%s",a_cpcDebugRootFlName);

    const char* cpcEntry;
    char* pcNext;
    std::ifstream indexFileIn;
    FILE* fpFileOut(NULL);
    TBranch *pBranch;
    TTree* pTree;
    TFile* tFile ;
    struct19 aStrForRootIn,aStrForRootFn;
    std::vector<std::string> vLastEntries;
    size_t i,unVectorSize;
    int nReturn(0), nNumOfEntries;
    int nTimeSeconds;
    int nOffset;
    int nSeekReturn;
    char vcBuffer[1024], vcBufferToAdd[1024],vcIndexFileNameBuffer[1024];
    bool bFound;

    //printf("Press any key to continue!\n");
    //getchar();

    if(argc<3){
        std::cerr<<"Too less arguments! ... "<<std::endl;
        return 1;
    }

    gROOT->GetPluginManager()->AddHandler("TVirtualStreamerInfo",
                                          "*",
                                          "TStreamerInfo",
                                          "RIO",
                                          "TStreamerInfo()");

    if(strncmp(argv[1],"/acs/",5)==0){
        snprintf(vcBuffer,1023,ROOT_FILE_NAME_BASE_ACS "%s",argv[1]);
    }
    else{
        snprintf(vcBuffer,1023,ROOT_FILE_NAME_BASE_LOC "%s",argv[1]);
    }


    tFile = TFile::Open(vcBuffer);

    if(!tFile || !tFile->IsOpen()){
        // report
        std::cerr<<"Unable to open root file"<<std::endl;
        return 1;
    }

    std::cout<< "Root file opened succes....!"<<std::endl;

    //tFile->ls("-d");
    //gFile = tFile;

    pTree = (TTree *)tFile->Get(argv[2]);
    printf("pTree = %p\n",pTree);
    if(!pTree){
        std::cerr<<"Tree not found in the file"<<std::endl;
        nReturn=2;goto returnPoint;
    }

    pBranch = pTree->GetBranch(argv[2]);
    printf("pBranch = %p\n",pBranch);
    if(!pBranch){nReturn=3;goto returnPoint;}

    nNumOfEntries = pBranch->GetEntries();
    printf("nNumOfEntries = %d\n",nNumOfEntries);

    pBranch->SetAddress(&aStrForRootIn);
    pBranch->GetEntry(0);

    pBranch->SetAddress(&aStrForRootFn);
    pBranch->GetEntry(nNumOfEntries-1);

    snprintf(vcBufferToAdd,1023,"%d:%d,%d:%d,%s",
             aStrForRootIn.time,aStrForRootIn.buffer,aStrForRootFn.time,aStrForRootFn.buffer,argv[1]);

    printf("%s\n",vcBufferToAdd);

    snprintf(vcIndexFileNameBuffer,1023,"/doocs/data/DAQdata/INDEX/%s.idx",argv[2]);

    // TO FIND
    //indexFile.open(vcIndexFileNameBuffer,std::ios_base::in|std::ios_base::out|std::ios::app);
    indexFileIn.open(vcIndexFileNameBuffer);

    if(!indexFileIn.is_open()){
        std::cerr<<"Unable to open index file"<<vcIndexFileNameBuffer<<std::endl;
        nReturn = 3;
        goto returnPoint;
    }

    bFound = false;
    nOffset = indexFileIn.tellg();
    while(!indexFileIn.fail()){
        indexFileIn.getline(vcBuffer,1023);
        if(strncmp(vcBufferToAdd,vcBuffer,1023)==0){
            printf("%s root file is already in the %s index file!\n",argv[1],vcIndexFileNameBuffer);
            goto returnPoint;
        }
        nTimeSeconds = (int)strtol(vcBuffer,&pcNext,10);
        if(nTimeSeconds>aStrForRootIn.time){
            vLastEntries.push_back(vcBuffer); bFound=true;break;
        }
        nOffset = indexFileIn.tellg();
    }

    while(!indexFileIn.fail()){
        indexFileIn.getline(vcBuffer,1023);
        vLastEntries.push_back(vcBuffer);
    }

    indexFileIn.close();

    fpFileOut = fopen(vcIndexFileNameBuffer,"r+");

    if(!fpFileOut){
        std::cerr<<"Unable to open index file for write!"<<std::endl;
        nReturn = 5;
        goto returnPoint;
    }

    if(!bFound){
        nSeekReturn = fseek(fpFileOut,0,SEEK_END);
        if(nSeekReturn){
            std::cerr<<"Unable to seek, error code="<<nSeekReturn<<"\n";
            nReturn = 7;
            goto returnPoint;
        }
        fprintf(fpFileOut,"%s\n",vcBufferToAdd);
        goto returnPoint;
    }

    nSeekReturn = fseek(fpFileOut,nOffset,SEEK_SET);
    if(nSeekReturn){
        std::cerr<<"Unable to seek, error code="<<nSeekReturn<<"\n";
        nReturn = 7;
        goto returnPoint;
    }
    fprintf(fpFileOut,"%s\n",vcBufferToAdd);


    unVectorSize = vLastEntries.size();
    for(i=0;i<unVectorSize;++i){
        cpcEntry = vLastEntries[i].c_str();
        //indexFileOut.write(cpcEntry,vLastEntries[i].length());
        //indexFileOut.write("\n",1);
        fprintf(fpFileOut,"%s\n",cpcEntry);
    }

returnPoint:
    if(fpFileOut){fclose(fpFileOut);}
    if(indexFileIn.is_open()){indexFileIn.close();}
    if(tFile){tFile->Close();}

    return nReturn;
}

