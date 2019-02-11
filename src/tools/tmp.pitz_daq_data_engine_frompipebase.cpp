//
// file:        mex_simple_root_reader.cpp
//

#include "pitz/daq/data/engine/frompipebase.hpp"
#include <string.h>
#include "pitz_daq_data_engine_branchitemprivate.hpp"
#ifdef _WIN32
#else
#include <sys/select.h>
#endif

using namespace pitz::daq;

/***********************************************************************************************************************/
namespace pitz{ namespace daq { namespace data{ namespace engine{ namespace privateN{
struct SMultyGetterInfo{
    ::common::listN::ListItem<engine::TBranchItemPrivate*>* item;
    uint32_t  memorySize;
    uint32_t  isStoped : 1;
    SMultyGetterInfo():item(NULL),memorySize(0){this->isStoped=0;}
};
struct SInfoGetterInfo{
    uint64_t  isStoped : 1;
    SInfoGetterInfo(){this->isStoped=0;}
};
}}}}}  // namespace pitz{ namespace daq { namespace data{ namespace engine{ namespace privateN{


/***********************************************************************************************************************/
data::engine::FromPipeBase::FromPipeBase(TypeReadPipe a_fpReader)
    :
      m_fpReader(a_fpReader),
      m_pFilter(NULL)
{
    m_nPipesDataRd = m_nPipesCtrlRd = m_nPipesInfoRd = -1;
    m_nWork2 = 0;
    m_isRunning = 0;
}


data::engine::FromPipeBase::~FromPipeBase()
{
}


