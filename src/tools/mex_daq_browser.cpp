//
// file:        mex_simple_root_reader.cpp
//

#include "common_daq_definations.h"
#include <mex.h>
#include <string.h>
#include "pitz/daq/base.hpp"
#include "pitz/daq/data/engine/bypipe.hpp"
#include "pitz/daq/data/getter/fromsocket.hpp"
#include "pitz/daq/data/getter/frompipe.hpp"
#include "pitz/daq/data/getter/tobuffer.hpp"

#define HELPER_BINARY_NAME  "bindaq_browser_actual"

using namespace pitz;
using namespace pitz::daq;

#ifdef _WIN32
typedef data::getter::FromSocket  GetterType;
#else
typedef data::getter::FromPipe  GetterType;
#endif

static int s_nLibraryStarted = 0;
static pthread_t    s_mainThreadHandle = (pthread_t)0;
static int s_nPid = 0;
static data::getter::ToBuffer<GetterType>   s_dataGetter;

static void PrintHelp(void);
static void MexCleanupFunction(void);
static double GetNumericData(const mxArray* a_numeric);
static mxArray* AdvInfoToMatlab(const ::std::vector< data::engine::EntryInfoAdv >& a_info);
static mxArray* DataToMatlab(const ::std::vector< ::std::string >& a_daqNames, const ::std::vector< data::getter::EntryInfoAdvTI >& a_data);
static bool GetBranchNamesFromMatlabCell(const mxArray* a_pMatlabCell, ::std::vector< ::std::string >* a_pVectorOfBrNames);
static int ReportFunction(const char* a_cpcFormatString, ...);
static int ErrorFunction(const char* a_cpcFormatString, ...);


void mexFunction(int a_nNumOuts, mxArray *a_Outputs[],
    int a_nNumInps, const mxArray*a_Inputs[])
{
    int nStrLen, nReturn;
    char *pcOption;

    if(!s_nLibraryStarted){
        s_nPid = (int)getpid();
        s_mainThreadHandle = pthread_self();
        log::g_globalLogger = ReportFunction;
        log::g_globalErrorLogger = ErrorFunction;
        //log::g_globalLogger = mexPrintf;
        //log::g_globalErrorLogger = mexPrintf;
        MAKE_REPORT_GLOBAL(0,"version 4.1.0. pid=%d, tid=%d",s_nPid,(int)((size_t)s_mainThreadHandle));
        mexAtExit(&MexCleanupFunction);
        s_nLibraryStarted = 1;
#ifdef USE_SOCKET
		s_dataGetter.connect("pi4-vm2");
#endif
    }

    if( (a_nNumInps<1)||(!mxIsChar(a_Inputs[0])) ){
        MAKE_ERROR_GLOBAL("Provide the option!");
        PrintHelp();
        goto returnPoint;
    }


    nStrLen=(int)(mxGetM(a_Inputs[0])*mxGetN(a_Inputs[0]));
    pcOption=(char*)alloca(nStrLen+4);
    mxGetString(a_Inputs[0],pcOption,nStrLen+1);

    if(strcmp("--help",pcOption)==0){PrintHelp();goto returnPoint;}
    else if(strcmp("--get-log-level",pcOption)==0){
        mexPrintf("log-level=%d\n",(int)pitz::daq::log::g_nLogLevel);
        goto returnPoint;
    }
    else if(strcmp("--set-log-level",pcOption)==0){
        if((a_nNumInps<2)||(!mxIsNumeric(a_Inputs[1]))){
            mexPrintf("logLevel is not provided!\n");
            goto returnPoint;
        }
        pitz::daq::log::g_nLogLevel= (ptrdiff_t)GetNumericData(a_Inputs[1]);
        goto returnPoint;
    }

    a_Outputs[0] = NULL;

    if(strcmp("--get-file-entries",pcOption)==0){
        char* pcRootFileName;
        const ::std::vector< data::engine::EntryInfoAdv >& info = s_dataGetter.Info();
        MAKE_REPORT_GLOBAL(1,"option: --get-file-entries");

        if( (a_nNumInps<2)||(!mxIsChar(a_Inputs[1]))  ){
            MAKE_ERROR_GLOBAL("Name of the root file should be provided!");
            goto returnPoint;
        }

        nStrLen=(int)(mxGetM(a_Inputs[1])*mxGetN(a_Inputs[1]));
        pcRootFileName=(char*)alloca(nStrLen+2);
        mxGetString(a_Inputs[1],pcRootFileName,nStrLen+1);

        nReturn=s_dataGetter.GetEntriesInfo(pcRootFileName);
        MAKE_REPORT_GLOBAL(2,"return from dataGetter.GetMultipleEntries is %d",nReturn);
        if(nReturn<0){goto returnPoint;}
        a_Outputs[0] = AdvInfoToMatlab(info);
    }
    else if(strcmp("--get-data-for-entries",pcOption)==0){
        char* pcRootFileName;
        ::std::vector< ::std::string > vectorBranches;
        const ::std::vector< data::getter::EntryInfoAdvTI >& data = s_dataGetter.Data();
        MAKE_REPORT_GLOBAL(1,"option: --get-data-for-entries");

        if( (a_nNumInps<3) || (!mxIsChar(a_Inputs[1])) || (!GetBranchNamesFromMatlabCell(a_Inputs[2],&vectorBranches))  ){
            MAKE_ERROR_GLOBAL("Name of the root file and the names of entries should be provided!");
            goto returnPoint;
        }

        nStrLen=(int)(mxGetM(a_Inputs[1])*mxGetN(a_Inputs[1]));
        pcRootFileName=(char*)alloca(nStrLen+2);
        mxGetString(a_Inputs[1],pcRootFileName,nStrLen+1);

        nReturn=s_dataGetter.GetMultipleEntries(pcRootFileName,vectorBranches);
        if(nReturn<0){goto returnPoint;}
        MAKE_REPORT_GLOBAL(2,"return from dataGetter.GetMultipleEntries is %d",nReturn);
        a_Outputs[0] = DataToMatlab(vectorBranches,data);

    }
    else if(strcmp("--get-data-for-time-interval",pcOption)==0){
        int nStartTime, nEndTime;
        ::std::vector< ::std::string > vectorBranches;
        const ::std::vector< data::getter::EntryInfoAdvTI >& data = s_dataGetter.Data();
        MAKE_REPORT_GLOBAL(1,"option: --get-data-for-time-interval");

        if( (a_nNumInps<4) || (!mxIsNumeric(a_Inputs[2])) || (!mxIsNumeric(a_Inputs[3])) || (!GetBranchNamesFromMatlabCell(a_Inputs[1],&vectorBranches))  ){
            MAKE_ERROR_GLOBAL("Name of the root file and the names of entries should be provided!");
            goto returnPoint;
        }

        nStartTime = (int)GetNumericData(a_Inputs[2]);
        nEndTime = (int)GetNumericData(a_Inputs[3]);

        nReturn=s_dataGetter.GetMultipleEntriesTI(vectorBranches,nStartTime,nEndTime);
        if(nReturn<0){goto returnPoint;}
        a_Outputs[0] = DataToMatlab(vectorBranches,data);
    }
    else{
        MAKE_ERROR_GLOBAL("Wrong option provided!");
        PrintHelp();
        goto returnPoint;
    }

returnPoint:
    if((!a_Outputs[0])&&(a_nNumOuts>0)){a_Outputs[0]=mxCreateString("Unknown error accured!\n");}
    return;
}


