//
// file:        libsimple_oot_reader.cpp
// created on:  2018 Jun 08
// created by:  D. Kalantaryan (davit.kalantaryan@desy.de)
//

#include <TFile.h>
#include <TTree.h>
#include <stddef.h>
#include <stdlib.h>
#include <fstream>
#include <vector>
#include <string>
#include "TROOT.h"
#include "TPluginManager.h"
#include "tool/simple_root_reader.h"
#include "dcap.h"
#include <alloca.h>
#include <TKey.h>
#include <stdio.h>
#ifdef _WIN32
#else
#include <unistd.h>
#endif
#include "TLeaf.h"

//#define CALL_DESTRUCTOR


//#define ROOT_FILE_NAME_BASE_ACS "dcap://dcap:22125/pnfs/ifh.de"
//#define ROOT_FILE_NAME_BASE_LOC ""
#ifndef TMP_FILE_NAME
#define TMP_FILE_NAME   ".tmp.root"
#endif


int ReadOneRootFileToClbk(const char* a_cpcRootFileName, const char* a_cpcTreeAndBranchName,
                          structCommonNew* a_pStrForRoot,void* a_pOwner,TypeRtReader a_fpReader, TypeReport a_fpReport)
{
    const char* cpcFileName(a_cpcRootFileName);
    TBranch *pBranch;
    TTree* pTree;
    TFile* tFile = NULL ;
    structCommonNew* pStrForRoot=a_pStrForRoot;
    int nIndexEntry;
    int nReturn(-1), nNumOfEntries;
    int nDeleteFile = 0;
    callbackReturnType clbkReturn;


    nDeleteFile=0;
    if(strncmp(a_cpcRootFileName,"/acs/",5)==0){
        char vcTmpBuffer[1024];
        cpcFileName = TMP_FILE_NAME;
        snprintf(vcTmpBuffer,1023,"dccp %s " TMP_FILE_NAME, a_cpcRootFileName);
        if(system(vcTmpBuffer)!=0){
            (*a_fpReport)(a_pOwner,MARKED_ERROR " Unable to stat the root file %s\n",a_cpcRootFileName);
            return nReturn;
        }
        nDeleteFile = 1;
    }

    gROOT->GetPluginManager()->AddHandler("TVirtualStreamerInfo",
                                          "*",
                                          "TStreamerInfo",
                                          "RIO",
                                          "TStreamerInfo()");

    tFile = TFile::Open(cpcFileName);

    if(!tFile || !tFile->IsOpen()){
        (*a_fpReport)(a_pOwner,MARKED_ERROR " Unable to open root file %s\n",cpcFileName);
        if(tFile){
#ifdef CALL_DESTRUCTOR
            delete tFile;
#endif
            tFile=NULL;
        }
        goto returnPoint;
    }

    (*a_fpReport)(a_pOwner,"Root file (%s) open success ...!\n",a_cpcRootFileName);

    pTree = (TTree *)tFile->Get(a_cpcTreeAndBranchName);
    if(!pTree){
        (*a_fpReport)(a_pOwner,MARKED_ERROR " Tree (%s) not found in the file\n",a_cpcTreeAndBranchName);
        nReturn=2;goto returnPoint;
    }

    pBranch = pTree->GetBranch(a_cpcTreeAndBranchName);
    if(!pBranch){
        (*a_fpReport)(a_pOwner,MARKED_ERROR " Branch not found in the tree\n");
        nReturn=3;goto returnPoint;
    }

    nNumOfEntries = pBranch->GetEntries();
    (*a_fpReport)(a_pOwner,"nNumOfEntries = %d\n",nNumOfEntries);


    pBranch->SetAddress(pStrForRoot);

    for(nIndexEntry=0;nIndexEntry<nNumOfEntries;++nIndexEntry){
        pBranch->GetEntry(nIndexEntry);
        clbkReturn=(*a_fpReader)(a_pOwner,nIndexEntry,nNumOfEntries,&pStrForRoot);
        pBranch->SetAddress(pStrForRoot);
        if(clbkReturn!=callbackReturn_continue){break;}
    }

    nReturn = 0;
returnPoint:
    if(tFile){
        if(tFile->IsOpen()){tFile->Close();}
#ifdef CALL_DESTRUCTOR
        delete tFile;
#endif
    }

    if(nDeleteFile){
        remove(TMP_FILE_NAME);
    }

    return nReturn;
}


