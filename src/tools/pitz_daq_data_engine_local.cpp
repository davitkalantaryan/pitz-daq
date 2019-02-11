//
// file:        pitz_daq_dataclientlocl.cpp
// created on:  2018 Jun 08
// created by:  D. Kalantaryan (davit.kalantaryan@desy.de)
//

#include "pitz/daq/data/engine/local.hpp"

#include <TFile.h>
#include <TTree.h>
#include <stddef.h>
#include <stdlib.h>
#include <fstream>
#include <vector>
#include <string>
#include "TROOT.h"
#include "TPluginManager.h"
#include "dcap.h"
#include <alloca.h>
#include <TKey.h>
#include <stdio.h>
#ifdef _WIN32
#else
#include <unistd.h>
#endif
#include "TLeaf.h"
#include "pitz_daq_data_engine_branchitemprivate.hpp"

//#define CALL_DESTRUCTOR


//#define ROOT_FILE_NAME_BASE_ACS "dcap://dcap:22125/pnfs/ifh.de"
//#define ROOT_FILE_NAME_BASE_LOC ""
#ifndef TMP_FILE_NAME
#define TMP_FILE_NAME   ".tmp.root"
#endif

using namespace pitz::daq;


data::engine::Local::Local()
{
}


data::engine::Local::~Local()
{
}


int data::engine::Local::Initialize()
{
    gROOT->GetPluginManager()->AddHandler("TVirtualStreamerInfo",
                                          "*",
                                          "TStreamerInfo",
                                          "RIO",
                                          "TStreamerInfo()");

    return 0;
}


void data::engine::Local::Cleanup()
{
}


