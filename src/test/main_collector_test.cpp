
#include <TROOT.h>
#include <TPluginManager.h>
#include <TFile.h>
#include <TTree.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#define TEST_ROOT_FLE_NAME      "test_root_file.root"
#define TEST_DAQ_ENTRY_NAME     "TEST_DAQ_NAME"

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
    int nFileSize;
    DaqDataStruct daqDataBuf;
    TFile* pRootFile = nullptr;// SetCompressionLevel(1)
    TTree* pRootTree = nullptr;
    TBranch *pBranchHeader, *pBranchData;

    gROOT->GetPluginManager()->AddHandler("TVirtualStreamerInfo",
                                          "*",
                                          "TStreamerInfo",
                                          "RIO",
                                          "TStreamerInfo()");

    pRootFile = new TFile(TEST_ROOT_FLE_NAME,"UPDATE","DATA",1);// SetCompressionLevel(1)
    if ( pRootFile->IsZombie() ){
        fprintf(stderr,"!!!! Error opening ROOT file %s. ln:%d\n",TEST_ROOT_FLE_NAME, __LINE__);
        goto returnPoint;
    }
    pRootFile->cd();gFile = pRootFile;

    pRootTree = new TTree(TEST_DAQ_ENTRY_NAME,"DATA");
    pBranchHeader=pRootTree->Branch("header",&daqDataBuf,TEST_ROOT_FORMAT_STRING_HEADER);
    pBranchData=pRootTree->Branch("data",&daqDataBuf.fData,TEST_ROOT_FORMAT_STRING_DATA);

    for(nIteration=0,nFileSize=0,daqDataBuf.eventNumber=0;nFileSize<1000000;++nIteration,++daqDataBuf.eventNumber){
        daqDataBuf.timeS= static_cast<int>(time(nullptr));
        daqDataBuf.fData = static_cast<float>(sin(static_cast<double>(daqDataBuf.timeS)/10.));
        //pRootTree->Fill();
        pBranchHeader->Fill();
        pBranchData->Fill();
        if(nIteration>10000){
            pRootTree->AutoSave("SaveSelf");
            nIteration = 0;
        }
        nFileSize = static_cast<int>(pRootFile->GetSize());
    }

    pRootFile->cd();gFile = pRootFile;
    pRootFile->TDirectory::DeleteAll();
    pRootFile->TDirectory::Close();
    delete pRootFile;

    nReturn = 0;
returnPoint:
    gROOT->GetPluginManager()->RemoveHandler("TVirtualStreamerInfo","*");
    return nReturn;
}