static void PrintHelp(void)
{
    mexPrintf(
                "--help\n"
                "--get-file-entries fileName\n"
                "--get-data-for-entries fileName entryNamesVector\n"
                "--get-data-for-time-interval entryNamesVector time1 time2\n"
                "--set-log-level verbosity\n"
                "--get-log-level \n");
}


static void MexCleanupFunction(void)
{
    if(s_nLibraryStarted){
#ifdef USE_SOCKET
		s_dataGetter.disconnet();
#endif
        s_mainThreadHandle = (pthread_t)0;
    }
    s_nLibraryStarted = 0;
}


static int ReportFunction(const char* a_cpcFormatString, ...)
{
    int nReturn;
    pthread_t  threadHandle = pthread_self();
    int nPid = (int)getpid();

    if(s_mainThreadHandle && (threadHandle==s_mainThreadHandle) && (nPid==s_nPid)){
        char vcReportBuffer[1024];
        va_list argsList;

        va_start(argsList,a_cpcFormatString);
        nReturn=vsnprintf(vcReportBuffer,1024,a_cpcFormatString,argsList);
        va_end(argsList);
        mexPrintf("%s",vcReportBuffer);
        mexEvalString("drawnow;");
    }
    else{
        va_list argsList;

        va_start(argsList,a_cpcFormatString);
        nReturn=vdprintf(STDOUT_FILENO,a_cpcFormatString,argsList);
        va_end(argsList);
    }

    return nReturn;
}


