//
// file:        mex_simple_root_reader.cpp
//

#include "pitz/daq/data/engine/bypipe.hpp"
#include <string.h>
#include "pitz_daq_data_engine_branchitemprivate.hpp"
#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#else
#include <sys/select.h>
#endif

using namespace pitz::daq;

static bool ReadAllData(pitz::daq::data::engine::TypeReadPipe a_fpReader, int a_pipe, void* a_data, int a_nIoSize)
{
	char* pcBuffer = (char*)a_data;
	int nIoReturn;
	nIoReturn = (*a_fpReader)(a_pipe, pcBuffer, a_nIoSize);
	if (nIoReturn == a_nIoSize) { return true; }
	else if (nIoReturn < 0) { return false; }  // other pear disconnected
	else { // pipe is full
		a_nIoSize -= nIoReturn;
		while (a_nIoSize > 0) {
			pcBuffer += nIoReturn;
			nIoReturn = (*a_fpReader)(a_pipe, pcBuffer, a_nIoSize); // blocking call
			if (nIoReturn < 0) { return false; }  // other pear disconnected
			a_nIoSize -= nIoReturn;
		}
	}
	return true;
}

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
data::engine::ByPipe::ByPipe(TypeReadPipe a_fpReader)
	:
	byPipe::Base(byPipe::pipeType::Unknown),
	m_fpReader(a_fpReader)
{
    m_isRunning = 0;
}


data::engine::ByPipe::~ByPipe()
{
}


void data::engine::ByPipe::Stop()
{
    m_nWork = 0;
}


pitz::daq::callbackN::retType::Type data::engine::ByPipe::CheckReportPipes( ::fd_set* a_pipesSet, int* a_pReturn)
{
	if( FD_ISSET(m_pipes[byPipe::pipePurpose::Rep], a_pipesSet) ){
		::std::string strReport;
		int nStrLenPlus1;
		
		if( !ReadAllData(m_fpReader,m_pipes[byPipe::pipePurpose::Rep],&nStrLenPlus1,4) ){return pitz::daq::callbackN::retType::Stop;}
		strReport.resize(nStrLenPlus1);
		if( !ReadAllData(m_fpReader,m_pipes[byPipe::pipePurpose::Rep],const_cast<char*>(strReport.c_str()),nStrLenPlus1) ){return pitz::daq::callbackN::retType::Stop;}

		MAKE_DEBUG_THIS_RAW(rep,"",0,"%s",strReport.c_str());
	}
	else if( FD_ISSET(m_pipes[byPipe::pipePurpose::Err],a_pipesSet) ){
		::std::string strReport;
		int nStrLenPlus1;
		
		if( !ReadAllData(m_fpReader,m_pipes[byPipe::pipePurpose::Err],&nStrLenPlus1,4) ){return pitz::daq::callbackN::retType::Stop;}
		strReport.resize(nStrLenPlus1);
		if( !ReadAllData(m_fpReader,m_pipes[byPipe::pipePurpose::Err],const_cast<char*>(strReport.c_str()),nStrLenPlus1) ){return pitz::daq::callbackN::retType::Stop;}

		MAKE_DEBUG_THIS_RAW(err, "", 0, "%s", strReport.c_str());
	}
	else if ( FD_ISSET(m_pipes[byPipe::pipePurpose::Cntr],a_pipesSet) ) {
		if( !ReadAllData(m_fpReader,m_pipes[byPipe::pipePurpose::Cntr], a_pReturn,4) ){*a_pReturn=-1;}
		return pitz::daq::callbackN::retType::Stop;
	}
	else{
		*a_pReturn = -1;
		return pitz::daq::callbackN::retType::Stop;
	}

	return pitz::daq::callbackN::retType::Continue;
}


