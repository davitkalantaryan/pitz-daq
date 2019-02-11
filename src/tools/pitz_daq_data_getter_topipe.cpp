//
// file:        mex_simple_root_reader.cpp
//

#include "pitz/daq/data/getter/topipe.hpp"
#include "pitz_daq_data_engine_branchitemprivate.hpp"
#include <signal.h>

#ifdef _WIN32
#else    // #ifdef _WIN32
#ifndef closesocket
#define closesocket close
#endif
#if !defined(SOCKET) && !defined(SOCKET_defined)
#define SOCKET_defined
typedef int SOCKET;
#endif
#endif   // #ifdef _WIN32

#define SOCKET_ERROR    -2018

using namespace pitz::daq;

static volatile bool s_isSigPipe = false;
static void SigPipeHandler(int){s_isSigPipe=true;}


static bool WriteAllData(pitz::daq::data::getter::TypeWritePipe a_fpWriter, int a_pipe,const void* a_data, int a_nIoSize)
{
    const char* cpcBuffer = (const char*)a_data;
    int nIoReturn;
    nIoReturn=(*a_fpWriter)(a_pipe,cpcBuffer,a_nIoSize);
    if(nIoReturn==a_nIoSize){ return true ;}
    else if(nIoReturn<0){return false;}  // other pear disconnected
    else { // pipe is full
        a_nIoSize -= nIoReturn;
        while(a_nIoSize>0){
            cpcBuffer += nIoReturn;
            nIoReturn = (*a_fpWriter)(a_pipe,cpcBuffer,a_nIoSize); // blocking call
            if(nIoReturn<0){return false;}  // other pear disconnected
            a_nIoSize -= nIoReturn;
        }
    }
    return true;
}


data::getter::ToPipe::ToPipe( byPipe::pipeType::Type a_pipeType,TypeWritePipe a_fpWriter )
    :
      byPipe::Base(a_pipeType),
      m_fpWriter(a_fpWriter)
{
    struct sigaction  sigusr1Action;

    sigusr1Action.sa_flags = 0;
    sigemptyset(&sigusr1Action.sa_mask);
    sigusr1Action.sa_restorer = NULL;
    sigusr1Action.sa_handler = &SigPipeHandler;
    sigaction(SIGPIPE,&sigusr1Action,NULL);

    m_pFilter = NULL;
}


data::getter::ToPipe::~ToPipe(  )
{
}


int data::getter::ToPipe::GetEntriesInfo( const char* a_rootFileName)
{
    int nReturn = getter::BaseTmp<getter::WithIndexer,engine::Local>::GetEntriesInfo(a_rootFileName);
    WriteAllData(m_fpWriter,m_pipes[byPipe::pipePurpose::Cntr],&nReturn,4);
    return nReturn;
}


int data::getter::ToPipe::GetMultipleEntries( const char* a_rootFileName, const ::std::vector< ::std::string >& a_branchNames)
{
    int nReturn = getter::BaseTmp<getter::WithIndexer,engine::Local>::GetMultipleEntries(a_rootFileName,a_branchNames);
    WriteAllData(m_fpWriter,m_pipes[byPipe::pipePurpose::Cntr],&nReturn,4);
    return nReturn;
}


int data::getter::ToPipe::GetMultipleEntriesTI( const ::std::vector< ::std::string >& a_branchNames, int a_startTime, int a_endTime)
{
    int nReturn = getter::BaseTmp<getter::WithIndexer,engine::Local>::GetMultipleEntriesTI(a_branchNames,a_startTime,a_endTime);
    WriteAllData(m_fpWriter,m_pipes[byPipe::pipePurpose::Cntr],&nReturn,4);
    return nReturn;
}


int data::getter::ToPipe::SetPipes( const char* a_cpcPipesString )
{
    char* cpcPrivatePipeTmp;

    if(m_pipes[0]>=0){MAKE_WARNING_THIS("Pipes already assigned!");}

    MAKE_DEBUG_THIS_RAW(rep,"",1,"fl:%s,ln:%d -> ",_FILE_FROM_PATH_(__FILE__),__LINE__);
    m_pipes[0] = strtol(a_cpcPipesString,&cpcPrivatePipeTmp,10);
    MAKE_DEBUG_THIS_RAW(rep,"",1,"pipe[0]=%d",m_pipes[0]);
    for(int i(1);i<byPipe::pipePurpose::Count;++i){
        if(cpcPrivatePipeTmp){m_pipes[i] = strtol(cpcPrivatePipeTmp+1,&cpcPrivatePipeTmp,10);}
        else{
            MAKE_DEBUG_THIS_RAW(rep,"",1,"\n");
            MAKE_ERROR_THIS("Unable to get all pipes");
            return -1;
        }
        MAKE_DEBUG_THIS_RAW(rep,"",1,",pipe[%d]=%d",i,m_pipes[i]);

    }
    MAKE_DEBUG_THIS_RAW(rep,"",1,"\n",m_pipes[0]);

    return 0;
}


void data::getter::ToPipe::ClosePipes()
{
    int nClose(0);
    int (*TypecloseSocket)(SOCKET) = m_pipeType==byPipe::pipeType::Socket ? closesocket : close;

    MAKE_REPORT_THIS(1,"Closing the pipes");

    if(m_pipes[byPipe::pipePurpose::Cntr]>0){
        WriteAllData(m_fpWriter,m_pipes[byPipe::pipePurpose::Cntr],&nClose,4);
        SleepMsInt(10);
    }

    for(int i(0);i<byPipe::pipePurpose::Count;++i){
        if(m_pipes[i]>=0){
            (*TypecloseSocket)(m_pipes[i]);
            m_pipes[i]=-1;
        }
    }

}


