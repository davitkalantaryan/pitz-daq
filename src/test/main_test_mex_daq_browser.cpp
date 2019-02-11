//
// file:        mex_simple_root_reader.cpp
//

//#include <matrix.h>
#include <string.h>
#include <stdio.h>
#include "pitz/daq/base.hpp"
#include "pitz/daq/data/getter/frompipe.hpp"
#include "pitz/daq/data/getter/tobuffer.hpp"


#define HELPER_BINARY_NAME  "bindaq_browser_actual"
#define mexPrintf   printf
#define mexAtExit   atexit

static int ErrorFunction(const char* a_cpcFormatString, ...);
static int ReportFunction(const char* a_cpcFormatString, ...);


using namespace pitz;
using namespace pitz::daq;

static data::getter::ToBuffer<data::getter::FromPipe>    s_dataGetter;
static pthread_t    s_mainThreadHandle = (pthread_t)0;


int main()
{
    int nReturn;

    s_mainThreadHandle = pthread_self();
    log::g_globalLogger = ReportFunction;
    log::g_globalErrorLogger = ErrorFunction;

    dprintf(STDOUT_FILENO,"pid=%d\n",(int)getpid());
    //s_dataGetter->SetIsDebug(1);
    log::g_nLogLevel = 10;

#if 0
    //const char* pcRootFileName = "/acs/pitz/daq/2018/06/pitznoadc0/PITZ_DATA.pitznoadc0.2018-06-10-0540.root";
    const char* pcRootFileName = "/acs/pitz/daq/2018/06/pitzrf2.adc/PITZ_DATA.pitzrf2.adc.2018-06-10-1255.root";
    ::std::vector< ::std::string > branches;
    branches.resize(2);

    //branches[0]="INTERLOCKV4_RF2_G5__CNTMAX__BIT18_20170803";
    branches[0]="RF2Cpl10MWFW";
    branches[1]="RF2Cpl10MWRE";
    nReturn=s_dataGetter.GetMultipleEntries(pcRootFileName,branches);
#endif

#if 0 // error function test


    nReturn = 0;
    MAKE_ERROR_GLOBAL("Provide the option!");

    //mexEvalString(vcCompleteBuffer);
#endif  // #if 1 // error function test


#if 1  // test of GetEntriesInfo
    //nReturn = s_dataGetter.GetEntriesInfo("/acs/pitz/daq/2018/06/pitznoadc0/PITZ_DATA.pitznoadc0.2018-06-10-0540t.root");
    nReturn = s_dataGetter.GetEntriesInfo("/acs/pitz/daq/2018/11/pitznoadc1/PITZ_DATA.pitznoadc1.2018-10-28-1032.root");
#endif  // #if 1  // test of GetEntriesInfo


#if 0  // test of GetMultipleEntriesTI
        ::std::vector< ::std::string > branches;
        branches.resize(18);

        branches[0]="INTERLOCKV4_RF2_G5__CNTMAX__BIT18_20170803";
        branches[1]="RF2Cpl10MWFW";
        branches[2]="RF2Cpl10MWRE";

        branches[3]="GUN__COUPLER__PMT_20140905";
        branches[4]="GUN__COUPLER__E_DET_20140905";
        branches[5]="GUN__WG1__THALES_PMT_VAC_20140905";
        branches[6]="GUN__WG2__THALES_PMT_VAC_20140905";
        branches[7]="GUN__WG1__THALES_E_DET_VAC_20140905";
        branches[8]="GUN__WG2__THALES_E_DET_VAC_20140905";
        branches[9]="RF2WG1CavityFW";
        branches[10]="RF2WG2CavityFW";
        branches[11]="RF2WG1CavityRE";
        branches[12]="RF2WG2CavityRE";
        branches[13]="GUN__WG1__THALES_PMT_AIR_20140905";
        branches[14]="GUN__WG2__THALES_PMT_AIR_20140905";
        branches[15]="GUN__WG1__RF_WINDOW_PMT_AIR_20140905";
        branches[16]="GUN__WG2__RF_WINDOW_PMT_AIR_20140905";
        branches[17]="X2TIMER_RF2_MACRO_PULSE_NUMBER_20170512";
        //branches[18]="INTERLOCKV4_RF2_G5__CNTMAX__BIT18_20170803";

        nReturn=s_dataGetter.GetMultipleEntriesTI(branches,1528628400,1528628405);
#endif   // #if 0  // test of GetMultipleEntriesTI


        dprintf(STDOUT_FILENO,"nReturn=%d\n",nReturn);

        return 0;
}