int data::engine::Local::GetEntriesInfo( const char* a_rootFileName)
{
    const char* cpcFileName(a_rootFileName);
    const char* cpcDaqEntryName;
    TKey* pKey;
    TObjLink* pNextLink;
    TList* pListOfKeys;
    TFile* tFile = NULL ;
    TBranch* pBranch;
    TTree* pTree;
    int nReturn(-1);
    int nDeleteFile = 0;
    daq::callbackN::retType::Type clbkReturn (daq::callbackN::retType::Continue);
    int i,nNumberOfDaqEntries;
    EntryInfoAdv aBrInfo;
    memory::Base mem;

    if(this->m_clbkType != callbackN::Type::Info){
        MAKE_ERROR_THIS("Proper callback to retrive all entries info is not set\n");
        return -1;
    }

    nDeleteFile=0;
    if(strncmp(a_rootFileName,"/acs/",5)==0){
        char vcTmpBuffer[1024];
        cpcFileName = TMP_FILE_NAME;
        snprintf(vcTmpBuffer,1023,"dccp %s " TMP_FILE_NAME, a_rootFileName);
        if(system(vcTmpBuffer)!=0){
            MAKE_ERROR_THIS("Unable to stat the root file %s\n",a_rootFileName);
            return nReturn;
        }
        nDeleteFile = 1;
    }

    tFile = TFile::Open(cpcFileName);

    if(!tFile || !tFile->IsOpen()){
        MAKE_ERROR_THIS("Unable to open root file %s\n",a_rootFileName);
        if(tFile){
#ifdef CALL_DESTRUCTOR
            delete tFile;
#endif
            tFile=NULL;
        }
        goto returnPoint;
    }

    nNumberOfDaqEntries=tFile->GetNkeys();
    clbkReturn=(*this->clbk.m_flInfo.numOfEntries)(m_clbkData,nNumberOfDaqEntries);
    if(clbkReturn!=daq::callbackN::retType::Continue){nReturn=0;goto returnPoint;}
    pListOfKeys = tFile->GetListOfKeys();

    pNextLink=pListOfKeys->FirstLink();
    i=0;
    while(pNextLink){
        pKey = (TKey*)pNextLink->GetObject();
        if(pKey){

            cpcDaqEntryName = pKey->GetName();

            //pTree = (TTree*)pKey;
            pTree = (TTree *)tFile->Get(cpcDaqEntryName);
            if(!pTree){goto nextLinkPoint;}

            pBranch = pTree->GetBranch(cpcDaqEntryName);
            if(!pBranch){goto nextLinkPoint;}

            aBrInfo.name = cpcDaqEntryName;
            aBrInfo.numberOfEntriesInTheFile = pBranch->GetEntries();
            GetDataTypeAndCount(pBranch,&aBrInfo);
            if(aBrInfo.numberOfEntriesInTheFile>0){
                mem.setBranchInfo(aBrInfo);
                pBranch->SetAddress(mem.rawBuffer());
                pBranch->GetEntry(0);
                aBrInfo.firstEvent=mem.gen_event();aBrInfo.firstTime=mem.time();
                pBranch->GetEntry(aBrInfo.numberOfEntriesInTheFile-1);
                aBrInfo.lastEvent=mem.gen_event();aBrInfo.lastTime=mem.time();
            }

            clbkReturn=(*this->clbk.m_flInfo.infoGetter)(m_clbkData,i,aBrInfo);
            if(clbkReturn!=daq::callbackN::retType::Continue){nReturn=0;goto returnPoint;}
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


int data::engine::Local::GetMultipleEntries( const char* a_rootFileName, ::common::List<TBranchItemPrivate*>* a_pBranches)
{
    const char* cpcFileName(a_rootFileName);
    TBranch *pBranch;
    TTree* pTree;
    TFile* tFile = NULL ;
    TBranchItemPrivate* pBranchRaw;
    int nIndexEntry;
    int nReturn(-1);
    int nDeleteFile = 0;
    daq::callbackN::retType::Type clbkReturn(daq::callbackN::retType::Continue);
    data::EntryInfo aBrInfo;
    const char* cpcDataType;
    ::common::listN::ListItem<TBranchItemPrivate*> *pBranchItemNext, *pBranchItem;
    memory::Base mem;
    bool bStopped = false;

    if(this->m_clbkType != callbackN::Type::MultiEntries){
        MAKE_ERROR_THIS("Proper callback to retrive multiple entries is not set");
        return -1;
    }
    MAKE_REPORT_THIS(2,"NumberOfBranches = %d",(int)a_pBranches->count());
    //if((int)a_pBranches->count()==0){goto returnPoint;}

    nDeleteFile=0;
    if(strncmp(a_rootFileName,"/acs/",5)==0){
        char vcTmpBuffer[1024];
        cpcFileName = TMP_FILE_NAME;
        snprintf(vcTmpBuffer,1023,"dccp %s " TMP_FILE_NAME, a_rootFileName);
        if(system(vcTmpBuffer)!=0){
            MAKE_ERROR_THIS("Unable to stat the root file %s\n",a_rootFileName);
            return nReturn;
        }
        nDeleteFile = 1;
    }


    tFile = TFile::Open(cpcFileName);

    if(!tFile || !tFile->IsOpen()){
        MAKE_ERROR_THIS("Unable to open root file %s\n",a_rootFileName);
        if(tFile){
#ifdef CALL_DESTRUCTOR
            delete tFile;
#endif
            tFile=NULL;
        }
        goto returnPoint;
    }

    MAKE_REPORT_THIS(1,"Root file (%s) open success ...!",a_rootFileName);

    pBranchItem = a_pBranches->first();
    while(pBranchItem){

        pBranchItemNext = pBranchItem->next;

        pTree = (TTree *)tFile->Get(pBranchItem->data->branchName.c_str());
        if(!pTree){
            MAKE_REPORT_THIS(2,"Tree \"%s\" is not found", pBranchItem->data->branchName.c_str());
            goto nextBranchItem;
        }

        pBranch = pTree->GetBranch(pBranchItem->data->branchName.c_str());
        if(!pBranch){
            MAKE_WARNING_THIS("Branch \"%s\" is not found in the tree\n",pBranchItem->data->branchName.c_str());
            goto nextBranchItem;
        }

        aBrInfo.numberOfEntriesInTheFile = pBranch->GetEntries();
        cpcDataType=GetDataTypeAndCount(pBranch,&aBrInfo);
        clbkReturn=(*this->clbk.m_multiEntries.infoGetter)(m_clbkData,pBranchItem->data->index,aBrInfo);
        if(clbkReturn==daq::callbackN::retType::StopForCurrent){
            pBranchRaw = pBranchItem->data;
            a_pBranches->RemoveData(pBranchItem);
            delete pBranchRaw;
            goto nextBranchItem;
        }
        else if(clbkReturn==daq::callbackN::retType::Stop){bStopped=true;nReturn=0;goto returnPoint;}

        mem.setBranchInfo(aBrInfo);
        pBranch->SetAddress(mem.rawBuffer());

        MAKE_REPORT_THIS(2,"nNumOfEntries[%s] = %d, dataType:%s, itemsCount=%d",
                         pBranchItem->data->branchName.c_str(), aBrInfo.numberOfEntriesInTheFile,
                         cpcDataType,aBrInfo.itemsCount);

        for(nIndexEntry=0;nIndexEntry<aBrInfo.numberOfEntriesInTheFile;++nIndexEntry){
            pBranch->GetEntry(nIndexEntry);
            clbkReturn=(*this->clbk.m_multiEntries.readEntry)(m_clbkData,pBranchItem->data->index,mem);
            if(clbkReturn==daq::callbackN::retType::StopForCurrent){
                pBranchRaw = pBranchItem->data;
                a_pBranches->RemoveData(pBranchItem);
                delete pBranchRaw;
                goto nextBranchItem;
            }
            else if(clbkReturn==daq::callbackN::retType::Stop){bStopped=true;nReturn=0;goto returnPoint;}
            //else {if(pFirstItem==pBranchItem){pFirstItem=NULL;}}
        }

nextBranchItem:
        pBranchItem = pBranchItemNext;
    }

    nReturn = 0;
returnPoint:
    if(bStopped){
        while(a_pBranches->first()){
            pBranchRaw = a_pBranches->first()->data;
            a_pBranches->RemoveData(a_pBranches->first());
            delete pBranchRaw;
        }
    }
    else{
        //if(pFirstItem){ a_pBranches->RemoveData(pFirstItem);}
    }

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


/*/////////////////////////////////////////////////////////////////////////////////////////////////*/

namespace pitz{ namespace daq { namespace data{ namespace engine{

const char* GetDataTypeAndCount(const TBranch* a_pBranch, data::EntryInfo* a_pInfo)
{
    TLeaf* pLeaf ;
    const char* cpcTypeName="";

    pLeaf = a_pBranch->GetLeaf("data");  // new approach
    if(pLeaf){goto finalizingInfo;}

    pLeaf = a_pBranch->GetLeaf("float_value");
    if(pLeaf){goto finalizingInfo;}

    pLeaf = a_pBranch->GetLeaf("int_value");
    if(pLeaf){goto finalizingInfo;}

    pLeaf = a_pBranch->GetLeaf("array_value");
    if(pLeaf){goto finalizingInfo;}

    pLeaf = a_pBranch->GetLeaf("char_array");
    if(pLeaf){cpcTypeName = "string";a_pInfo->dataType = Type::String;goto finalizeCharArrays;}

    pLeaf = a_pBranch->GetLeaf("IIII_array");
    if(pLeaf){cpcTypeName = "IIII_old";a_pInfo->dataType = Type::IIII_old;goto finalizeCharArrays;}

    pLeaf = a_pBranch->GetLeaf("IFFF_array");
    if(pLeaf){cpcTypeName = "IFFF_old";a_pInfo->dataType = Type::IFFF_old;goto finalizeCharArrays;}

    a_pInfo->dataType = Type::Error;
    a_pInfo->itemsCount = 0;
    return "unknown";

finalizingInfo:
    cpcTypeName = pLeaf->GetTypeName();

    if(strcmp(cpcTypeName,"Float_t")==0){
        a_pInfo->dataType = Type::Float;
    }
    else if(strcmp(cpcTypeName,"Int_t")==0){
        a_pInfo->dataType = Type::Int;
    }
    else{
        a_pInfo->dataType = Type::Error;
    }

    a_pInfo->itemsCount =  pLeaf->GetLen();
    return cpcTypeName;

finalizeCharArrays:
    a_pInfo->itemsCount =  1;
    return cpcTypeName;
}

}}}}  // namespace pitz{ namespace daq { namespace data{ namespace engine{
