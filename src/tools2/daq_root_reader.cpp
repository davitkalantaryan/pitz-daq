/*
 *  Copyright (C) DESY For details refers to LICENSE.md
 *
 *  Written by Davit Kalantaryan <davit.kalantaryan@desy.de>
 *
 */

/**
 *
 * @file       daq_root_reader.c
 * @copyright  Copyright (C) DESY For details refers to LICENSE.md
 * @brief      Header for providing API to comunicate with extensions
 * @author     Davit Kalantaryan <davit.kalantaryan@desy.de>
 * @date       2019 Sep 3
 * @details
 *   To be done \n
 *
 */
#include <mex_daq_browser2_common.h>
#include <daq_root_reader.hpp>
#include <pitz/daq/data/indexing.hpp>
#include <cpp11+/common_defination.h>
#include <string.h>
#include <stdlib.h>
#include <TFile.h>
#include <TTree.h>
#include "TROOT.h"
#include "TPluginManager.h"
#include <TKey.h>
#include "TLeaf.h"

#define MAKE_ERROR_THIS(...)
#define MAKE_REPORT_THIS(...)
#define MAKE_WARNING_THIS(...)

#define TMP_FILE_NAME       "tmp.root.file.root"



namespace pitz{ namespace daq{

struct STimeCompareData{
    time_t startTime, endTime;
};

namespace callbackReturn { enum Type{Collect,SkipThisEntry,StopThisBranch,StopForAll,Next}; }

typedef callbackReturn::Type (*TypeContinue)(void* clbkData, const data::memory::ForClient&);

static callbackReturn::Type TimeComparePrivate(void* a_pData,const data::memory::ForClient& a_memory);
static bool GetDataTypeAndCountStatic(const TBranch* a_pBranch, ::std::list< BranchOutputForUserInfo* >::iterator a_pOutBranchItem);
static int GetMultipleBranchesFromFileStatic( const char* a_rootFileName,
                                              ::std::list< BranchOutputForUserInfo* >* a_pOutputIn,
                                              ::std::list< BranchOutputForUserInfo* >* a_pOutputOut,
                                              TypeContinue a_fpContinue, void* a_pClbkData);



int RootInitialize(void)
{
    gROOT->GetPluginManager()->AddHandler("TVirtualStreamerInfo",
                                          "*",
                                          "TStreamerInfo",
                                          "RIO",
                                          "TStreamerInfo()");

    return 0;
}

void RootCleanup(void)
{
    //
}


int GetMultipleBranchesFromFile( const char* a_rootFileName, const ::std::list< BranchUserInputInfo >& a_Input, ::std::list< BranchOutputForUserInfo* >* a_pOutput)
{
    int nReturn;
    ::std::list< BranchUserInputInfo >::const_iterator pInpBranchItem(a_Input.begin()), pInpBranchItemEnd(a_Input.end());
    //::std::list< BranchOutputForUserInfo >::iterator pOutBranchItem;
    ::std::list< BranchOutputForUserInfo* > aOutputIn;
    BranchOutputForUserInfo* pOutData;

    //aOutputIn.resize(a_Input.size());
    for(;pInpBranchItem!=pInpBranchItemEnd;++pInpBranchItem){
        //(*pOutBranchItem).userClbk = &(*pInpBranchItem);
        pOutData = new BranchOutputForUserInfo;
        pOutData->userClbk = &(*pInpBranchItem);
        aOutputIn.push_back(pOutData);
    }

    nReturn = GetMultipleBranchesFromFileStatic(a_rootFileName,&aOutputIn,a_pOutput,
                                             [](void*,const data::memory::ForClient&){return callbackReturn::Collect;},nullptr);

    a_pOutput->splice(a_pOutput->end(),aOutputIn,aOutputIn.begin(),aOutputIn.end());
    return nReturn;
}


void GetMultipleBranchesForTime( time_t a_startTime, time_t a_endTime, const ::std::list< BranchUserInputInfo >& a_Input, ::std::list< BranchOutputForUserInfo* >* a_pOutput)
{
    ::std::list< BranchUserInputInfo >::const_iterator pInpBranchItem(a_Input.begin()), pInpBranchItemEnd(a_Input.end());
    //::std::list< BranchOutputForUserInfo >::iterator pOutBranchItem;
    ::std::list< BranchOutputForUserInfo* > aOutputIn;
    BranchOutputForUserInfo* pOutData;
    STimeCompareData aTmData({a_startTime,a_endTime});
    ::std::vector< ::std::string > vectFiles;
    BranchOutputForUserInfo* pActiveBrnch;
    size_t unNumberOfFiles, unFileIndex;

    //aOutputIn.resize(a_Input.size());
    for(;pInpBranchItem!=pInpBranchItemEnd;++pInpBranchItem){
        //(*pOutBranchItem).userClbk = &(*pInpBranchItem);
        pOutData = new BranchOutputForUserInfo;
        pOutData->userClbk = &(*pInpBranchItem);
        aOutputIn.push_back(pOutData);
    }

    while(aOutputIn.size()){
        pActiveBrnch = aOutputIn.front();
        vectFiles.clear();
        data::indexing::GetListOfFilesForTimeInterval(aOutputIn.front()->userClbk->branchName.c_str(),a_startTime,a_endTime,&vectFiles);
        unNumberOfFiles = vectFiles.size();
        for(unFileIndex=0;unFileIndex<unNumberOfFiles;++unFileIndex){
            GetMultipleBranchesFromFileStatic(vectFiles[unFileIndex].c_str(),&aOutputIn,a_pOutput,TimeComparePrivate,&aTmData);
        }

        if(pActiveBrnch == aOutputIn.front()){
            a_pOutput->splice(a_pOutput->end(),aOutputIn,aOutputIn.begin());
        }
    }


    //data::indexing::GetListOfFilesForTimeInterval()
    //nReturn = GetMultipleBranchesFromFileStatic(a_rootFileName,&aOutputIn,a_pOutput,
    //                                         [](void*,const data::memory::ForClient&){return callbackReturn::Collect;},nullptr);


    a_pOutput->splice(a_pOutput->end(),aOutputIn,aOutputIn.begin(),aOutputIn.end());
}



static callbackReturn::Type TimeComparePrivate(void* a_pData,const data::memory::ForClient& a_memory)
{
    STimeCompareData* pTmData = STATIC_CAST(STimeCompareData*,a_pData);
    time_t measureTime = a_memory.time();

    if(measureTime>pTmData->endTime){
        return callbackReturn::StopThisBranch;
    }
    else if(measureTime<pTmData->startTime){
        return callbackReturn::SkipThisEntry;
    }

    return callbackReturn::Collect;
}

static int GetMultipleBranchesFromFileStatic(
        const char* a_rootFileName,
        ::std::list< BranchOutputForUserInfo* >* a_pOutputIn,
        ::std::list< BranchOutputForUserInfo* >* a_pOutputOut,
        TypeContinue a_fpContinue, void* a_pClbkData)
{
    const char* cpcFileName(a_rootFileName);
    const char* cpcDaqEntryName;
    TBranch *pBranch;
    TTree* pTree;
    TFile* tFile  ;
    int64_t nIndexEntry;
    int64_t numberOfEntriesInTheFile;
    int nReturn(-1);
    int nDeleteFile = 0;
    //daq::callbackN::retType::Type clbkReturn(daq::callbackN::retType::Continue);
    //data::EntryInfo aBrInfo;
    //const char* cpcDataType;
    //::common::listN::ListItem<TBranchItemPrivate*> *pBranchItemNext, *pBranchItem;
    //memory::Base mem;
    ::std::list< BranchOutputForUserInfo* >::iterator pOutBranchItem, pOutBranchItemEnd, pOutBranchItemTmp;
    data::memory::ForClient aMemory(data::EntryInfoBase(),nullptr);
    data::memory::ForClient* pMemToAdd;
    callbackReturn::Type aClbkReturn;

    //if(this->m_clbkType != callbackN::Type::MultiEntries)
    //{
    //    MAKE_ERROR_THIS("Proper callback to retrive multiple entries is not set");
    //    return -1;
    //}
    MAKE_REPORT_THIS(2,"NumberOfBranches = %d",(int)a_pBranches->size());
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
            tFile=nullptr;
        }
        goto returnPoint;
    }

