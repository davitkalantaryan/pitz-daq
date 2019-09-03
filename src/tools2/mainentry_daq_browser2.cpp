/*
 *  Copyright (C) DESY For details refers to LICENSE.md
 *
 *  Written by Davit Kalantaryan <davit.kalantaryan@desy.de>
 *
 */

/**
 *
 * @file       mainentry_daq_browser2.c
 * @copyright  Copyright (C) DESY For details refers to LICENSE.md
 * @brief      Header for providing API to comunicate with extensions
 * @author     Davit Kalantaryan <davit.kalantaryan@desy.de>
 * @date       2019 Sep 3
 * @details
 *   Include file declars symbols necessary for extending MATLAB emulator (method 1) \n
 *   No library is needed in order to use functionalities described in this header
 *
 */

#include <mex_daq_browser2_common.h>
#include "daq_root_reader.hpp"
#include <string.h>
#include <utility>

#ifndef HANDLE_MEM_DEF
#define HANDLE_MEM_DEF(...)
#endif

static mxArray*  MatlabGetMultipleBranchesForTimeInterval2(int a_argc, char* a_argv[]);

int main(int a_argc, char* a_argv[])
{
    printf("argc=%d, argv=%p\n",a_argc,static_cast<void*>(a_argv));

    //if(a_argc>1){
    //    mxArray* pArray = nullptr;
    //    pitz::daq::RootInitialize();
    //    pArray = MatlabGetMultipleBranchesForTimeInterval2(a_argc,a_argv);
    //    printf("pArray=%p\n",static_cast<void*>(pArray));
    //    pitz::daq::RootCleanup();
    //}

    return 0;
}


static mxArray* DataToMatlab( const ::std::list< pitz::daq::BranchOutputForUserInfo* >& a_data );


static mxArray*  MatlabGetMultipleBranchesForTimeInterval2(int a_argc, char* a_argv[])
{
    using namespace ::pitz::daq;
    ::std::list< BranchUserInputInfo > aInput;
    ::std::list< BranchOutputForUserInfo* > aOutput;
    time_t timeStart, timeEnd;
    int i;

    if(a_argc<4){
        fprintf(stderr,"at least 3 arguments should be provided!\n");
        return nullptr;
    }


    for(i=3;i<a_argc;++i){
        aInput.push_back( ::std::string(a_argv[i]) );
    }

    timeStart = strtol(a_argv[1],NEWNULLPTR,10);
    timeEnd = strtol(a_argv[2],NEWNULLPTR,10);

    ::pitz::daq::GetMultipleBranchesForTime(timeStart,timeEnd,aInput,&aOutput);

    return DataToMatlab(aOutput);

}

static size_t InfoToMatlabRaw(mxClassID* a_pClsIdOfData,mxArray* a_pMatlabArray, size_t a_nIndex, int a_unFieldOffset,const pitz::daq::BranchOutputForUserInfo& a_info);

namespace glbDataFields{enum{Name,Data=3};}
namespace locDataFields{enum{Time,Event,Data};}
namespace baseInfoFields{enum{Type,itemsCount};}
namespace advInfoFields{enum{Name=0,numberOfEntriesInTheFile=3,firstTime,lastTime,firstEvent,lastEvent};}