static int ErrorFunction(const char* a_cpcFormatString, ...)
{
    int nReturn;
    pthread_t  threadHandle = pthread_self();
    int nPid = (int)getpid();

    if(s_mainThreadHandle && (threadHandle==s_mainThreadHandle) && (nPid==s_nPid)){
        char vcReportBuffer[1024];
        char vcReportFinal[1024];
        char vcCompleteBuffer[1024];
        int nIndexIn(0), nIndexFinal(0);
        va_list argsList;

        va_start(argsList,a_cpcFormatString);
        nReturn=vsnprintf(vcReportBuffer,1024,a_cpcFormatString,argsList);
        va_end(argsList);

        for(;vcReportBuffer[nIndexIn]&&(nIndexIn<1023)&&(nIndexFinal<1023);++nIndexIn){
            switch(vcReportBuffer[nIndexIn]){
            case '\n':
                vcReportFinal[nIndexFinal++]='\\';
                vcReportFinal[nIndexFinal++]='n';
                break;
            case '\t':
                vcReportFinal[nIndexFinal++]='\\';
                vcReportFinal[nIndexFinal++]='t';
                break;
            default:
                vcReportFinal[nIndexFinal++]=vcReportBuffer[nIndexIn];
                break;
            }
        }
        vcReportFinal[nIndexFinal++]=0;

        snprintf(vcCompleteBuffer,1024,"fprintf(2,%c%s%c);drawnow;",'\'',vcReportFinal,'\'');

        mexEvalString(vcCompleteBuffer);
    }
    else{
        va_list argsList;

        va_start(argsList,a_cpcFormatString);
        nReturn=vdprintf(STDERR_FILENO,a_cpcFormatString,argsList);
        va_end(argsList);
    }

    return nReturn;
}



static double GetNumericData(const mxArray* a_numeric)
{
    mxClassID classId=mxGetClassID(a_numeric);

    switch(classId)
    {
    case mxDOUBLE_CLASS:
        return *mxGetPr(a_numeric);
    case mxSINGLE_CLASS:
        return (double)(*((float*)mxGetData( a_numeric )));
        break;
    case mxINT8_CLASS:
        return (double)(*((int8_t*)mxGetData( a_numeric )));
        break;
    case mxUINT8_CLASS:
        return (double)(*((uint8_t*)mxGetData( a_numeric )));
        break;
    case mxINT16_CLASS:
        return (double)(*((int16_t*)mxGetData( a_numeric )));
        break;
    case mxUINT16_CLASS:
        return (double)(*((uint16_t*)mxGetData( a_numeric )));
        break;

    case mxINT32_CLASS:
        return (double)(*((int32_t*)mxGetData( a_numeric )));
        break;
    case mxUINT32_CLASS:
        return (double)(*((uint32_t*)mxGetData( a_numeric )));
        break;
    case mxINT64_CLASS:
        return (double)(*((int64_t*)mxGetData( a_numeric )));
        break;
    case mxUINT64_CLASS:
        return (double)(*((uint64_t*)mxGetData( a_numeric )));
        break;
    default:
        break;
    }

    return 0;
}


static bool GetBranchNamesFromMatlabCell(const mxArray* a_pMatlabCell, ::std::vector< ::std::string >* a_pVectorOfBrNames)
{
    mxArray* pBranchName;
    int nStrLen,nIndexBranch,nBranchCount;

    if( !mxIsCell(a_pMatlabCell) ){return false;}
    nBranchCount =(int) mxGetNumberOfElements(a_pMatlabCell);
    if(nBranchCount<1){return false;}

    a_pVectorOfBrNames->resize(nBranchCount);

    for(nIndexBranch=0;nIndexBranch<nBranchCount;++nIndexBranch){
        pBranchName = mxGetCell(a_pMatlabCell,nIndexBranch);  // todo: investigate whethe copy returned
        nStrLen = (int)mxGetNumberOfElements(pBranchName);
        (*a_pVectorOfBrNames)[nIndexBranch].resize(nStrLen+2);
        mxGetString(pBranchName,const_cast<char*>((*a_pVectorOfBrNames)[nIndexBranch].data()),nStrLen+1);
        MAKE_REPORT_GLOBAL(2,"str=%s, nStrLen=%d",(*a_pVectorOfBrNames)[nIndexBranch].c_str(),nStrLen);
    }

    return true;
}


namespace glbDataFields{enum{Name,Data=3};}
namespace locDataFields{enum{Time,Event,Data};}
namespace baseInfoFields{enum{Type,itemsCount};}
namespace advInfoFields{enum{Name=0,numberOfEntriesInTheFile=3,firstTime,lastTime,firstEvent,lastEvent};}
static size_t InfoToMatlabRaw(mxClassID* a_pClsIdOfData,mxArray* a_pMatlabArray, size_t a_nIndex, int a_unFieldOffset,const data::EntryInfoBase& a_info);


