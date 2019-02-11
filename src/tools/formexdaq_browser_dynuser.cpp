//
// file:        mex_simple_root_reader.cpp
//

#include <stdarg.h>
#include <vector>
#include <string>
#include "tool/formexdaq_browser_dynuser.h"
#include <stdint.h>
#include <math.h>

#define _INDEX_FILE_DIR_        "//afs/ifh.de/group/pitz/doocs/data/DAQdata/INDEX"

typedef struct DataForRoot
{
    std::vector<structCommon>& vectorData;
    /*--------------------------------*/
    DataForRoot(std::vector<structCommon>* a_pVectorData):vectorData(*a_pVectorData){}
}DataForRoot;

typedef struct SAddInfoForSearch{
    uint16_t  isBeginFound: 1;
    uint16_t  isComplete: 1;
    uint16_t  indexInTheVector : 10;
    SAddInfoForSearch(){this->isBeginFound=this->isComplete=0;}
}SAddInfoForSearch;


typedef struct DataForMultiRoot
{
    std::vector< std::vector<structCommon> >& vectorData;
    const std::vector<std::string>& daqNames;
    std::vector<SAddInfoForSearch>& addInfo;
    int startTimeS, endTimeS;
    int firstEventNumber, maxEventsCount;
    /*--------------------------------*/
    DataForMultiRoot(std::vector< std::vector<structCommon> >* a_pVectorData, const std::vector<std::string>& a_daqNames,
                     std::vector<SAddInfoForSearch>& a_addInfo,int a_start, int a_end)
        :vectorData(*a_pVectorData),daqNames(a_daqNames),addInfo(a_addInfo)
    {
        this->startTimeS=a_start;
        this->endTimeS = a_end;
        this->firstEventNumber = this->maxEventsCount = 0;
        vectorData.resize(a_daqNames.size());
    }
}DataForMultiRoot;

typedef struct DataForNames
{
    std::vector<std::string>& vectorNames;
    /*--------------------------------*/
    DataForNames(std::vector<std::string>* a_pVectorNames):vectorNames(*a_pVectorNames){}
}DataForNames;

static int FunctionReport(void* a_owner,const char* a_format,...);
static callbackReturnType RootReaderHandle(void* a_owner,int a_index, int a_count,structCommon** a_ppForRoot);
static callbackReturnType FuncBrahNameHandler(void* ,int,int,const char* a_daqEntryName);
static callbackReturnType MultiRootReaderHandle(void* a_owner,int a_index, int a_count,structCommon** a_ppForRoot,int a_nIndexBranch);

static int FindListOfFiles(const char* a_cpcDaqEntryName, int a_start, int a_end, std::vector<std::string>* a_pFileNames)
{
    FILE* fpFile=NULL;
    char vcBuffer[1024];
    int nBegTimeFile,nBegEvNumFile,nEndTimeFile,nEndEvNumFile;
    bool bBegFound(false);

    snprintf(vcBuffer,1023,"%s/%s.idx",_INDEX_FILE_DIR_,a_cpcDaqEntryName );
    fpFile = fopen(vcBuffer,"r");
    if(!fpFile){
        (*g_fpReport)("ERROR: Line(%d): \"%s\": INCORRECT BRANCH NAME!\n",__LINE__,a_cpcDaqEntryName);
        return -1;
    }

    while(fscanf(fpFile,"%d:%d,%d:%d,%1024s",&nBegTimeFile,&nBegEvNumFile,&nEndTimeFile,&nEndEvNumFile,vcBuffer)>0){
        if((nBegTimeFile<=a_start)&&(nEndTimeFile>=a_start)){bBegFound=true;}
        if(bBegFound){
            a_pFileNames->push_back(vcBuffer);
        }
        if(bBegFound && (nEndTimeFile>=a_end)){break;}
    }

    if(fpFile){fclose(fpFile);}
    return 0;
}


