//#include "main_simple_root_reader_test.h"
//  Example of usage
//   ./simple_root_reader_test /doocs/data/DAQdata/daqL/pitznoadc0/PITZ_DATA.pitznoadc0.2019-02-25-0803.root CATH__NAME

#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

#include <TROOT.h>
#include <TPluginManager.h>
#include <TFile.h>
#include <TTree.h>
#include "test_collector_test_reader_common.h"
#include <iostream>


struct DaqDataStruct
{
    int   timeS;
    int   eventNumber;
    float fData;
};


int main(void)
{
    int nReturn = -1;
    int i, nNumOfEntries,nNumOfEntriesHeader,nNumOfEntriesData;
    TList* keyList;
    TFile* tFile = nullptr;
    TTree* pTree;
    TBranch *pBranchHeader, *pBranchData;
    TObjArray* pList;
    DaqDataStruct daqDataBuf;

    gROOT->GetPluginManager()->AddHandler("TVirtualStreamerInfo",
                                          "*",
                                          "TStreamerInfo",
                                          "RIO",
                                          "TStreamerInfo()");

    tFile = TFile::Open(TEST_ROOT_FILE_NAME);
    if(!tFile || !tFile->IsOpen()){
        goto returnPoint;
    }

    keyList=tFile->GetListOfKeys();
    ::std::cout << "keyList.length="<<keyList->GetEntries() << ::std::endl;
    pTree = static_cast<TTree *>(tFile->Get(TEST_DAQ_ENTRY_NAME));
    if(!pTree){
        goto returnPoint;
    }

    nNumOfEntries = static_cast<int>(pTree->GetEntries());

    //pBranch = pTree->GetBranch(DAQ_ENTRY_NAME); // old approach
    {// below is new approach
        pBranchHeader = nullptr;
        pBranchData = nullptr;
        pList = static_cast<TObjArray*>(pTree->GetListOfBranches());
        if(pList && (pList->GetEntries()>1)){
            pBranchHeader = static_cast<TBranch*>(pList->First());
            pBranchData = static_cast<TBranch*>(pList->At(1));
        }
    }
    if((!pBranchHeader)||(!pBranchData)){
        goto returnPoint;
    }

    nNumOfEntriesHeader = static_cast<int>(pBranchHeader->GetEntries());
    pBranchHeader->SetAddress(&daqDataBuf);

    nNumOfEntriesData = static_cast<int>(pBranchData->GetEntries());
    pBranchData->SetAddress(&daqDataBuf.fData);

    nNumOfEntries = nNumOfEntriesHeader<nNumOfEntriesData ? nNumOfEntriesHeader:nNumOfEntriesData;

    for(i=0;i<nNumOfEntries;++i){
        pBranchHeader->GetEntry(i);
        pBranchData->GetEntry(i);
    }

    nReturn =0;
returnPoint:
    if(tFile){tFile->Close();}
    gROOT->GetPluginManager()->RemoveHandler("TVirtualStreamerInfo","*");
    return nReturn;

}