    MAKE_REPORT_THIS(1,"Root file (%s) open success ...!",a_rootFileName);


    pOutBranchItemEnd = a_pOutputIn->end();
    for(pOutBranchItem=a_pOutputIn->begin();pOutBranchItem!=pOutBranchItemEnd;){

nextBranchPreparedInAdvance:
        cpcDaqEntryName = (*pOutBranchItem)->userClbk->branchName.c_str();

        pTree = static_cast<TTree *>(tFile->Get(cpcDaqEntryName));
        if(!pTree){
            MAKE_REPORT_THIS(2,"Tree \"%s\" is not found", cpcDaqEntryName);
            goto nextBranchItem;
        }

        pBranch = pTree->GetBranch(cpcDaqEntryName);
        if(!pBranch){
            MAKE_WARNING_THIS("Branch \"%s\" is not found in the tree\n",cpcDaqEntryName);
            goto nextBranchItem;
        }

        numberOfEntriesInTheFile = pBranch->GetEntries();
        if( (numberOfEntriesInTheFile<1) || (!GetDataTypeAndCountStatic(pBranch,pOutBranchItem)) ){
            goto nextBranchItem;
        }

        MAKE_REPORT_THIS(2,"nNumOfEntries[%s] = %d, dataType:%s, itemsCount=%d",
                         cpcDaqEntryName, (*pOutBranchItem).numberOfEntries,
                         cpcDataType,aBrInfo.itemsCount);

        for(nIndexEntry=0;nIndexEntry<numberOfEntriesInTheFile;++nIndexEntry){
            aMemory.SetBranchInfo((*pOutBranchItem)->info);
            pBranch->SetAddress(aMemory.rawBuffer());
            pBranch->GetEntry(nIndexEntry);

            // namespace callbackReturn { enum Type{Collect,SkipThisEntry,StopThisBranch,StopForAll}; }
            aClbkReturn = a_fpContinue(a_pClbkData,aMemory);
            switch(aClbkReturn){
            case callbackReturn::Collect:
                pMemToAdd = new data::memory::ForClient(aMemory,(*pOutBranchItem));
                (*pOutBranchItem)->data.push_back(pMemToAdd);
                break;
            case callbackReturn::SkipThisEntry:
                continue;
            case callbackReturn::StopThisBranch:
                pOutBranchItemTmp = pOutBranchItem++;
                a_pOutputOut->splice(a_pOutputOut->end(),*a_pOutputIn,pOutBranchItemTmp);
                if(pOutBranchItem!=pOutBranchItemEnd){
                    goto nextBranchPreparedInAdvance;
                }
                else{
                    goto loopFinished;
                }

                //continue;
            case callbackReturn::StopForAll:
                nReturn = 0;
                goto returnPoint;
            default:
                break;
            }

        }

nextBranchItem:
        ++pOutBranchItem;

    }  // for(pOutBranchItem=a_pOutputIn->begin();pOutBranchItem!=pOutBranchItemEnd;){
loopFinished:

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

static bool GetDataTypeAndCountStatic(const TBranch* a_pBranch, ::std::list< BranchOutputForUserInfo* >::iterator a_pOutBranchItem)
{
    TLeaf* pLeaf ;
    const char* cpcTypeName=a_pBranch->GetClassName();
    TIter  aList = const_cast<TBranch*>(a_pBranch)->GetListOfLeaves();

    //qDebug()<<pList;

    pLeaf = a_pBranch->GetLeaf("IIII_array");
    if(pLeaf){cpcTypeName = "IIII_old";(*a_pOutBranchItem)->info.dataType = data::type::IIII_old;goto finalizeOldArrays;}

    pLeaf = a_pBranch->GetLeaf("IFFF_array");
    if(pLeaf){cpcTypeName = "IFFF_old";(*a_pOutBranchItem)->info.dataType = data::type::IFFF_old;goto finalizeOldArrays;}

    ++(++aList);  // skipping time and eventNumber
    if ((pLeaf = STATIC_CAST(TLeaf*,aList()))) {
        cpcTypeName = pLeaf->GetName();
        //qDebug()<<pLeaf<<cpcTypeName;
    }
    else{
        (*a_pOutBranchItem)->info.dataType = data::type::Error;
        (*a_pOutBranchItem)->info.itemsCountPerEntry = 0;
        (*a_pOutBranchItem)->info.dataTypeFromRoot = "unknown";
        return false;
    }

    (*a_pOutBranchItem)->info.itemsCountPerEntry = 0;

    //pLeaf = a_pBranch->GetLeaf("data");  // new approach
    //if(pLeaf){goto finalizingInfo;}
    //
    //pLeaf = a_pBranch->GetLeaf("float_value");
    //if(pLeaf){goto finalizingInfo;}
    //
    //pLeaf = a_pBranch->GetLeaf("int_value");
    //if(pLeaf){goto finalizingInfo;}
    //
    //pLeaf = a_pBranch->GetLeaf("array_value");
    //if(pLeaf){goto finalizingInfo;}
    //
    //pLeaf = a_pBranch->GetLeaf("char_array");
    //if(pLeaf){
    //    cpcTypeName = pLeaf->GetTypeName();
    //    (*a_pOutBranchItem)->info.itemsCountPerEntry =  pLeaf->GetLen();
    //    cpcTypeName = "string";
    //    (*a_pOutBranchItem)->info.dataType = data::type::String;goto finalizeCharArrays;
    //}
    //
    //pLeaf = a_pBranch->GetLeaf("IIII_array");
    //if(pLeaf){cpcTypeName = "IIII_old";(*a_pOutBranchItem)->info.dataType = data::type::IIII_old;goto finalizeCharArrays;}
    //
    //pLeaf = a_pBranch->GetLeaf("IFFF_array");
    //if(pLeaf){cpcTypeName = "IFFF_old";(*a_pOutBranchItem)->info.dataType = data::type::IFFF_old;goto finalizeCharArrays;}
    //
    //(*a_pOutBranchItem)->info.dataType = data::type::Error;
    //(*a_pOutBranchItem)->info.itemsCountPerEntry = 0;
    //(*a_pOutBranchItem)->info.dataTypeFromRoot = "unknown";
    //return false;

//finalizingInfo:
    cpcTypeName = pLeaf->GetTypeName();

    if(strcmp(cpcTypeName,"Float_t")==0){
        (*a_pOutBranchItem)->info.dataType = data::type::Float;
    }
    else if(strcmp(cpcTypeName,"Int_t")==0){
        (*a_pOutBranchItem)->info.dataType = data::type::Int;
    }
    else if(strcmp(cpcTypeName,"Char_t")==0){
        (*a_pOutBranchItem)->info.itemsCountPerEntry = 1;
        (*a_pOutBranchItem)->info.dataType = data::type::CharAscii;
    }
    else{
        (*a_pOutBranchItem)->info.dataType = data::type::Error;
        (*a_pOutBranchItem)->info.itemsCountPerEntry = 0;
        (*a_pOutBranchItem)->info.dataTypeFromRoot = "unknown";
        return false;
    }

    (*a_pOutBranchItem)->info.itemsCountPerEntry +=  pLeaf->GetLen();
    (*a_pOutBranchItem)->info.dataTypeFromRoot = cpcTypeName;
    return true;

finalizeOldArrays:
    (*a_pOutBranchItem)->info.itemsCountPerEntry =  1;
    return true;
}


}} // namespace pitz{ namespace daq{