int data::engine::ByPipe::GetMultipleEntries(const char* a_rootFileName, ::common::List<engine::TBranchItemPrivate*>* a_pListBranches)
{
    fd_set rFds,eFds;
    int nIndex,nSelectReturn2;
    int nReturn =0;
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
    MAKE_REPORT_THIS(4,"a_rootFileName=%s",a_rootFileName);

    if(nBranchCountMin1<0){return 0;}

    m_isRunning = 1;

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

	m_nWork = 1;
	m_isRunning = 1;
    nReturn = 0;

    while( m_nWork && m_isRunning && (a_pListBranches->count()>0) && (clbkReturn!= daq::callbackN::retType::Stop) ){
        FD_ZERO(&rFds);
        FD_SET(m_pipes[byPipe::pipePurpose::Data], &rFds);
        FD_SET(m_pipes[byPipe::pipePurpose::Cntr], &rFds);
		FD_SET(m_pipes[byPipe::pipePurpose::Info], &rFds);
		FD_SET(m_pipes[byPipe::pipePurpose::Rep], &rFds);
		FD_SET(m_pipes[byPipe::pipePurpose::Err], &rFds);
        FD_ZERO(&eFds);
        FD_SET(m_pipes[byPipe::pipePurpose::Data], &eFds);
        FD_SET(m_pipes[byPipe::pipePurpose::Cntr], &eFds);
		FD_SET(m_pipes[byPipe::pipePurpose::Info], &eFds);
		FD_SET(m_pipes[byPipe::pipePurpose::Rep], &eFds);
		FD_SET(m_pipes[byPipe::pipePurpose::Err], &eFds);
        nSelectReturn2 = ::select(m_nMaxPipePlus1, &rFds, NULL,&eFds, NULL);

        if(nSelectReturn2<0){
            MAKE_ERROR_THIS("Binary finished current stage with error");
            nReturn = -1;
            goto returnPoint;
        }
        else if( FD_ISSET(m_pipes[byPipe::pipePurpose::Data], &rFds) ){

			if( !ReadAllData(m_fpReader,m_pipes[byPipe::pipePurpose::Data],&nIndex,4) ){goto returnPoint;}
			if( !ReadAllData(m_fpReader,m_pipes[byPipe::pipePurpose::Data],forRead.rawBuffer(),addInfo[nIndex].memorySize) ){goto returnPoint;}
			if( addInfo[nIndex].isStoped ){continue;}

            clbkReturn=(*this->clbk.m_multiEntries.readEntry)(m_clbkData, nIndex, forRead);
            if(clbkReturn==daq::callbackN::retType::StopForCurrent){
                if(addInfo[nIndex].item){
                    a_pListBranches->RemoveData(addInfo[nIndex].item);
                    addInfo[nIndex].item = NULL;
                }
                addInfo[nIndex].isStoped = 1;
            }
            else if(clbkReturn==daq::callbackN::retType::Stop){goto returnPoint;}

        }
        else if( FD_ISSET(m_pipes[byPipe::pipePurpose::Info], &rFds) ){

			if( !ReadAllData(m_fpReader,m_pipes[byPipe::pipePurpose::Info],&nIndex,4) ){goto returnPoint;}
			if( !ReadAllData(m_fpReader,m_pipes[byPipe::pipePurpose::Info],&entInfo,sizeof(EntryInfo)) ){goto returnPoint;}
            if( addInfo[nIndex].isStoped ){continue;}

            clbkReturn=(*this->clbk.m_multiEntries.infoGetter)(m_clbkData, nIndex, entInfo);
            if(clbkReturn==daq::callbackN::retType::StopForCurrent){
                if(addInfo[nIndex].item){
                    a_pListBranches->RemoveData(addInfo[nIndex].item);
                    addInfo[nIndex].item = NULL;
                }
                addInfo[nIndex].isStoped = 1;
            }
            else if(clbkReturn==daq::callbackN::retType::Stop){ goto returnPoint;}
            forRead.setBranchInfo(entInfo);

            addInfo[nIndex].memorySize=entInfo.memorySize();

        }
        else{clbkReturn=CheckReportPipes(&rFds,&nReturn);}
    }  // while(m_nWork && (nNumberRemained>0)){

    bStopped = false;

returnPoint:
    if( bStopped || (a_pListBranches->count()<1) || (m_nWork==0) || (m_isRunning==0) ){
        // todo: should be cleared the list remaining content
		//while(a_pListBranches->first())
		MAKE_REPORT_THIS(1, "Binary finished current stage\n");
        m_nWork = 0;
        m_isRunning = 0;
    }

    return nReturn;

}