int GetAllEntriesInTheRootFile(const char* a_cpcRootFileName, void* a_pOwner,
                               TypeEntriesCountHandler a_fpEntriesCount,TypeBranchNameHandler a_brahNameHandler,TypeReport a_fpReport)
{
    const char* cpcFileName(a_cpcRootFileName);
    const char* cpcDataType;
    TKey* pKey;
    TObjLink* pNextLink;
    TList* pListOfKeys;
    TFile* tFile = NULL ;
    TBranch* pBranch;
    int nReturn(-1);
    int nDeleteFile = 0;
    int nContinue;
    int i,nNumberOfDaqEntries;
    pitz::daq::dataClient::StrForEntries aEntry;
    structCommonNew mem;

    nDeleteFile=0;
    if(strncmp(a_cpcRootFileName,"/acs/",5)==0){
        char vcTmpBuffer[1024];
        cpcFileName = TMP_FILE_NAME;
        snprintf(vcTmpBuffer,1023,"dccp %s " TMP_FILE_NAME, a_cpcRootFileName);
        if(system(vcTmpBuffer)!=0){
            (*a_fpReport)(a_pOwner,MARKED_ERROR " Unable to stat the root file %s\n",a_cpcRootFileName);
            return nReturn;
        }
        nDeleteFile = 1;
    }

    gROOT->GetPluginManager()->AddHandler("TVirtualStreamerInfo",
                                          "*",
                                          "TStreamerInfo",
                                          "RIO",
                                          "TStreamerInfo()");

    tFile = TFile::Open(cpcFileName);

    if(!tFile || !tFile->IsOpen()){
        (*a_fpReport)(a_pOwner,MARKED_ERROR " Unable to open root file %s\n",cpcFileName);
        if(tFile){
#ifdef CALL_DESTRUCTOR
            delete tFile;
#endif
            tFile=NULL;
        }
        goto returnPoint;
    }

    nNumberOfDaqEntries=tFile->GetNkeys();
    nContinue=(*a_fpEntriesCount)(a_pOwner,nNumberOfDaqEntries);
    if(!nContinue){goto returnPoint;}
    pListOfKeys = tFile->GetListOfKeys();

    pNextLink=pListOfKeys->FirstLink();
    i=0;
    while(pNextLink){
        pKey = (TKey*)pNextLink->GetObject();
        if(pKey){

            cpcDaqEntryName = pKey->GetName();
            pBranch = ((TTree*)pKey)->GetBranch(cpcDaqEntryName);
            if(!pBranch){goto nextLinkPoint;}

            aEntry.name = cpcDaqEntryName;
            aEntry.numberOfEntries = pBranch->GetEntries();
            cpcDataType=GetDataTypeAndCount(pBranch,&aEntry.info);
            if(aEntry.numberOfEntries>0){
                mem.setBranchInfo(aEntry.info);
                pBranch->SetAddress(mem.dataVoidPtr());
                pBranch->GetEntry(0);
                aEntry.firstEvent=mem.gen_event();aEntry.firstTime=mem.time();
                pBranch->GetEntry(aEntry.numberOfEntries-1);
                aEntry.lastEvent=mem.gen_event();aEntry.lastTime=mem.time();
            }

            nContinue=(*a_brahNameHandler)(a_pOwner,i,aEntry);
            if(!nContinue){break;}
        }
        nextLinkPoint:
        pNextLink = pNextLink->Next();
        ++i;

    }

    nReturn = 0;
returnPoint:
    if(tFile){
        if(tFile->IsOpen()){tFile->Close();}
#ifdef CALL_DESTRUCTOR
        delete tFile;
#endif
    }

    if(nDeleteFile){
        remove(TMP_FILE_NAME);
    }

    return nReturn;
}


TFile* OpenFileFromDcacheOrFromLocal2(const char* a_cpcRootFileName, int* a_pnDeleteFile, void* a_pOwner, TypeReport a_fpReport)
{
    // todo
    // dc_open should be used
    //int nDCacheFd;
    TFile*  tFile;
    const char* cpcFileName(a_cpcRootFileName);

    *a_pnDeleteFile=0;
    if(strncmp(a_cpcRootFileName,"/acs/",5)==0){
        char vcTmpBuffer[1024];
        cpcFileName = TMP_FILE_NAME;
        snprintf(vcTmpBuffer,1023,"dccp %s " TMP_FILE_NAME, a_cpcRootFileName);
        if(system(vcTmpBuffer)!=0){
            (*a_fpReport)(a_pOwner,MARKED_ERROR " Unable to stat the root file %s\n",a_cpcRootFileName);
            return NULL;
        }
#if 0
        nDCacheFd=dc_open(a_cpcRootFileName,O_RDONLY);
        (*a_fpReport)(a_pOwner,"++++++ nDCacheFd=%d\n",nDCacheFd);
        if(nDCacheFd){
            dc_close(nDCacheFd);
        }
#endif
        *a_pnDeleteFile = 1;
    }

    tFile = TFile::Open(cpcFileName);

    if(!tFile || !tFile->IsOpen()){
        (*a_fpReport)(a_pOwner,MARKED_ERROR " Unable to open root file %s\n",cpcFileName);
        if(tFile){
#ifdef CALL_DESTRUCTOR
            delete tFile;
#endif
            tFile=NULL;
        }
    }
    return tFile;
}


#include <vector>
#include <string>


const char* GetDataTypeAndCount(const TBranch* a_pBranch, structBranchDataInfo* a_pInfo)
{
    TLeaf* pLeaf ;
    const char* cpcTypeName="";

    pLeaf = a_pBranch->GetLeaf("int_value");
    if(pLeaf){goto finalizingInfo;}

    pLeaf = a_pBranch->GetLeaf("float_value");
    if(pLeaf){goto finalizingInfo;}

    pLeaf = a_pBranch->GetLeaf("array_value");
    if(pLeaf){goto finalizingInfo;}

    pLeaf = a_pBranch->GetLeaf("data");
    if(pLeaf){goto finalizingInfo;}

    a_pInfo->dataType = dataTypeErrorType;
    a_pInfo->itemsCount = 0;
    return "unknown";

finalizingInfo:
    cpcTypeName = pLeaf->GetTypeName();

    if(strcmp(cpcTypeName,"Float_t")==0){
        a_pInfo->dataType = dataTypeFloat;
    }
    else if(strcmp(cpcTypeName,"Int_t")==0){
        a_pInfo->dataType = dataTypeInt;
    }
    else{
        a_pInfo->dataType = dataTypeErrorType;
    }

    a_pInfo->itemsCount =  pLeaf->GetLen();
    return cpcTypeName;
}
