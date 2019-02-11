#include "browsing_funcs.h"
#include <vector>
#include <TFile.h>
#include <TH1F.h>
#include <TNtuple.h>
#include <TKey.h>
#include <TROOT.h>
#include <stdlib.h>

#define _INDEX_FILE_DIR_        "//afs/ifh.de/group/pitz/doocs/data/DAQdata/INDEX"
#define _INDEX_FL_NAME_MX_LEN_  1024
#define _ACCURACY_              10

int g_nLoglevel = 1;

template <typename Type>
class NewVector : private std::vector<Type>
{
    public:
    void push_back_new(int a_logLevel,const char* a_srcFile,int a_line,const char* a_fnc,const Type& a_tData){
        if(a_logLevel<=g_nLoglevel){
            printf("push_back:  fl:\"%s\",ln:%d,fnc:%s\n",_FILE_PATH(a_srcFile),a_line,a_fnc);
        }
        std::vector<Type>::push_back(a_tData);
    }
    size_t size()const {return std::vector<Type>::size();}
    Type& operator [](size_t a_index){return std::vector<Type>::operator[](a_index);}
    void clear(){std::vector<Type>::clear();}
};

#define VECT_TYPE               NewVector
#define PUSH_BACK(__data__)     push_back_new(1,__FILE__,__LINE__,__FUNCTION__,(__data__))


static int ReadEntries(const char* a_cpcDebugRootFlName,
                       char* a_pcFinalRootFileName, int a_nFinalRtFlNmLen,
                       int& a_nNumberRemained,
                       std::vector<BranchItem>* a_pNonFoundedIndexes,
                       int a_nCurIndex2,struct19& a_StructForRoot,
                       TmEvPointers* a_tmp,
                       struct19* a_pBufferForData, int a_nEventsMaxCount);

static inline int FindRootFiles(const char* a_cpcBranchName,int (*a_fpErrFunc)(const char*,...),
                                const TmEvPointers& a_tm, VECT_TYPE<std::string>&  a_RootFiles,
                                char* a_pcBuffer,int a_nBuffLen);