int MultyReadEntriesFromIndexToVectorDl(std::vector<std::vector<structCommon> >* a_outVector,
                                   const std::vector<std::string>& a_daqNames, int a_start, int a_end)
{
    int nFoundBranches = 0;
    std::vector<std::string> daqNames(a_daqNames);
    std::vector<SAddInfoForSearch> addInfo(a_daqNames.size());
    DataForMultiRoot clbkData(a_outVector,a_daqNames,addInfo,a_start,a_end);
    structCommon aInitialDataBuffer;
    std::vector<std::string> vectRootFilesName;
    size_t indexFile,unSizeFile;
    size_t indexBranch;

    for(indexBranch=0;indexBranch<addInfo.size();++indexBranch){
        addInfo[indexBranch].indexInTheVector = indexBranch;
    }

    while(daqNames.size()){
        vectRootFilesName.clear();
        FindListOfFiles(daqNames[0].c_str(),a_start,a_end,&vectRootFilesName);
        if(!vectRootFilesName.size()){
            DEBUG_ROOT_APP(0,"ERROR: wrong time interval startTime=%d\\endTime=%d",a_start,a_end);
            return  0;
        }
        unSizeFile = vectRootFilesName.size();
        for(indexFile=0;indexFile<unSizeFile;++indexFile){
            //addInfo.resize(daqNames.size());
            //for(indexBranch=0;indexBranch<addInfo.size();++indexBranch){addInfo[indexBranch].isComplete=addInfo[indexBranch].isBeginFound=0;}
            MultyReadOneRootFileToClbkCDl(vectRootFilesName[indexFile].c_str(),daqNames,&aInitialDataBuffer,&clbkData,MultiRootReaderHandle,FunctionReport);
            for(indexBranch=0;indexBranch<addInfo.size();){
                if(addInfo[indexBranch].isComplete){
                    ++nFoundBranches;
                    addInfo.erase(addInfo.begin()+indexBranch);
                    daqNames.erase(daqNames.begin()+indexBranch);
                }
                else{++indexBranch;}
            } // for(indexBranch=0;indexBranch<addInfo.size();){
        }  // for(indexFile=0;indexFile<unSizeFile;++indexFile){
    }  // while(daqNames.size()){

    //return MultyReadOneRootFileToClbkCDl(a_cpcRootFileName,a_daqNames,&aInitialDataBuffer,&clbkData,MultiRootReaderHandle,FunctionReport);
    return clbkData.maxEventsCount;
}


int ReadOneRootFileToVectorDl(const char* a_cpcRootFileName, const char* a_cpcTreeAndBranchName, std::vector<structCommon>* a_outVector)
{
    DataForRoot clbkData(a_outVector);
    structCommon aInitialDataBuffer;

    return ReadOneRootFileToClbkDl(a_cpcRootFileName,a_cpcTreeAndBranchName,&aInitialDataBuffer,&clbkData,RootReaderHandle,FunctionReport);
}


int GetAllEntriesInTheRootFileToVectorDl(const char* a_cpcRootFileName, std::vector<std::string>* a_outVector)
{
    DataForNames clbkData(a_outVector);
    return GetAllEntriesInTheRootFileDl(a_cpcRootFileName,&clbkData,FuncBrahNameHandler,FunctionReport);
}

int MultyReadOneRootFileToVectorDl(const char* a_cpcRootFileName,std::vector<std::vector<structCommon> >* a_outVector,
                                   const std::vector<std::string>& a_daqNames, int a_start, int a_end)
{
    std::vector<SAddInfoForSearch> addInfo;
    addInfo.resize(a_daqNames.size());
    DataForMultiRoot clbkData(a_outVector,a_daqNames,addInfo,a_start,a_end);
    structCommon aInitialDataBuffer;

    return MultyReadOneRootFileToClbkCDl(a_cpcRootFileName,a_daqNames,&aInitialDataBuffer,&clbkData,MultiRootReaderHandle,FunctionReport);
}