static mxArray* DataToMatlab(const ::std::vector< ::std::string >& a_daqNames, const ::std::vector< data::getter::EntryInfoAdvTI >& a_data)
{
    //Type::Type   dataType;int          itemsCount;  int          numberOfEntriesInTheFile;
    // struct EntryInfoAdv : data::EntryInfo{int firstTime,lastTime,firstEvent,lastEvent; ::std::string name; const int* ptr()const{return &firstTime;} int* ptr(){return &firstTime;}};
    static const char* svcpcFieldNamesGlb[] = {"name","type","itemsCount","data"};
    static const int scnNumberOfFieldsGlb = sizeof(svcpcFieldNamesGlb)/sizeof(const char*);
    static const char* svcpcFieldNames[] = {"time","event","data"};
    static const int scnNumberOfFields = sizeof(svcpcFieldNames)/sizeof(const char*);
    const size_t cunNumOfBranches(a_data.size());
    size_t nIndexBranch, nIndexEvent, nNumberOfEvents;
    size_t unItemSize;
    mxClassID clsId;
    mxArray *pNameGlb,*pDataGlb;
    mxArray *pTime, *pEvent,*pData;
    mxArray* pMatArray = mxCreateStructMatrix(cunNumOfBranches,1,scnNumberOfFieldsGlb,svcpcFieldNamesGlb);
    HANDLE_MEM_DEF(pMatArray,"No memory to create struct matrix");

    for(nIndexBranch=0;nIndexBranch<cunNumOfBranches;++nIndexBranch){

        nNumberOfEvents = a_data[nIndexBranch].entryData.size();

        unItemSize=InfoToMatlabRaw(&clsId,pMatArray,nIndexBranch,1,a_data[nIndexBranch].entryInfo);

        pNameGlb = mxCreateString(a_daqNames[nIndexBranch].c_str());HANDLE_MEM_DEF(pNameGlb," ");
        pDataGlb = mxCreateStructMatrix(nNumberOfEvents,1,scnNumberOfFields,svcpcFieldNames);HANDLE_MEM_DEF(pDataGlb," ");

        for(nIndexEvent=0; nIndexEvent<nNumberOfEvents;++nIndexEvent){
            pTime = mxCreateNumericMatrix(1,1,mxINT32_CLASS,mxREAL);HANDLE_MEM_DEF(pTime," ");*((int32_t*)mxGetData(pTime))=a_data[nIndexBranch].entryData[nIndexEvent].time();
            pEvent = mxCreateNumericMatrix(1,1,mxINT32_CLASS,mxREAL);HANDLE_MEM_DEF(pEvent," ");*((int32_t*)mxGetData(pEvent))=a_data[nIndexBranch].entryData[nIndexEvent].gen_event();
            pData = mxCreateNumericMatrix(1,a_data[nIndexBranch].entryInfo.itemsCount,clsId,mxREAL);HANDLE_MEM_DEF(pData," ");
            memcpy( mxGetData(pData),a_data[nIndexBranch].entryData[nIndexEvent].data<void>(),unItemSize*a_data[nIndexBranch].entryInfo.itemsCount);

            mxSetFieldByNumber(pDataGlb,nIndexEvent,(size_t)locDataFields::Time,pTime);
            mxSetFieldByNumber(pDataGlb,nIndexEvent,(size_t)locDataFields::Event,pEvent);
            mxSetFieldByNumber(pDataGlb,nIndexEvent,(size_t)locDataFields::Data,pData);

        }

        mxSetFieldByNumber(pMatArray,nIndexBranch,(size_t)glbDataFields::Name,pNameGlb);
        mxSetFieldByNumber(pMatArray,nIndexBranch,(size_t)glbDataFields::Data,pDataGlb);

    }

    return pMatArray;
}

