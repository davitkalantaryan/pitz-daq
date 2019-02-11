//
// file:        pitz_daq_data_indexer.cpp
// created on:  2018 Nov 19
//

#include "pitz/daq/data/indexer.hpp"
#include <stdio.h>

#define DIRECTORY_FOR_INDEXING  "//afs/ifh.de/group/pitz/doocs/data/DAQdata/INDEX/"

namespace pitz{ namespace daq { namespace data { namespace indexer{

static inline void GetIndexFileName(const char* a_daqEntryName,char* a_pcBuffer, int a_bufLen)
{snprintf(a_pcBuffer,a_bufLen,DIRECTORY_FOR_INDEXING "%s.idx",a_daqEntryName);}

int DoIndexing(const char* a_rootFileName,const char* a_daqEntryName,int a_startTime, int a_endTime,int a_startGenEvent,int a_endGenEvent)
{
    FILE* fpIndexFile;
    char vcBuffer[1024];

    GetIndexFileName(a_daqEntryName,vcBuffer,1024);
    fpIndexFile = fopen(vcBuffer,"a");
    if(!fpIndexFile){return -1;}

    fprintf(fpIndexFile,"%d:%d,%d:%d,%s\n",a_startTime,a_startGenEvent,a_endTime,a_endGenEvent,a_rootFileName);
    fclose(fpIndexFile);
    return 0;
}


int GetListOfFilesForTimeInterval(void* a_clbkData, callbackN::TypeFileNameReader a_fpReader, const char* a_daqEntryName, int a_startTime, int a_endTime)
{
    FILE* fpFile=NULL;
    char vcBuffer[1024];
    int nBegTimeFile,nBegEvNumFile,nEndTimeFile,nEndEvNumFile;
    bool bBegFound(false);
    daq::callbackN::retType::Type clbkReturn(daq::callbackN::retType::Continue);

    GetIndexFileName(a_daqEntryName,vcBuffer,1024);
    fpFile = fopen(vcBuffer,"r");
    if(!fpFile){
        MAKE_ERROR_GLOBAL("ERROR: \"%s\": INCORRECT BRANCH NAME",a_daqEntryName);
        return -1;
    }

    while(fscanf(fpFile,"%d:%d,%d:%d,%1024s",&nBegTimeFile,&nBegEvNumFile,&nEndTimeFile,&nEndEvNumFile,vcBuffer)>0){
        if(!bBegFound){
            if(  ( (nBegTimeFile<=a_startTime)&&(a_startTime<=nEndTimeFile) )  ||
                 ( (nBegTimeFile<=a_endTime)&&(a_endTime<=nEndTimeFile) )  ||
                 ( (a_startTime<=nBegTimeFile)&&(nBegTimeFile<=a_endTime) ) ||
                 ( (a_startTime<=nEndTimeFile)&&(nEndTimeFile<=a_endTime) )    )
            {
                bBegFound = true;
            }
        }
        if(bBegFound){

            if(  ( (nBegTimeFile<=a_startTime)&&(a_startTime<=nEndTimeFile) )  ||
                 ( (nBegTimeFile<=a_endTime)&&(a_endTime<=nEndTimeFile) )  ||
                 ( (a_startTime<=nBegTimeFile)&&(nBegTimeFile<=a_endTime) ) ||
                 ( (a_startTime<=nEndTimeFile)&&(nEndTimeFile<=a_endTime) )  )
            {
                clbkReturn = (*a_fpReader)(a_clbkData,vcBuffer);
                if(clbkReturn!=daq::callbackN::retType::Continue){break;}
            }
            else if(nBegTimeFile>a_endTime){break;}

        }
    }

    if(fpFile){fclose(fpFile);}
    return 0;
}


int GetListOfFilesForGenEventInterval(void* a_clbkData, callbackN::TypeFileNameReader a_fpReader,const char* a_daqEntryName, int a_startGenEvent, int a_endGenEvent)
{
    FILE* fpFile=NULL;
    char vcBuffer[1024];
    int nBegTimeFile,nBegEvNumFile,nEndTimeFile,nEndEvNumFile;
    bool bBegFound(false);
    daq::callbackN::retType::Type clbkReturn(daq::callbackN::retType::Continue);

    GetIndexFileName(a_daqEntryName,vcBuffer,1024);
    fpFile = fopen(vcBuffer,"r");
    if(!fpFile){
        MAKE_ERROR_GLOBAL("ERROR: \"%s\": INCORRECT BRANCH NAME",a_daqEntryName);
        return -1;
    }

    while(fscanf(fpFile,"%d:%d,%d:%d,%1024s",&nBegTimeFile,&nBegEvNumFile,&nEndTimeFile,&nEndEvNumFile,vcBuffer)>0){
        if((nBegEvNumFile<=a_startGenEvent)&&(nEndEvNumFile>=a_startGenEvent)){bBegFound=true;}
        if(bBegFound){

            if(nEndEvNumFile>=a_endGenEvent){break;}
            clbkReturn = (*a_fpReader)(a_clbkData,vcBuffer);
            if(clbkReturn!=daq::callbackN::retType::Continue){break;}

        }
    }

    if(fpFile){fclose(fpFile);}
    return 0;
}






}}}} // namespace pitz{ namespace daq { namespace data { namespace indexer{