int data::engine::FromPipeBase::GetMultipleEntries(const char* a_rootFileName, ::common::List<engine::TBranchItemPrivate*>* a_pListBranches)
{
    fd_set rFds,eFds;
    int nIndex,nSelectReturn, nMaxPipePlus1;
    int nReturn =-1;
    const int numberOfBranches((int)a_pListBranches->count());
    const int nBranchCountMin1(numberOfBranches-1);
    ::std::string daqNameAll;
    ::std::vector< privateN::SMultyGetterInfo > addInfo;
    memory::Base    forRead;
    daq::callbackN::retType::Type clbkReturn(daq::callbackN::retType::Continue);
    data::EntryInfo entInfo;
    ::common::listN::ListItem<engine::TBranchItemPrivate*>* pBrListItem;
    bool bStopped(true);

    if(this->m_clbkType != callbackN::Type::MultiEntries){
        MAKE_ERROR_THIS("Proper callback to retrive multiple entries is not set\n");
        return -1;
    }

    MAKE_REPORT_THIS(0,"nBranchCountMin1=%d",nBranchCountMin1);

    if(nBranchCountMin1<0){return 0;}

    //m_vectorForData.resize(m_numberOfBranches);
    addInfo.resize(numberOfBranches);
    pBrListItem = a_pListBranches->first();

    for(nIndex=0;nIndex<nBranchCountMin1;++nIndex){
        MAKE_REPORT_THIS(0,"i=%d, branchName=%s",nIndex,pBrListItem->data->branchName.c_str());
        daqNameAll += pBrListItem->data->branchName.c_str();
        addInfo[nIndex].item = pBrListItem;
        daqNameAll += ";";
        pBrListItem = pBrListItem->next;
    }
    MAKE_REPORT_THIS(0,"i=%d, branchName=%s",nIndex,pBrListItem->data->branchName.c_str());
    daqNameAll += pBrListItem->data->branchName.c_str();
    addInfo[nBranchCountMin1].item = pBrListItem;
    MAKE_REPORT_THIS(0,"daqNameAll=%s",daqNameAll.c_str());

    if(m_nPipesDataRd<0){
        nSelectReturn=this->StartPipes(daqNameAll,a_rootFileName);
        if(nSelectReturn<0){
            MAKE_ERROR_THIS("Unable to open the pipes!");
            return nSelectReturn;
        }
        nMaxPipePlus1 = (m_nPipesDataRd>m_nPipesCtrlRd) && (m_nPipesDataRd>m_nPipesInfoRd) ?
                    (m_nPipesDataRd+1) : ( m_nPipesCtrlRd>m_nPipesInfoRd?(m_nPipesCtrlRd+1):(m_nPipesInfoRd+1));

        m_nWork2 = 1;
        m_isRunning = 1;
    }

    nReturn = 0;

    while(m_nWork2 && m_isRunning && (a_pListBranches->count()>0)){
        FD_ZERO(&rFds);
        FD_SET(m_nPipesDataRd, &rFds);
        FD_SET(m_nPipesCtrlRd, &rFds);
        if(m_nPipesInfoRd>0){FD_SET(m_nPipesInfoRd, &rFds);}
        FD_ZERO(&eFds);
        FD_SET(m_nPipesDataRd, &eFds);
        FD_SET(m_nPipesCtrlRd, &eFds);
        if(m_nPipesInfoRd>0){FD_SET(m_nPipesInfoRd, &eFds);}
        nSelectReturn = ::select(nMaxPipePlus1, &rFds, NULL,&eFds, NULL);

        if(nSelectReturn<0){
            MAKE_ERROR_THIS("Binary finished current stage with error");
            nReturn = -1;
            goto returnPoint;
        }
        else if( FD_ISSET(m_nPipesDataRd, &rFds) ){

            nSelectReturn = (*m_fpReader)(m_nPipesDataRd,&nIndex,4);
            if( (nSelectReturn!=4) || (nIndex<0)||(nIndex>=numberOfBranches)){
                MAKE_REPORT_THIS(5,"readReturn=%d, index=%d",nSelectReturn,nIndex);
                //if(nSelectReturn<0){goto returnPoint;}
                //else {continue;}
                goto returnPoint;
            }
            //MAKE_REPORT_THIS(5,"memorySize[%d]=%d",nIndex,(int)addInfo[nIndex].memorySize);
            if( (*m_fpReader)(m_nPipesDataRd,forRead.rawBuffer(),addInfo[nIndex].memorySize)!=addInfo[nIndex].memorySize){goto returnPoint;}
            if( addInfo[nIndex].isStoped){continue;}

            clbkReturn=(*this->clbk.m_multiEntries.readEntry)(m_clbkData, nIndex, forRead);
            if(clbkReturn==daq::callbackN::retType::StopForCurrent){
                if(addInfo[nIndex].item){
                    a_pListBranches->RemoveData(addInfo[nIndex].item);
                    addInfo[nIndex].item = NULL;
                }
                addInfo[nIndex].isStoped = 1;
            }
            else if(clbkReturn==daq::callbackN::retType::Stop){
                goto returnPoint;
            }

        }
        else if( FD_ISSET(m_nPipesInfoRd, &rFds) ){

            if( ( (*m_fpReader)(m_nPipesInfoRd,&nIndex,4)!=4) || (nIndex<0)||(nIndex>=numberOfBranches)){
                goto returnPoint;
            }
            if( (*m_fpReader)(m_nPipesInfoRd,&entInfo,sizeof(EntryInfo))!=sizeof(EntryInfo)){goto returnPoint ;}
            if( addInfo[nIndex].isStoped){continue;}

            clbkReturn=(*this->clbk.m_multiEntries.infoGetter)(m_clbkData, nIndex, entInfo);
            if(clbkReturn==daq::callbackN::retType::StopForCurrent){
                if(addInfo[nIndex].item){
                    a_pListBranches->RemoveData(addInfo[nIndex].item);
                    addInfo[nIndex].item = NULL;
                }
                addInfo[nIndex].isStoped = 1;
            }
            else if(clbkReturn==daq::callbackN::retType::Stop){
                goto returnPoint;
            }
            forRead.setBranchInfo(entInfo);

            addInfo[nIndex].memorySize=entInfo.memorySize();

        }
        else{
            MAKE_REPORT_THIS(1,"Binary finished current stage\n");
            goto returnPoint;
        }
    }  // while(m_nWork && (nNumberRemained>0)){

    bStopped = false;

returnPoint:
    if( bStopped || (a_pListBranches->count()<1) || (m_nWork2==0) || (m_isRunning==0) ){
        // todo: should be cleared the list remaining content
        StopPipes();
        m_nPipesDataRd = m_nPipesCtrlRd = m_nPipesInfoRd = -1;
        m_nWork2 = 0;
        m_isRunning = 0;
    }

    return nReturn;

}