static mxArray* DataToMatlab( const ::std::list< pitz::daq::BranchOutputForUserInfo* >& a_data )
{
    //Type::Type   dataType;int          itemsCount;  int          numberOfEntriesInTheFile;
    // struct EntryInfoAdv : data::EntryInfo{int firstTime,lastTime,firstEvent,lastEvent; ::std::string name; const int* ptr()const{return &firstTime;} int* ptr(){return &firstTime;}};
    static const char* svcpcFieldNamesGlb[] = {"name","type","itemsCount","data"};
    static const int scnNumberOfFieldsGlb = sizeof(svcpcFieldNamesGlb)/sizeof(const char*);
    static const char* svcpcFieldNames[] = {"time","event","data"};
    static const int scnNumberOfFields = sizeof(svcpcFieldNames)/sizeof(const char*);
    pitz::daq::data::memory::ForClient* pDataRaw;
    const size_t cunNumOfBranches(a_data.size());
    size_t nIndexBranch,nIndexEvent, nNumberOfEvents;
    size_t unItemSize;
    mxClassID clsId;
    mxArray *pNameGlb,*pDataGlb;
    mxArray *pTime, *pEvent,*pData;
    ::std::list< pitz::daq::BranchOutputForUserInfo* >::const_iterator pIter(a_data.begin()),pIterEnd(a_data.end());
    mxArray* pMatArray = mxCreateStructMatrix(cunNumOfBranches,1,scnNumberOfFieldsGlb,svcpcFieldNamesGlb);
    HANDLE_MEM_DEF(pMatArray,"No memory to create struct matrix");

    for( nIndexBranch=0;pIter != pIterEnd;++nIndexBranch,++pIter ){

        //nNumberOfEvents = a_data[nIndexBranch].entryData.size();
        nNumberOfEvents = (*pIter)->data.size();

        unItemSize=InfoToMatlabRaw(&clsId,pMatArray,nIndexBranch,1,*(*pIter));

        pNameGlb = mxCreateString((*pIter)->userClbk->branchName.c_str());HANDLE_MEM_DEF(pNameGlb," ");
        pDataGlb = mxCreateStructMatrix(nNumberOfEvents,1,scnNumberOfFields,svcpcFieldNames);HANDLE_MEM_DEF(pDataGlb," ");

        for(nIndexEvent=0; nIndexEvent<nNumberOfEvents;++nIndexEvent){
            pDataRaw = (*pIter)->data[nIndexEvent];
            pTime = mxCreateNumericMatrix(1,1,mxINT32_CLASS,mxREAL);HANDLE_MEM_DEF(pTime," ");
            *(STATIC_CAST(int32_t*,mxGetData(pTime)))=(*pIter)->data[nIndexEvent]->time();
            pEvent = mxCreateNumericMatrix(1,1,mxINT32_CLASS,mxREAL);HANDLE_MEM_DEF(pEvent," ");*(STATIC_CAST(int32_t*,mxGetData(pEvent)))=pDataRaw->gen_event();
            pData = mxCreateNumericMatrix(1,STATIC_CAST(size_t,(*pIter)->info.itemsCountPerEntry),clsId,mxREAL);HANDLE_MEM_DEF(pData," ");
            memcpy( mxGetData(pData),(*pIter)->data[nIndexEvent]->dataPtr<void>(),unItemSize* STATIC_CAST(size_t,(*pIter)->info.itemsCountPerEntry));

            mxSetFieldByNumber(pDataGlb,nIndexEvent,STATIC_CAST(size_t,locDataFields::Time),pTime);
            mxSetFieldByNumber(pDataGlb,nIndexEvent,STATIC_CAST(size_t,locDataFields::Event),pEvent);
            mxSetFieldByNumber(pDataGlb,nIndexEvent,STATIC_CAST(size_t,locDataFields::Data),pData);

        }

        mxSetFieldByNumber(pMatArray,nIndexBranch,STATIC_CAST(size_t,glbDataFields::Name),pNameGlb);
        mxSetFieldByNumber(pMatArray,nIndexBranch,STATIC_CAST(size_t,glbDataFields::Data),pDataGlb);

    }

    return pMatArray;
}


static size_t InfoToMatlabRaw(mxClassID* a_pClsIdOfData,mxArray* a_pMatlabArray, size_t a_nIndex, int a_unFieldOffset,const pitz::daq::BranchOutputForUserInfo& a_info)
{
    // time_field = mxGetFieldNumber(pDataGlb,svcpcFieldNames[0]);
    mxArray *pType, *pItemsCount;
    size_t unItemSize;
    mxClassID& clsId = *a_pClsIdOfData;

    switch(a_info.info.dataType){
    case pitz::daq::data::type::Int:
        pType = mxCreateString("Int_t");
        clsId = mxINT32_CLASS;
        unItemSize = 4;
        break;
    case pitz::daq::data::type::Float:
        pType = mxCreateString("Float_t");
        clsId = mxSINGLE_CLASS;
        unItemSize = 4;
        break;
    case pitz::daq::data::type::CharAscii:
        pType = mxCreateString("String");
        clsId = mxCHAR_CLASS;
        unItemSize = 1;
        break;
    case pitz::daq::data::type::IIII_old:
        pType = mxCreateString("IIII_old");
        clsId = mxSTRUCT_CLASS;
        unItemSize = 16;
        break;
    case pitz::daq::data::type::IFFF_old:
        pType = mxCreateString("IFFF_old");
        clsId = mxSTRUCT_CLASS;
        unItemSize = 16;
        break;
    default:
        pType = mxCreateString("Unknown");
        clsId = mxUNKNOWN_CLASS;
        unItemSize = 1;
        break;
    }
    HANDLE_MEM_DEF(pType,"No memory to create type string");

    pItemsCount = mxCreateNumericMatrix(1,1,mxINT32_CLASS,mxREAL);HANDLE_MEM_DEF(pItemsCount," ");*(STATIC_CAST(int32_t*,mxGetData( pItemsCount )))=a_info.info.itemsCountPerEntry;

    mxSetFieldByNumber(a_pMatlabArray,a_nIndex,a_unFieldOffset + STATIC_CAST(int,baseInfoFields::Type),pType);
    mxSetFieldByNumber(a_pMatlabArray,a_nIndex,a_unFieldOffset + STATIC_CAST(int,baseInfoFields::itemsCount),pItemsCount);

    return unItemSize;
}