#ifdef __cplusplus
extern "C"
{
#endif

int ReadDataFromDCacheByEv2(
                       int a_nEventsMaxCount, struct19* a_pBufferForData,
                       int (*a_fpErrFunc)(const char*,...),
                       STmEvPointers* a_pPointers,
                       int a_nNumberOfSpectra, const BranchItem* a_pBranchItems,
                       void* a_pNotFoundedBranches ,
                       char* a_pcBufferForNames,
                       int a_nBuffLen,
                       void* a_pRootFiles)
{
    int nReadedEntries = 0;
    size_t unNumbOfFiles,nIndexCurFile;
    std::vector<BranchItem>* pNonFoundedIndexes;
    int nIndexSpectr;
    struct19 aStruct;

    if(a_pNotFoundedBranches){pNonFoundedIndexes = (std::vector<BranchItem>*)a_pNotFoundedBranches;}
    else
    {
        pNonFoundedIndexes = new std::vector<BranchItem>;
        for(nIndexSpectr=0;nIndexSpectr<a_nNumberOfSpectra;++nIndexSpectr)
        {
            pNonFoundedIndexes->push_back(a_pBranchItems[nIndexSpectr]);
        }
    }
    std::vector<BranchItem>& vNonFoundedIndexes = *pNonFoundedIndexes;

    char* pcBuffer;
    int nBuffLen;
    if(a_pcBufferForNames)
    {
        pcBuffer = a_pcBufferForNames;
        nBuffLen = a_nBuffLen>0 ? a_nBuffLen : _DEF_BUFFER_LEN_IND_FILES_;
    }
    else
    {
        pcBuffer = (char*)alloca(_DEF_BUFFER_LEN_IND_FILES_);
        nBuffLen = _DEF_BUFFER_LEN_IND_FILES_;
    }

    VECT_TYPE<std::string>*  pRootFiles;
    if(a_pRootFiles){pRootFiles = (VECT_TYPE<std::string>*)a_pRootFiles;}
    else {pRootFiles = new VECT_TYPE<std::string>;}
    VECT_TYPE<std::string>&  aRootFiles = *pRootFiles;

    int nFoundedFR;

    int nCurIndex2 ;
    int nNumberRemained = vNonFoundedIndexes.size();
    int nFromEvent(a_pPointers->m_nBegBufFile);
    int nToEvent(a_pPointers->m_nEndBufFile);

    TmEvPointers& aPointers = *((TmEvPointers*)a_pPointers);

    aPointers.SetType(SEARCH_TYPE_BY_EVENT,nFromEvent,nToEvent);

    //////////////////////////////////////
    const char* cpcDebugRootFlName;
    char vcDebugFileName[512];

    while(nNumberRemained)
    {
        nCurIndex2 = vNonFoundedIndexes[0].m_nIndex;
        unNumbOfFiles = aRootFiles.size();

        for(nIndexCurFile=0;nIndexCurFile<unNumbOfFiles;++nIndexCurFile)
        {
            cpcDebugRootFlName = aRootFiles[nIndexCurFile].c_str();
            nReadedEntries += ReadEntries(  cpcDebugRootFlName, vcDebugFileName, 512,nNumberRemained,
                                            pNonFoundedIndexes, nCurIndex2,aStruct,&aPointers,
                                            a_pBufferForData,a_nEventsMaxCount);

        } // end  for(nIndexCurFile=0;nIndexCurFile<unNumbOfFiles;++nIndexCurFile)

        if(nNumberRemained)
        {
            nFoundedFR = FindRootFiles(vNonFoundedIndexes[0].m_cpcBranchName,a_fpErrFunc,aPointers,
                                    aRootFiles,pcBuffer,nBuffLen);

            if(nFoundedFR<=0)
            {
                // report
                if(!a_pNotFoundedBranches){delete pNonFoundedIndexes;}
                if(!a_pRootFiles){delete pRootFiles;}
                return -1;
            }

        } // end  if(nNumberRemained)

    } // end  while(nNumberRemained)

    if(!a_pNotFoundedBranches){delete pNonFoundedIndexes;}
    if(!a_pRootFiles){delete pRootFiles;}
    return nReadedEntries;
}


int ReadDataFromDCache(int a_nEventsMaxCount, struct19* a_pBufferForData,
                       int (*a_fpErrFunc)(const char*,...),
                       int a_fromTime, int a_toTime,
                       int a_nNumberOfSpectra, const BranchItem* a_pBranchItems)
{
    char vcBuffer[_INDEX_FL_NAME_MX_LEN_];
    VECT_TYPE<std::string>  aRootFiles;

    std::vector<BranchItem> vNonFoundedIndexes;
    for( int i(0);i<a_nNumberOfSpectra;++i)
    {
        vNonFoundedIndexes.push_back(a_pBranchItems[i]);
    }

    struct19 aStructForRoot;

    TmEvPointers aPointers(SEARCH_TYPE_BY_TIME,a_fromTime,a_toTime);

    char vcDebugFileName[512];
    int nNumberRemained(a_nNumberOfSpectra);
    int nReturned,nRet;

    if( FindRootFiles(vNonFoundedIndexes[0].m_cpcBranchName,a_fpErrFunc,
                      aPointers,aRootFiles,vcBuffer,_INDEX_FL_NAME_MX_LEN_)>0 )
    {
        _DEBUG_APP_(1,"size=%d\n",(int)aRootFiles.size());
        nReturned = ReadEntries(aRootFiles[0].c_str(),vcDebugFileName, 512,nNumberRemained,
                                &vNonFoundedIndexes, 0,aStructForRoot,&aPointers,
                                a_pBufferForData,a_nEventsMaxCount);

        aRootFiles.clear();

        nRet = ReadDataFromDCacheByEv2(   a_nEventsMaxCount, a_pBufferForData,
                                          a_fpErrFunc,&aPointers,
                                          a_nNumberOfSpectra, a_pBranchItems,
                                          &vNonFoundedIndexes ,vcBuffer,_INDEX_FL_NAME_MX_LEN_,
                                          &aRootFiles);

        nReturned = nRet>=0 ? nReturned + nRet : nRet;
    }
    else
    {
        _DEBUG_APP_(1," ");
        nReturned = 0;
    }

    return nReturned;
}


#ifdef __cplusplus
}
#endif