int data::engine::FromPipeBase::GetEntriesInfo( const char* a_rootFileName)
{
    fd_set rFds,eFds;
    int nSelectReturn;
    int nMaxPipePlus1;
    int nReturn(0);
    int nStrLen;
    int nIndex;
    int nNumberOfEntriesInTheFile=-1;
    engine::EntryInfoAdv entInfoAdv;
    daq::callbackN::retType::Type clbkReturn(daq::callbackN::retType::Continue);

    if(this->m_clbkType != callbackN::Type::Info){
        MAKE_ERROR_THIS("Proper callback to retrive all entries info is not set\n");
        return -1;
    }

    nSelectReturn=this->StartPipes("",a_rootFileName);
    if(nSelectReturn<0){
        MAKE_ERROR_THIS("Unable to open the pipes!");
        return nSelectReturn;
    }
    nMaxPipePlus1 = (m_nPipesDataRd>m_nPipesCtrlRd) && (m_nPipesDataRd>m_nPipesInfoRd) ?
                (m_nPipesDataRd+1) : ( m_nPipesCtrlRd>m_nPipesInfoRd?(m_nPipesCtrlRd+1):(m_nPipesInfoRd+1));

    m_nWork2 = 1;
    m_isRunning = 1;

    while(m_nWork2 && m_isRunning){
        FD_ZERO(&rFds);
        FD_SET(m_nPipesDataRd, &rFds);
        FD_SET(m_nPipesCtrlRd, &rFds);
        if(m_nPipesInfoRd>0){FD_SET(m_nPipesInfoRd, &rFds);}
        FD_ZERO(&eFds);
        FD_SET(m_nPipesDataRd, &eFds);
        FD_SET(m_nPipesCtrlRd, &eFds);
        if(m_nPipesInfoRd>0){FD_SET(m_nPipesInfoRd, &eFds);}
        nSelectReturn = ::select(nMaxPipePlus1, &rFds, NULL,&eFds, NULL);

        if(nSelectReturn<0){
            MAKE_ERROR_THIS("Binary finished current stage with error\n");
            nReturn = -1;
            goto returnPoint;
        }
        else if( FD_ISSET(m_nPipesDataRd, &rFds) ){
            if( (*m_fpReader)(m_nPipesDataRd,&nNumberOfEntriesInTheFile,4)!=4 ){
                if((nIndex<0)||(nIndex!=(nNumberOfEntriesInTheFile-1))){nReturn = -1;}
                goto returnPoint;
            }
            clbkReturn=(*this->clbk.m_flInfo.numOfEntries)(m_clbkData, nNumberOfEntriesInTheFile);
            if(clbkReturn==daq::callbackN::retType::StopForCurrent){goto returnPoint;}
            else if(clbkReturn==daq::callbackN::retType::Stop){goto returnPoint;}
        }
        else if( FD_ISSET(m_nPipesInfoRd, &rFds) ){

            if( (*m_fpReader)(m_nPipesInfoRd,&nIndex,4)!=4 ){
                nReturn = -1; goto returnPoint;
            }
            if( (nIndex<0) || (nIndex>=nNumberOfEntriesInTheFile) ){
                nReturn = -1; goto returnPoint;
            }
            if( (*m_fpReader)(m_nPipesInfoRd,&entInfoAdv,sizeof(data::EntryInfo))!=sizeof(data::EntryInfo) ){continue ;}
            if( (*m_fpReader)(m_nPipesInfoRd,entInfoAdv.ptr(),ADV_INF_INT_BYTES)!=ADV_INF_INT_BYTES ){continue ;}
            if( ( (*m_fpReader)(m_nPipesInfoRd,&nStrLen,4)!=4)||(nStrLen<1) ){continue ;}
            entInfoAdv.name.resize(nStrLen);
            if( (*m_fpReader)(m_nPipesInfoRd,const_cast<char*>(entInfoAdv.name.c_str()),nStrLen)!=nStrLen ){continue ;}

            clbkReturn=(*this->clbk.m_flInfo.infoGetter)(m_clbkData, nIndex, entInfoAdv);
            if(clbkReturn==daq::callbackN::retType::StopForCurrent){
                // ?
            }
            else if(clbkReturn==daq::callbackN::retType::Stop){goto returnPoint;}

        }
        else{
            MAKE_REPORT_THIS(1,"Binary finished current stage\n");
            goto returnPoint;
        }
    }  // while(m_nWork && (nNumberRemained>0)){

returnPoint:
    StopPipes();
    return nReturn;
}

