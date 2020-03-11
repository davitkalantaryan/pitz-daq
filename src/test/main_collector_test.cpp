
#include <TROOT.h>
#include <TPluginManager.h>
#include <TFile.h>
#include <TTree.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "test_collector_test_reader_common.h"

#define TEST_ROOT_FORMAT_STRING_HEADER  "seconds/I:gen_event/I"
#define TEST_ROOT_FORMAT_STRING_DATA    "data[1]/F"
#define TEST_ROOT_FORMAT_STRING2        TEST_ROOT_FORMAT_STRING_HEADER ":" TEST_ROOT_FORMAT_STRING_DATA

struct DaqDataStruct
{
    int   timeS;
    int   eventNumber;
    float fData;
};

int main()
{
    int nReturn = -1;
    int nIteration;
    int nFileSize, nFileSizePrev=0;
    //DaqDataStruct daqDataBuf;
    DaqDataStruct* pDaqDataBuf;
    TFile* pRootFile = nullptr;// SetCompressionLevel(1)
    TTree* pRootTree = nullptr;
    TBranch *pBranchHeader, *pBranchData;

    gROOT->GetPluginManager()->AddHandler("TVirtualStreamerInfo",
                                          "*",
                                          "TStreamerInfo",
                                          "RIO",
                                          "TStreamerInfo()");

    pRootFile = new TFile(TEST_ROOT_FILE_NAME,"UPDATE","DATA",1);// SetCompressionLevel(1)
    if ( pRootFile->IsZombie() ){
        fprintf(stderr,"!!!! Error opening ROOT file %s. ln:%d\n",TEST_ROOT_FILE_NAME, __LINE__);
        goto returnPoint;
    }
    pRootFile->cd();gFile = pRootFile;

    pRootTree = new TTree(TEST_DAQ_ENTRY_NAME,"DATA");
    pBranchHeader=pRootTree->Branch("header",nullptr,TEST_ROOT_FORMAT_STRING_HEADER);
    pBranchData=pRootTree->Branch("data",nullptr,TEST_ROOT_FORMAT_STRING_DATA);

    for(nIteration=0,nFileSize=0;nFileSize<1000000;++nIteration){

        pDaqDataBuf = new DaqDataStruct;
        pDaqDataBuf->timeS= static_cast<int>(time(nullptr));
        pDaqDataBuf->eventNumber = nIteration;
        pDaqDataBuf->fData = static_cast<float>(sin(static_cast<double>(pDaqDataBuf->timeS)/10.));

        pBranchHeader->SetAddress(pDaqDataBuf);
        pBranchData->SetAddress(&pDaqDataBuf->fData);

        pRootTree->Fill();

        delete pDaqDataBuf;

        nFileSize = static_cast<int>(pRootFile->GetSize());
        if(nFileSizePrev!=nFileSize){
            printf("%.5d  => fileSize=%d\n",nIteration,nFileSize);
            nFileSizePrev = nFileSize;
        }
    }

    pRootTree->AutoSave("SaveSelf");
    pRootFile->TDirectory::DeleteAll();
    pRootFile->TDirectory::Close();
    delete gFile;

    nReturn = 0;
returnPoint:
    gROOT->GetPluginManager()->RemoveHandler("TVirtualStreamerInfo","*");
    return nReturn;
}