::pitz::daq::callbackN::retType::Type data::getter::ToPipe::NumberOfEntries(int a_number)
{
    int nIoSize = 4;

    if(s_isSigPipe){s_isSigPipe=false;return daq::callbackN::retType::Stop;}
    if(m_pipes[byPipe::pipePurpose::Data]<0){return daq::callbackN::retType::Stop;}

    return WriteAllData(m_fpWriter,m_pipes[byPipe::pipePurpose::Data],&a_number,nIoSize) ? daq::callbackN::retType::Continue : daq::callbackN::retType::Stop;
}


::pitz::daq::callbackN::retType::Type data::getter::ToPipe::ReadEntry(int a_index, const memory::Base& a_mem)
{
    int nIoSize = 4;

    if(s_isSigPipe){s_isSigPipe=false;return daq::callbackN::retType::Stop;}
    if(m_pipes[byPipe::pipePurpose::Data]<0){return daq::callbackN::retType::Stop;}

    if(!WriteAllData(m_fpWriter,m_pipes[byPipe::pipePurpose::Data],&a_index,nIoSize)){return daq::callbackN::retType::Stop;}

    nIoSize = (int)a_mem.memorySize();
    MAKE_REPORT_THIS(9,"index=%d, memSize=%d",a_index,nIoSize);
    return WriteAllData(m_fpWriter,m_pipes[byPipe::pipePurpose::Data],a_mem.rawBuffer(),nIoSize) ? daq::callbackN::retType::Continue : daq::callbackN::retType::Stop;
}


::pitz::daq::callbackN::retType::Type data::getter::ToPipe::InfoGetter(int a_index, const data::EntryInfo& a_info)
{
    int nIoSize = 4;

    if(s_isSigPipe){s_isSigPipe=false;return daq::callbackN::retType::Stop;}
    if(m_pipes[byPipe::pipePurpose::Info]<0){return daq::callbackN::retType::Stop;}

    if(!WriteAllData(m_fpWriter,m_pipes[byPipe::pipePurpose::Info],&a_index,nIoSize)){return daq::callbackN::retType::Stop;}

    nIoSize = (int)sizeof(data::EntryInfo);
    return WriteAllData(m_fpWriter,m_pipes[byPipe::pipePurpose::Info],&a_info,nIoSize) ? daq::callbackN::retType::Continue : daq::callbackN::retType::Stop;
}


::pitz::daq::callbackN::retType::Type data::getter::ToPipe::AdvInfoGetter(int a_index, const engine::EntryInfoAdv& a_info)
{
    int nIoSize = 4;
    int nStrLen;

    if(s_isSigPipe){s_isSigPipe=false;return daq::callbackN::retType::Stop;}
    if(m_pipes[byPipe::pipePurpose::Info]<0){return daq::callbackN::retType::Stop;}

    if(!WriteAllData(m_fpWriter,m_pipes[byPipe::pipePurpose::Info],&a_index,nIoSize)){return daq::callbackN::retType::Stop;}

    nIoSize = (int)sizeof(data::EntryInfo);
    if(!WriteAllData(m_fpWriter,m_pipes[byPipe::pipePurpose::Info],&a_info,nIoSize)){return daq::callbackN::retType::Stop;}

    nIoSize = ADV_INF_INT_BYTES;
    if(!WriteAllData(m_fpWriter,m_pipes[byPipe::pipePurpose::Info],a_info.ptr(),nIoSize)){return daq::callbackN::retType::Stop;}

    nStrLen = a_info.name.size();
    nIoSize = 4;
    if(!WriteAllData(m_fpWriter,m_pipes[byPipe::pipePurpose::Info],&nStrLen,nIoSize)){return daq::callbackN::retType::Stop;}

    nIoSize =  nStrLen;
    return WriteAllData(m_fpWriter,m_pipes[byPipe::pipePurpose::Info],a_info.name.c_str(),nIoSize) ? daq::callbackN::retType::Continue : daq::callbackN::retType::Stop;
}


void data::getter::ToPipe::StartLoop()
{
    int nReturn;
    ::std::vector< ::std::string > branchNames;
    data::getter::SPipeCallArgs aArgs;
    callbackN::retType::Type clbkReturn (daq::callbackN::retType::Continue);

    m_nWork = 1;

    while(m_nWork ){
        clbkReturn = DoNextStep(&aArgs);
        if(clbkReturn!=daq::callbackN::retType::Continue){return;}
        switch(aArgs.filt.type){
        case data::filter::Type::FileEntriesInfo:
            nReturn=this->GetEntriesInfo(aArgs.rootFileName.c_str());
            break;
        case data::filter::Type::MultyBranchFromFile:
            GetBranchNames(aArgs.branches.c_str(),&branchNames);
            nReturn=this->GetMultipleEntries(aArgs.rootFileName.c_str(),branchNames);
            break;
        case data::filter::Type::ByTime2:
            GetBranchNames(aArgs.branches.c_str(),&branchNames);
            nReturn=this->GetMultipleEntriesTI(branchNames,aArgs.filt.start,aArgs.filt.end);
            break;
        case data::filter::Type::ByEvent2:
            nReturn = -1;
            break;
        default:
            goto returnPoint;
            break;
        }  // switch(aArgs.filt.type){

        if(nReturn==SOCKET_ERROR){goto returnPoint;}

    }  // while(m_nWork ){

returnPoint:
    m_nWork = 0;
    return;
}