static inline int FindRootFiles(const char* a_cpcBranchName,int (*a_fpErrFunc)(const char*,...),
                                const TmEvPointers& a_tm, VECT_TYPE<std::string>&  a_RootFiles,
                                char* a_pcBuffer,int a_nBuffLen)
{

    snprintf(a_pcBuffer,a_nBuffLen,"%s/%s.idx",_INDEX_FILE_DIR_,a_cpcBranchName );

    a_RootFiles.clear();

    FILE* fpFile = fopen(a_pcBuffer,"r");
    if(!fpFile)
    {
        (*a_fpErrFunc)("Line(%d): \"%s\": INCORRECT BRANCH NAME!\n",__LINE__,a_cpcBranchName);
        return _INCORRECT_BRANCH_NAME_;
    }

    int nEvJump;
    int nBegFoundedFR = 0;
    a_pcBuffer[0] = 0;
    int nFoundedFR = 0;

    while(fscanf(fpFile,"%d:%d,%d:%d,%s",&a_tm.m_nBegTimeFile,&a_tm.m_nBegBufFile,&a_tm.m_nEndTimeFile,&a_tm.m_nEndBufFile,a_pcBuffer)>0)
    {
        nEvJump = *a_tm.m_pnEndFile - *a_tm.m_pnBegFile;
        if(nEvJump<1 || nEvJump>1728000) continue;  // 1728000 corresponds to 2 days (for events)
        //if( ++nDebugLineNumber == 733 )continue;  // end 20 days for seconds
        if( !nBegFoundedFR )
        {
            //_DEBUG_APP_(1, "tm.m_nFromIn=%d,*a_tm.m_pnBegFile=%d",a_tm.m_nFromIn,*a_tm.m_pnBegFile);
            if(a_tm.m_nFromIn>=*a_tm.m_pnBegFile && a_tm.m_nFromIn<=*a_tm.m_pnEndFile)
            {
                //_DEBUG_APP_(1, "*tm.m_cpnBegTimeIn=%d,tm.m_nBegTimeFile=%d,tm.m_nFromIn=%d,tm.m_nBegTimeFile=%d,*a_tm.m_pnBegFile=%d",
                //            a_tm.m_cpnBegTimeIn?*a_tm.m_cpnBegTimeIn:-1,a_tm.m_nBegTimeFile,a_tm.m_nFromIn,a_tm.m_nBegTimeFile,*a_tm.m_pnBegFile);
                _DEBUG_APP_(1, "tm.m_nFromIn=%d,*a_tm.m_pnBegFile=%d",a_tm.m_nFromIn,*a_tm.m_pnBegFile);
                if(a_tm.m_cpnBegTimeIn && ( ((*a_tm.m_cpnBegTimeIn)-a_tm.m_nBegTimeFile)>172800)){continue;}
                nBegFoundedFR = 1;
                a_RootFiles.PUSH_BACK(a_pcBuffer);
                _DEBUG_APP_(1,
                            "begFounded=%s, *tm.m_pnBeg=%d, *tm.m_pnEnd=%d, tm.m_nFrom0=%d,tm.m_nTo0=%d"
                            "\n!!!!!!! fileName=\"%s\"",
                            nBegFoundedFR?"true":"false",*a_tm.m_pnBegFile,*a_tm.m_pnEndFile,a_tm.m_nFromIn,a_tm.m_nToIn,
                            a_pcBuffer);
                if(/*a_toEvent>nBegBuf && */a_tm.m_nToIn <= *a_tm.m_pnEndFile) // if a_fromEvent>=nBegBuf   then for shure
                                                                // a_toEvent>nBegBuf
                {
                    nFoundedFR = 1;
                    break;
                } // end if(a_toEvent>nBegBuf && a_toEvent<=nEndBuf)

            } // end if(a_fromEvent>=nBegBuf && a_fromEvent<nEndBuf)

        } // end  if( !nBegFoundedFR )
        else // fur  if( !nBegFoundedFR )
        {
            _DEBUG_APP_(1,"begFounded=%s, *tm.m_pnBeg=%d, *tm.m_pnEnd=%d, tm.m_nFrom0=%d,tm.m_nTo0=%d",
                        nBegFoundedFR?"true":"false",*a_tm.m_pnBegFile,*a_tm.m_pnEndFile,a_tm.m_nFromIn,a_tm.m_nToIn);
            a_RootFiles.PUSH_BACK(a_pcBuffer);
            if(/*a_toEvent>nBegBuf &&*/ a_tm.m_nToIn <= *a_tm.m_pnEndFile )
            {
                nFoundedFR = 1;
                break;
            } // end  if(a_toEvent>nBegBuf && a_toSec<=nEndBuf)
        }

    } // end  while(fscanf(fpFile,"%d %d %d %d %s",&nBegTime,&nBegBuf,&nEndTime, &nEndBuf,vcIndexFileName))

    fclose(fpFile);

    return nFoundedFR;
}

