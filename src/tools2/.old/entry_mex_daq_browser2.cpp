/*
 *  Copyright (C) DESY For details refers to LICENSE.md
 *
 *  Written by Davit Kalantaryan <davit.kalantaryan@desy.de>
 *
 */

/**
 *
 * @file       entry_mex_daq_browser2.cpp
 * @copyright  Copyright (C) DESY For details refers to LICENSE.md
 * @brief      Header for providing API to comunicate with extensions
 * @author     Davit Kalantaryan <davit.kalantaryan@desy.de>
 * @date       2019 Sep 3
 * @details
 *   Include file declars symbols necessary for extending MATLAB emulator (method 1) \n
 *   No library is needed in order to use functionalities described in this header
 *
 */

//#include "common_daq_definations.h"
#include <matrix.h>
#include <stdio.h>
#include <mex.h>
#include <list>
#include "daq_root_reader.hpp"
#include <string.h>
#include <string>
#include <stdarg.h>

#ifdef printf
#undef printf
#endif


//void mexFunction (int a_nNumOuts, mxArray *a_Outputs[],int a_nNumInps, const mxArray*a_Inputs[]) __attribute__ ((unused));
//extern "C" int mexPrintf(const char	*/* printf style format */,.../* any additional arguments */)  __attribute__((weak));

void mexFunction(int a_nNumOuts, mxArray *a_Outputs[],int a_nNumInps, const mxArray*a_Inputs[])
{
    mexPrintf("version: 5. ni=%d, inps=%p, no=%d, outs=%p\n",a_nNumInps,a_Inputs,a_nNumOuts,a_Outputs);

}

extern "C" int printf(const char *a_format, ...)
{
    //vprintf(a_format);
    return 0;
}


int main(int a_argc, char* a_argv[])
{
    printf("argc=%d, argv=%p\n",a_argc,static_cast<void*>(a_argv));
    return 0;
}

//extern "C" int mexPrintf(const char	*/* printf style format */,.../* any additional arguments */)
//{
//    return 0;
//}




#if 0

//#include "common_daq_definations.h"
#include <matrix.h>
#include <stdio.h>
#include <mex.h>
#include <list>
#include "daq_root_reader.hpp"
#include <string.h>
#include <string>
#include <stdarg.h>

static mxArray* DataToMatlab( const ::std::list< pitz::daq::BranchOutputForUserInfo* >& a_data );

mxArray*  MatlabGetMultipleBranchesForTimeInterval(const char* a_argumentsLine)
{
    using namespace ::pitz::daq;
    ::std::list< BranchUserInputInfo > aInput;
    ::std::list< BranchOutputForUserInfo* > aOutput;
    const char* copaStr = strchr(a_argumentsLine,',');
    //int nIndex = a_argumentsLine.indexOf(QChar(','));
    size_t nIndex = copaStr ? static_cast<size_t>(copaStr-a_argumentsLine) : ::std::string::npos;
    //QString timeStartStr, timeEndStr;
    //QString branchName;
    //QString remainingLine;
    ::std::string timeStartStr, timeEndStr;
    ::std::string branchName;
    ::std::string remainingLine;
    time_t timeStart, timeEnd;

    if(nIndex==::std::string::npos){
        return NEWNULLPTR;
    }
    //timeStartStr = a_argumentsLine.left(nIndex).trimmed();
    //remainingLine = a_argumentsLine.mid(nIndex+1);
    timeStartStr = ::std::string(a_argumentsLine,static_cast<size_t>(nIndex));
    remainingLine = a_argumentsLine + nIndex + 1;

    //nIndex = remainingLine.indexOf(QChar(','));
    nIndex = remainingLine.find(',');
    if(nIndex==::std::string::npos){
        return NEWNULLPTR;
    }
    timeEndStr = remainingLine.substr(nIndex);
    remainingLine = remainingLine.substr(nIndex+1);

    timeStart = strtol(timeStartStr.c_str(),NEWNULLPTR,10);
    timeEnd = strtol(timeEndStr.c_str(),NEWNULLPTR,10);

    while(1){
        //nIndex = remainingLine.indexOf(QChar(','));
        nIndex = remainingLine.find(',');
        if(nIndex==::std::string::npos){
            break;
        }
        branchName = remainingLine.substr(nIndex);
        remainingLine = remainingLine.substr(nIndex+1);
        aInput.push_back(branchName);
    }

    aInput.push_back(remainingLine);

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

#endif
