//
// file:        pitz_daq_data_indexing.cpp
// created on:  2019 Jul 07
//

#include <pitz/daq/data/indexing.hpp>
#include <cpp11+/common_defination.h>
#include <stdio.h>

#define DIRECTORY_FOR_INDEXING  "//afs/ifh.de/group/pitz/doocs/data/DAQdata/INDEX/"
#define MAKE_ERROR_GLOBAL(...)


namespace pitz{ namespace daq{ namespace data{ namespace indexing{


static inline void GetIndexFileName(const char* a_daqEntryName,char* a_pcBuffer, int a_bufLen)
{snprintf(a_pcBuffer,STATIC_CAST(size_t,a_bufLen),DIRECTORY_FOR_INDEXING "%s.idx",a_daqEntryName);}

bool DoIndexing(const char* a_rootFileName,const char* a_daqEntryName,int a_startTime, int a_endTime,int a_startGenEvent,int a_endGenEvent)
{
    FILE* fpIndexFile;
    char vcBuffer[1024];

    GetIndexFileName(a_daqEntryName,vcBuffer,1024);
    fpIndexFile = fopen(vcBuffer,"a");
    if(!fpIndexFile){return false;}

    fprintf(fpIndexFile,"%d:%d,%d:%d,%s\n",a_startTime,a_startGenEvent,a_endTime,a_endGenEvent,a_rootFileName);
    fclose(fpIndexFile);
    return true;
}


bool GetListOfFilesForTimeInterval(const char* a_daqEntryName, time_t a_startTime, time_t a_endTime, ::std::vector< ::std::string >* a_pFiles )
{
    FILE* fpFile=nullptr;
    char vcBuffer[1024];
    int nBegTimeFile,nBegEvNumFile,nEndTimeFile,nEndEvNumFile;
    bool bBegFound(false);

    GetIndexFileName(a_daqEntryName,vcBuffer,1024);
    fpFile = fopen(vcBuffer,"r");
    if(!fpFile){
        MAKE_ERROR_GLOBAL("ERROR: \"%s\": INCORRECT BRANCH NAME",a_daqEntryName);
        return false;
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
                (*a_pFiles).push_back(vcBuffer);
            }
            else if(nBegTimeFile>a_endTime){break;}

        }
    }

    if(fpFile){fclose(fpFile);}
    return true;
}



bool GetListOfFilesForGenEventInterval(const char* a_daqEntryName, int64_t a_startGenEvent, int64_t a_endGenEvent, ::std::vector< ::std::string >* a_pFiles )
{
    FILE* fpFile=NEWNULLPTR;
    char vcBuffer[1024];
    int nBegTimeFile,nBegEvNumFile,nEndTimeFile,nEndEvNumFile;
    bool bBegFound(false);

    GetIndexFileName(a_daqEntryName,vcBuffer,1024);
    fpFile = fopen(vcBuffer,"r");
    if(!fpFile){
        MAKE_ERROR_GLOBAL("ERROR: \"%s\": INCORRECT BRANCH NAME",a_daqEntryName);
        return false;
    }

    while(fscanf(fpFile,"%d:%d,%d:%d,%1024s",&nBegTimeFile,&nBegEvNumFile,&nEndTimeFile,&nEndEvNumFile,vcBuffer)>0){
        if((nBegEvNumFile<=a_startGenEvent)&&(nEndEvNumFile>=a_startGenEvent)){bBegFound=true;}
        if(bBegFound){

            if(nEndEvNumFile>=a_endGenEvent){break;}
            (*a_pFiles).push_back(vcBuffer);

        }
    }

    if(fpFile){fclose(fpFile);}
    return true;
}


}}}}  // namespace pitz{ namespace daq{ namespace data{ namespace indexing{