int data::engine::ByPipe::GetEntriesInfo( const char* a_rootFileName)
{
    fd_set rFds,eFds;
    int nSelectReturn2;
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

    MAKE_REPORT_THIS(4,"a_rootFileName=%s",a_rootFileName);

    m_nWork = 1;
    m_isRunning = 1;

    while( m_nWork && m_isRunning && (clbkReturn!= daq::callbackN::retType::Stop) ){
		FD_ZERO(&rFds);
		FD_SET(m_pipes[byPipe::pipePurpose::Data], &rFds);
		FD_SET(m_pipes[byPipe::pipePurpose::Cntr], &rFds);
		FD_SET(m_pipes[byPipe::pipePurpose::Info], &rFds);
		FD_SET(m_pipes[byPipe::pipePurpose::Rep], &rFds);
		FD_SET(m_pipes[byPipe::pipePurpose::Err], &rFds);
		FD_ZERO(&eFds);
		FD_SET(m_pipes[byPipe::pipePurpose::Data], &eFds);
		FD_SET(m_pipes[byPipe::pipePurpose::Cntr], &eFds);
		FD_SET(m_pipes[byPipe::pipePurpose::Info], &eFds);
		FD_SET(m_pipes[byPipe::pipePurpose::Rep], &eFds);
		FD_SET(m_pipes[byPipe::pipePurpose::Err], &eFds);
		nSelectReturn2 = ::select(m_nMaxPipePlus1, &rFds, NULL, &eFds, NULL);

        if(nSelectReturn2<0){
            MAKE_ERROR_THIS("Binary finished current stage with error\n");
            nReturn = -1;
            goto returnPoint;
        }
        else if( FD_ISSET(m_pipes[byPipe::pipePurpose::Data], &rFds) ){

			if( !ReadAllData(m_fpReader,m_pipes[byPipe::pipePurpose::Data],&nNumberOfEntriesInTheFile,4) || (nNumberOfEntriesInTheFile<1) ){goto returnPoint;}

            clbkReturn=(*this->clbk.m_flInfo.numOfEntries)(m_clbkData, nNumberOfEntriesInTheFile);
            if( clbkReturn!=daq::callbackN::retType::Continue ){goto returnPoint;}
        }
        else if( FD_ISSET(m_pipes[byPipe::pipePurpose::Info], &rFds) ){

			if( !ReadAllData(m_fpReader, m_pipes[byPipe::pipePurpose::Info],&nIndex,4) || (nIndex>=nNumberOfEntriesInTheFile) ){goto returnPoint;}
			if( !ReadAllData(m_fpReader, m_pipes[byPipe::pipePurpose::Info],&entInfoAdv,sizeof(data::EntryInfo)) ){goto returnPoint;}
			if( !ReadAllData(m_fpReader, m_pipes[byPipe::pipePurpose::Info],entInfoAdv.ptr(),ADV_INF_INT_BYTES) ){goto returnPoint;}
			if( !ReadAllData(m_fpReader, m_pipes[byPipe::pipePurpose::Info],&nStrLen,4) ){goto returnPoint;}
			entInfoAdv.name.resize(nStrLen);
			if( !ReadAllData(m_fpReader, m_pipes[byPipe::pipePurpose::Info],const_cast<char*>(entInfoAdv.name.c_str()),nStrLen) ){goto returnPoint;}

            clbkReturn=(*this->clbk.m_flInfo.infoGetter)(m_clbkData, nIndex, entInfoAdv);
			if( clbkReturn!=daq::callbackN::retType::Continue ){goto returnPoint;}

        }
		else { clbkReturn = CheckReportPipes(&rFds,&nReturn); }
    }  // while(m_nWork && (nNumberRemained>0)){

returnPoint:
    m_isRunning = 0;
    return nReturn;
}