static int ReadEntries(const char* a_cpcDebugRootFlName,
                       char* a_pcFinalRootFileName, int a_nFinalRtFlNmLen,
                       int& a_nNumberRemained,
                       std::vector<BranchItem>* a_pNonFoundedIndexes,
                       int a_nCurIndex2,struct19& a_StructForRoot,
                       TmEvPointers* a_tmp,
                       struct19* a_pBufferForData, int a_nEventsMaxCount)
{
    std::vector<BranchItem>& vNonFoundedIndexes = *a_pNonFoundedIndexes;
    TTree *tree;
    TBranch *branch;
    int nNumOfEntries,nIndexEntry;
    int nReadedEntries(0);
    //int nFromEvent = a_pnFromEvent ? *a_pnFromEvent : 0;
    //int nToEvent = a_pnToEvent ? *a_pnToEvent : 0;
    int nArrayIndex;

    snprintf(a_pcFinalRootFileName,a_nFinalRtFlNmLen,"dcap://dcap:22125/pnfs/ifh.de%s",a_cpcDebugRootFlName);
    printf("!!!!! fname=%s\n",a_pcFinalRootFileName);
    //snprintf(a_pcFinalRootFileName, a_nFinalRtFlNmLen,"%s",a_cpcDebugRootFlName);
    _FNC_START("nFromEvent=%d, fileName=\"%s\"",a_tmp->m_nFromIn,a_pcFinalRootFileName);
    TFile* tFile = TFile::Open(a_pcFinalRootFileName,"READ");
    if(!tFile || !tFile->IsOpen())
    {
        // report
        nReadedEntries = -2;
        goto returnPoint;
    }

    BranchItem debugBranchItem2;

    tFile->ls("-d");
    gFile = tFile;

    if(a_tmp->m_cpnBegBufIn) {a_tmp->m_pnBegFile = &(a_StructForRoot.buffer);}
    else {a_tmp->m_pnBegFile = &(a_StructForRoot.time);}

    //for( nIndexSpectr=0; nIndexSpectr<nNumberRemained;/*++nIndexSpectr*/)
    for( int nIndexSpectr(0); nIndexSpectr<a_nNumberRemained;++nIndexSpectr)
    {
        debugBranchItem2 = vNonFoundedIndexes[nIndexSpectr];
        tree = (TTree *)tFile->Get(vNonFoundedIndexes[nIndexSpectr].m_cpcTreeName);
        if (!tree)
        {
            if(vNonFoundedIndexes[nIndexSpectr].m_nIndex==a_nCurIndex2)
            {
                //cerr << "can't get tree >" << tree << "< from >" << file << "<" << endl;
                // report
                delete tFile;
                nReadedEntries = -3;
                goto returnPoint;
            }
            continue;

        } // end  if (!tree)

        branch = tree->GetBranch(vNonFoundedIndexes[nIndexSpectr].m_cpcBranchName);
        if (!branch)
        {
            if(vNonFoundedIndexes[nIndexSpectr].m_nIndex==a_nCurIndex2)
            {
                //cerr << "can't get branch  " << endl;
                // report
                delete tFile;
                nReadedEntries = -4;
                goto returnPoint;
            }
            continue;

        } // end  if (!branch)

        nNumOfEntries = branch->GetEntries();
        branch->SetAddress(&a_StructForRoot);

        for(nIndexEntry=0;nIndexEntry<nNumOfEntries;++nIndexEntry)
        {
            //_DEBUG_APP_(1,"++++++++++++++++++nIndexEntry=%d,a_StructForRoot.buffer=%d",nIndexEntry,a_StructForRoot.buffer);
            branch->GetEntry(nIndexEntry);

            if(!vNonFoundedIndexes[nIndexSpectr].m_nBegFounded)
            {
                if( (*(a_tmp->m_pnBegFile)) >= a_tmp->m_nFromIn)
                {
                    vNonFoundedIndexes[nIndexSpectr].m_nBegFounded = 1;
                    a_tmp->m_nBegBufFile = a_StructForRoot.buffer;
                    a_tmp->m_nBegTimeFile = a_StructForRoot.time;

                    nArrayIndex = a_StructForRoot.buffer - a_tmp->m_nBegBufFile; // =0
                    //TEST_AND_FILL3(a_Struct);

                    if(nArrayIndex<a_nEventsMaxCount)
                    { // TEST_AND_FILL3
                        BranchItem& aBranchItem = vNonFoundedIndexes[nIndexSpectr];
                        //int nArrayIndex = a_Struct.buffer - s_fromEvent ;

                        int nIndex = _GET_INDEX_(aBranchItem.m_nIndex,nArrayIndex,a_nEventsMaxCount);
                        _DEBUG_APP_(1,"!!!!!! index=%d,aBranchItem.m_nIndex=%d",nIndex,aBranchItem.m_nIndex);
                        //printf("%s:  spectrIndex = %d, arrayIndex = %d, lineIndex = %d\n",aBranchItem.m_cpcBranchName,aBranchItem.m_nIndex,nArrayIndex,nIndex);
                        memcpy(&a_pBufferForData[nIndex],&a_StructForRoot,sizeof(struct19));
                        ++nReadedEntries;

                    } // end // TEST_AND_FILL3

                }  // if( (*(a_tmp->m_pnBegFile)) >= a_tmp->m_nFromIn)

            } // if(!vNonFoundedIndexes[k].m_nBegFounded)
            else
            {
                nArrayIndex = a_StructForRoot.buffer - a_tmp->m_nBegBufFile;
                _DEBUG_APP_(2, "!!!!! nArrayIndex=%d,a_tmp->m_nBegBufFile=%d,a_StructForRoot.buffer=%d,nIndexEntry=%d",
                            nArrayIndex,a_tmp->m_nBegBufFile,a_StructForRoot.buffer,nIndexEntry);
                //TEST_AND_FILL3(a_Struct);

                //if( (nArrayIndex<a_nEventsMaxCount) && ( (*(a_tmp->m_pnEndFile))<=a_tmp->m_nToIn )  )
                if( (nArrayIndex<a_nEventsMaxCount)   )
                { // TEST_AND_FILL3

                    a_tmp->m_nEndBufFile = a_StructForRoot.buffer;
                    a_tmp->m_nEndTimeFile = a_StructForRoot.time;

                    BranchItem& aBranchItem = vNonFoundedIndexes[nIndexSpectr];
                    //int nArrayIndex = a_Struct.buffer - s_fromEvent ;

                    int nIndex = _GET_INDEX_(aBranchItem.m_nIndex,nArrayIndex,a_nEventsMaxCount);
                    //printf("%s:  spectrIndex = %d, arrayIndex = %d, lineIndex = %d\n",aBranchItem.m_cpcBranchName,aBranchItem.m_nIndex,nArrayIndex,nIndex);
                    memcpy(&a_pBufferForData[nIndex],&a_StructForRoot,sizeof(struct19));
                    ++nReadedEntries;                                       \

                    //return false;
                } // if( (nArrayIndex<a_nEventsMaxCount) && ( (*(a_tmp->m_pnEndFile))<=a_tmp->m_nToIn )  )end // TEST_AND_FILL3
                else
                {
                    vNonFoundedIndexes.erase(vNonFoundedIndexes.begin()+(nIndexSpectr--));
                    --a_nNumberRemained;
                    break;
                }
            } // else for // if(!vNonFoundedIndexes[k].m_nBegFounded)

        } // end  for(nIndexEntry=0;nIndexEntry<nNumOfEntries;++nIndexEntry)

    } // end  for( nIndexSpectr=0; nIndexSpectr<nNumberRemained;++nIndexSpectr)

    delete tFile ;

returnPoint:

    _FNC_END("fileName=\"%s\"",a_pcFinalRootFileName);
    return nReadedEntries;
}