int MultyReadOneRootFileToClbkCDl(const char* a_cpcRootFileName, const std::vector<std::string>& a_branchNames,
                          structCommon* a_pStrForRoot,void* a_pOwner,TypeMultyRtReader a_fpReader, TypeReport a_fpReport)
{
    int nReturn = -1;
    void* pLibHandle=NULL;
    TypeMultyReadOneRootFile fpReader = (TypeMultyReadOneRootFile)InitFunctionGlb(&pLibHandle,MultyReadOneRootFileToClbkCName);

    if(!fpReader){
        (*g_fpReport)("Unable to find function " ReadOneRootFileToClbkFncName "\n");
        goto returnPoint;
    }
    nReturn=(*fpReader)(a_cpcRootFileName,(void*)(&a_branchNames),a_pStrForRoot,a_pOwner,a_fpReader,a_fpReport);

returnPoint:
    if(pLibHandle){dlclose(pLibHandle);}
    return nReturn;
}



#ifdef __cplusplus
extern "C"{
#endif

const size_t g_unErrorStrLen = strlen(MARKED_ERROR);
int g_nLogLevel = 0;
int (*g_fpReport)(const char* format, ...) = &printf;


int ReadOneRootFileToClbkDl(const char* a_cpcRootFileName, const char* a_cpcTreeAndBranchName,
                          structCommon* a_pStrForRoot,void* a_pOwner,TypeRtReader a_fpReader, TypeReport a_fpReport)
{
    int nReturn = -1;
    void* pLibHandle=NULL;
    TypeReadOneRootFileToClbk fpReader = (TypeReadOneRootFileToClbk)InitFunctionGlb(&pLibHandle,ReadOneRootFileToClbkFncName);

    if(!fpReader){
        (*g_fpReport)("Unable to find function " ReadOneRootFileToClbkFncName "\n");
        goto returnPoint;
    }
    nReturn=(*fpReader)(a_cpcRootFileName,a_cpcTreeAndBranchName,a_pStrForRoot,a_pOwner,a_fpReader,a_fpReport);

returnPoint:
    if(pLibHandle){dlclose(pLibHandle);}
    return nReturn;
}


int GetAllEntriesInTheRootFileDl(const char* a_cpcRootFileName, void* a_pOwner, TypeBrahNameHandler a_brahNameHandler,TypeReport a_fpReport)
{
    int nReturn = -1;
    void* pLibHandle=NULL;
    TypeGetAllEntriesInTheRootFile fpEntryGetter = (TypeGetAllEntriesInTheRootFile)InitFunctionGlb(&pLibHandle,EntryGetterFncName);

    if(!fpEntryGetter){
        (*g_fpReport)("Unable to find function " ReadOneRootFileToClbkFncName "\n");
        goto returnPoint;
    }
    nReturn=(*fpEntryGetter)(a_cpcRootFileName,a_pOwner,a_brahNameHandler,a_fpReport);

returnPoint:
    if(pLibHandle){dlclose(pLibHandle);}
    return nReturn;
}



void* InitFunctionGlb(void** a_ppHandle,const char* a_fncName)
{
    void* fpReturn = NULL;
    void*& pLibHandle = *a_ppHandle;

    if(!pLibHandle){
        pLibHandle=dlopen(RtReaderLibraryName,RTLD_LAZY);
        if(!pLibHandle){
            (*g_fpReport)("Unable to open library " RtReaderLibraryName "\n");
            return NULL;
        }
    }

    fpReturn = dlsym(pLibHandle,a_fncName);
    if(!fpReturn){
        dlclose(pLibHandle);
        pLibHandle = NULL;
    }
    return fpReturn;
}


#ifdef __cplusplus
}
#endif


static callbackReturnType RootReaderHandle(void* a_owner,int a_index, int a_count,structCommon** a_ppForRoot)
{
    DataForRoot* pDataForRoot = (DataForRoot*)a_owner;

    if(a_index>=a_count){
        DEBUG_ROOT_APP(0,"ERROR: index=%d\\count=%d",a_index,a_count);
        return callbackReturn_fatal;
    }

    if(((int)pDataForRoot->vectorData.size())<a_count){
        DEBUG_ROOT_APP(1,"!!!! NewCount=%d",a_count);
        pDataForRoot->vectorData.resize(a_count);
    }
    DEBUG_ROOT_APP(2,"count=%d, index=%d, eventNumber=%d, time=%d",a_count,a_index,(*a_ppForRoot)->buffer,(*a_ppForRoot)->time);
    (pDataForRoot->vectorData)[a_index] = **a_ppForRoot;

    return callbackReturn_continue;
}