static mxArray* AdvInfoToMatlab(const ::std::vector< data::engine::EntryInfoAdv >& a_info)
{
    //Type::Type   dataType;int          itemsCount;  int          numberOfEntriesInTheFile;
    // struct EntryInfoAdv : data::EntryInfo{int firstTime,lastTime,firstEvent,lastEvent; ::std::string name; const int* ptr()const{return &firstTime;} int* ptr(){return &firstTime;}};
    static const char* svcpcFieldNamesGlb[] = {"name","type","itemsCount","numberOfEntriesInTheFile","firstTime","lastTime","firstEvent","lastEvent"};
    static const int scnNumberOfFieldsGlb = sizeof(svcpcFieldNamesGlb)/sizeof(const char*);
    const size_t cunNumOfBranches(a_info.size());
    size_t nIndexBranch;
    mxClassID clsId;
    mxArray *pName, *pNumberInFile, *pFirstTime, *pLastTime, *pFirstEvent, *pLastEvent;
    mxArray* pMatArray = mxCreateStructMatrix(cunNumOfBranches,1,scnNumberOfFieldsGlb,svcpcFieldNamesGlb);
    HANDLE_MEM_DEF(pMatArray,"No memory to create struct matrix");

    for(nIndexBranch=0;nIndexBranch<cunNumOfBranches;++nIndexBranch){
        InfoToMatlabRaw(&clsId,pMatArray,nIndexBranch,1,a_info[nIndexBranch]);

        pName = mxCreateString(a_info[nIndexBranch].name.c_str());HANDLE_MEM_DEF(pName," ");
        pNumberInFile = mxCreateNumericMatrix(1,1,mxINT32_CLASS,mxREAL);HANDLE_MEM_DEF(pNumberInFile," ");*((int32_t*)mxGetData(pNumberInFile))=a_info[nIndexBranch].numberOfEntriesInTheFile;
        pFirstTime = mxCreateNumericMatrix(1,1,mxINT32_CLASS,mxREAL);HANDLE_MEM_DEF(pFirstTime," ");*((int32_t*)mxGetData( pFirstTime ))=a_info[nIndexBranch].firstTime;
        pLastTime = mxCreateNumericMatrix(1,1,mxINT32_CLASS,mxREAL);HANDLE_MEM_DEF(pLastTime," ");*((int32_t*)mxGetData( pLastTime ))=a_info[nIndexBranch].lastTime;
        pFirstEvent = mxCreateNumericMatrix(1,1,mxINT32_CLASS,mxREAL);HANDLE_MEM_DEF(pFirstEvent," ");*((int32_t*)mxGetData( pFirstEvent ))=a_info[nIndexBranch].firstEvent;
        pLastEvent = mxCreateNumericMatrix(1,1,mxINT32_CLASS,mxREAL);HANDLE_MEM_DEF(pLastEvent," ");*((int32_t*)mxGetData( pLastEvent ))=a_info[nIndexBranch].lastEvent;

        mxSetFieldByNumber(pMatArray,nIndexBranch,(size_t)advInfoFields::Name,pName);
        mxSetFieldByNumber(pMatArray,nIndexBranch,(size_t)advInfoFields::numberOfEntriesInTheFile,pNumberInFile);
        mxSetFieldByNumber(pMatArray,nIndexBranch,(size_t)advInfoFields::firstTime,pFirstTime);
        mxSetFieldByNumber(pMatArray,nIndexBranch,(size_t)advInfoFields::lastTime,pLastTime);
        mxSetFieldByNumber(pMatArray,nIndexBranch,(size_t)advInfoFields::firstEvent,pFirstEvent);
        mxSetFieldByNumber(pMatArray,nIndexBranch,(size_t)advInfoFields::lastEvent,pLastEvent);

    }

    return pMatArray;
}


static size_t InfoToMatlabRaw(mxClassID* a_pClsIdOfData,mxArray* a_pMatlabArray, size_t a_nIndex, int a_unFieldOffset,const data::EntryInfoBase& a_info)
{
    // time_field = mxGetFieldNumber(pDataGlb,svcpcFieldNames[0]);
    mxArray *pType, *pItemsCount;
    size_t unItemSize;
    mxClassID& clsId = *a_pClsIdOfData;

    switch(a_info.dataType){
    case data::Type::Int:
        pType = mxCreateString("Int_t");
        clsId = mxINT32_CLASS;
        unItemSize = 4;
        break;
    case data::Type::Float:
        pType = mxCreateString("Float_t");
        clsId = mxSINGLE_CLASS;
        unItemSize = 4;
        break;
    case data::Type::String:
        pType = mxCreateString("String");
        clsId = mxCHAR_CLASS;
        unItemSize = 512;
        break;
    case data::Type::IIII_old:
        pType = mxCreateString("IIII_old");
        clsId = mxSTRUCT_CLASS;
        unItemSize = 16;
        break;
    case data::Type::IFFF_old:
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

    pItemsCount = mxCreateNumericMatrix(1,1,mxINT32_CLASS,mxREAL);HANDLE_MEM_DEF(pItemsCount," ");*((int32_t*)mxGetData( pItemsCount ))=a_info.itemsCount;

    mxSetFieldByNumber(a_pMatlabArray,a_nIndex,a_unFieldOffset + (size_t)baseInfoFields::Type,pType);
    mxSetFieldByNumber(a_pMatlabArray,a_nIndex,a_unFieldOffset + (size_t)baseInfoFields::itemsCount,pItemsCount);

    return unItemSize;
}