static int ReportFunction(const char* a_cpcFormatString, ...)
{
    int nReturn;
    pthread_t  threadHandle = pthread_self();

    if(s_mainThreadHandle && (threadHandle==s_mainThreadHandle)){
        char vcReportBuffer[1024];
        va_list argsList;

        va_start(argsList,a_cpcFormatString);
        nReturn=vsnprintf(vcReportBuffer,1024,a_cpcFormatString,argsList);
        va_end(argsList);
        //mexPrintf("%s",vcReportBuffer);
        //mexEvalString("drawnow;");
        fprintf(stdout,"%s",vcReportBuffer);
        fflush(stdout);
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

    if(s_mainThreadHandle && (threadHandle==s_mainThreadHandle)){
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

        //mexEvalString(vcCompleteBuffer);
        fprintf(stderr,"%s",vcCompleteBuffer);
        fflush(stderr);
    }
    else{
        va_list argsList;

        va_start(argsList,a_cpcFormatString);
        nReturn=vdprintf(STDERR_FILENO,a_cpcFormatString,argsList);
        va_end(argsList);
    }

    return nReturn;
}


#if 0

static int s_nLibraryStarted = 0;
static data::engine::FromPipe   s_engine;
static data::getter::ToBuffer   s_dataGetter(&s_engine);

static void PrintHelp(void);
static void MexCleanupFunction(void);
static double GetNumericData(const mxArray* a_numeric);
static mxArray* AdvInfoToMatlab(const ::std::vector< data::engine::EntryInfoAdv >& a_info);
static mxArray* DataToMatlab(const ::std::vector< ::std::string >& a_daqNames, const ::std::vector< data::getter::EntryInfoAdvTI >& a_data);
static bool GetBranchNamesromMatlabCell(const mxArray* a_pMatlabCell, ::std::vector< ::std::string >* a_pVectorOfBrNames);

void mexFunction(int a_nNumOuts, mxArray *a_Outputs[],
    int a_nNumInps, const mxArray*a_Inputs[])
{
    int nStrLen, nReturn;
    char *pcOption;

    if(!s_nLibraryStarted){
        mexPrintf("version 4.0.0. numberOfInputs=%d, inp=%p, numberOfOutputs=%d, outp=%p\n",a_nNumInps,a_Inputs,a_nNumOuts,a_Outputs);
        log::g_globalLogger = mexPrintf;
        log::g_globalErrorLogger = mexPrintf;
        mexAtExit(&MexCleanupFunction);
        s_nLibraryStarted = 1;
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
        if(nReturn<0){goto returnPoint;}
        a_Outputs[0] = AdvInfoToMatlab(info);
    }
    else if(strcmp("--get-data-for-entries",pcOption)==0){
        char* pcRootFileName;
        ::std::vector< ::std::string > vectorBranches;
        const ::std::vector< data::getter::EntryInfoAdvTI >& data = s_dataGetter.Data();
        MAKE_REPORT_GLOBAL(1,"option: --get-data-for-entries");

        if( (a_nNumInps<3) || (!mxIsChar(a_Inputs[1])) || (!GetBranchNamesromMatlabCell(a_Inputs[2],&vectorBranches))  ){
            MAKE_ERROR_GLOBAL("Name of the root file and the names of entries should be provided!");
            goto returnPoint;
        }

        nStrLen=(int)(mxGetM(a_Inputs[1])*mxGetN(a_Inputs[1]));
        pcRootFileName=(char*)alloca(nStrLen+2);
        mxGetString(a_Inputs[1],pcRootFileName,nStrLen+1);

        nReturn=s_dataGetter.GetMultipleEntries(pcRootFileName,vectorBranches);
        if(nReturn<0){goto returnPoint;}
        a_Outputs[0] = DataToMatlab(vectorBranches,data);

    }
    else if(strcmp("--get-data-for-time-interval",pcOption)==0){
        int nStartTime, nEndTime;
        ::std::vector< ::std::string > vectorBranches;
        const ::std::vector< data::getter::EntryInfoAdvTI >& data = s_dataGetter.Data();
        MAKE_REPORT_GLOBAL(1,"option: --get-data-for-time-interval");

        if( (a_nNumInps<4) || (!mxIsNumeric(a_Inputs[2])) || (!mxIsNumeric(a_Inputs[3])) || (!GetBranchNamesromMatlabCell(a_Inputs[1],&vectorBranches))  ){
            MAKE_ERROR_GLOBAL("Name of the root file and the names of entries should be provided!");
            goto returnPoint;
        }

        nStartTime = GetNumericData(a_Inputs[2]);
        nEndTime = GetNumericData(a_Inputs[3]);

        nReturn=s_dataGetter.GetMultipleEntriesTI(vectorBranches,nStartTime,nEndTime);
        if(nReturn<0){goto returnPoint;}
        a_Outputs[0] = DataToMatlab(vectorBranches,data);
    }
    else{
        a_Outputs[0] = mxCreateString("Wrong option provided!\n");
        PrintHelp();
    }

returnPoint:
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
        //
    }
    s_nLibraryStarted = 0;
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


static bool GetBranchNamesromMatlabCell(const mxArray* a_pMatlabCell, ::std::vector< ::std::string >* a_pVectorOfBrNames)
{
    mxArray* pBranchName;
    int nStrLen,nIndexBranch,nBranchCount;

    if( !mxIsCell(a_pMatlabCell) ){return false;}
    nBranchCount =(int) mxGetNumberOfElements(a_pMatlabCell);
    if(nBranchCount<1){return false;}

    a_pVectorOfBrNames->resize(nBranchCount);

    for(nIndexBranch=0;nIndexBranch<nBranchCount;++nIndexBranch){
        pBranchName = mxGetCell(a_pMatlabCell,nIndexBranch);  // todo: investigate whethe copy returned
        nStrLen = mxGetNumberOfElements(pBranchName);
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
static size_t InfoToMatlabRaw(mxClassID* a_pClsIdOfData,mxArray* a_pMatlabArray, size_t a_nIndex, size_t a_unFieldOffset,const data::EntryInfoBase& a_info);


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

            // todo: this 3 lines can crash MATLAB, should be tested
            mxDestroyArray(pData);
            mxDestroyArray(pEvent);
            mxDestroyArray(pTime);
        }

        mxSetFieldByNumber(pMatArray,nIndexBranch,(size_t)glbDataFields::Name,pNameGlb);
        mxSetFieldByNumber(pMatArray,nIndexBranch,(size_t)glbDataFields::Data,pDataGlb);

        // todo: this 2 lines can crash MATLAB, should be tested
        mxDestroyArray(pDataGlb);
        mxDestroyArray(pNameGlb);
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

        // todo: this 6 lines can crash MATLAB, should be tested
        mxDestroyArray(pLastEvent);
        mxDestroyArray(pFirstEvent);
        mxDestroyArray(pLastTime);
        mxDestroyArray(pFirstTime);
        mxDestroyArray(pNumberInFile);
        mxDestroyArray(pName);
    }

    return pMatArray;
}


static size_t InfoToMatlabRaw(mxClassID* a_pClsIdOfData,mxArray* a_pMatlabArray, size_t a_nIndex, size_t a_unFieldOffset,const data::EntryInfoBase& a_info)
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

    // todo: this 2 lines can crash MATLAB, should be tested
    mxDestroyArray(pItemsCount);
    mxDestroyArray(pType);

    return unItemSize;
}

#endif