static callbackReturnType MultiRootReaderHandle(void* a_owner,int a_index, int a_count,structCommon** a_ppData,int a_nIndexBranch)
{
    DataForMultiRoot* pDataForRoot = (DataForMultiRoot*)a_owner;
    int nIndexInitial;

    if(a_nIndexBranch>=(int)pDataForRoot->daqNames.size()){
        DEBUG_ROOT_APP(0,"ERROR: branch_index=%d\\count=%d",a_nIndexBranch,(int)pDataForRoot->daqNames.size());
        return callbackReturn_fatal;
    }
    nIndexInitial = pDataForRoot->addInfo[a_nIndexBranch].indexInTheVector;

    if(((*a_ppData)->time>pDataForRoot->startTimeS)&&((*a_ppData)->time<pDataForRoot->endTimeS)){
        if(!pDataForRoot->addInfo[a_nIndexBranch].isBeginFound){
            if(!pDataForRoot->firstEventNumber){pDataForRoot->firstEventNumber=(*a_ppData)->buffer;}
            else if((*a_ppData)->buffer > pDataForRoot->firstEventNumber){
                //DEBUG_ROOT_APP(0,"ERROR: unable to match by event number");
            }
            else if((pDataForRoot->firstEventNumber-(*a_ppData)->buffer)==0){}
            else if((pDataForRoot->firstEventNumber-(*a_ppData)->buffer)<10){return callbackReturn_continue;}
            else {DEBUG_ROOT_APP(0,"ERROR: unable to match by event number");}
            pDataForRoot->addInfo[a_nIndexBranch].isBeginFound=1;
        }
        if(a_index>=a_count){
            DEBUG_ROOT_APP(0,"ERROR: index=%d\\count=%d",a_index,a_count);
            //return callbackReturn_fatal;
        }
        (pDataForRoot->vectorData[nIndexInitial]).push_back( **a_ppData);
        if(((int)(pDataForRoot->vectorData[nIndexInitial]).size())>pDataForRoot->maxEventsCount){pDataForRoot->maxEventsCount=(int)(pDataForRoot->vectorData[nIndexInitial]).size();}
    }
    else if(  (*a_ppData)->time > pDataForRoot->endTimeS  ){
        pDataForRoot->addInfo[a_nIndexBranch].isComplete = 1;
        return callbackReturn_finishForCurrent;
    }


    return callbackReturn_continue;
}


static callbackReturnType FuncBrahNameHandler(void* a_owner,int a_index,int a_count,const char* a_daqEntryName)
{
    DataForNames* pDataForNames = (DataForNames*)a_owner;

    if(a_index>=a_count){
        DEBUG_ROOT_APP(0,"ERROR: index=%d\\count=%d",a_index,a_count);
        return callbackReturn_fatal;
    }

    if(((int)pDataForNames->vectorNames.size())<a_count){
        DEBUG_ROOT_APP(1,"!!!! NewCount=%d",a_count);
        pDataForNames->vectorNames.resize(a_count);
    }

    DEBUG_ROOT_APP(2," %s   %d/%d",a_daqEntryName?a_daqEntryName:"nill",a_index,a_count);
    (pDataForNames->vectorNames)[a_index]=a_daqEntryName;
    return callbackReturn_continue;
}


static int FunctionReport(void*,const char* a_format,...)
{

    int nReturn(0);

    if((1<=g_nLogLevel)||(strncmp(a_format,MARKED_ERROR,g_unErrorStrLen)==0)){
        va_list argList;
        char vcBuffer[1024];

        va_start(argList,a_format);
        nReturn=vsnprintf(vcBuffer,1023,a_format,argList);
        va_end(argList);
        (*g_fpReport)("%s",vcBuffer);
    }

    return nReturn;
}
