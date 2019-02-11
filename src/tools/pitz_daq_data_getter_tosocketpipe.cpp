//
// file:        mex_simple_root_reader.cpp
//

#include "pitz/daq/data/getter/tosocketpipe.hpp"
#include "pitz_daq_data_engine_branchitemprivate.hpp"
#include <signal.h>
#include <common/common_socketbase.hpp>

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

using namespace pitz::daq;


static ssize_t WriteToPipeStatic(int a_nSocket, const void* a_cpBuffer, size_t a_nSize2)
{
#define MAX_NUMBER_OF_ITERS	100000
    const char* pcBuffer = (const char*)a_cpBuffer;
    const char *cp = NULL;
    int len_to_write = 0;
    int len_wrote = 0;
    int n = 0;

    len_to_write = (int)a_nSize2;
    cp = pcBuffer;
    for (int i(0);len_to_write > 0;++i)
    {
        n = ::send(a_nSocket, cp, len_to_write, 0);
        if (CHECK_FOR_SOCK_ERROR(n)){
            if (SOCKET_WOULDBLOCK(errno)){
                if(i<MAX_NUMBER_OF_ITERS){SWITCH_SCHEDULING(0);continue;}
                else{return _SOCKET_TIMEOUT_;}
            }else{return E_SEND;}
        }
        else{
            cp += n;
            len_to_write -= n;
            len_wrote += n;
        }
    }

    return len_wrote;
}


data::getter::ToSocketPipe::ToSocketPipe(  )
    :
      getter::ToPipe(byPipe::pipeType::Socket,WriteToPipeStatic)
{
}


data::getter::ToSocketPipe::~ToSocketPipe(  )
{
}


callbackN::retType::Type data::getter::ToSocketPipe::DoNextStep(data::getter::SPipeCallArgs* a_pArgs)
{
    int nStrSize;

    int nIoToReceive=sizeof(a_pArgs->filt);

    if(m_pipes[byPipe::pipePurpose::Cntr]<0){return callbackN::retType::Stop;}

    if( nIoToReceive!=recv(m_pipes[byPipe::pipePurpose::Cntr],(char*)(&a_pArgs->filt), nIoToReceive, MSG_WAITALL) ){return callbackN::retType::Stop;}
    m_filter = a_pArgs->filt;

    switch(a_pArgs->filt.type){
    case filter::Type::NoFilter2:
        break;
    case filter::Type::FileEntriesInfo:

        if( (4!=recv(m_pipes[byPipe::pipePurpose::Cntr],(char*)(&nStrSize),4,MSG_WAITALL))||(nStrSize<1) ){return callbackN::retType::Stop;}
        a_pArgs->rootFileName.resize(nStrSize);
        if( nStrSize!=recv(m_pipes[byPipe::pipePurpose::Cntr],const_cast<char*>(a_pArgs->rootFileName.c_str()),nStrSize,MSG_WAITALL) ){return callbackN::retType::Stop;}

        break;
    case filter::Type::MultyBranchFromFile:

        if( (4!=recv(m_pipes[byPipe::pipePurpose::Cntr],(char*)(&nStrSize),4,MSG_WAITALL))||(nStrSize<1) ){return callbackN::retType::Stop;}
        a_pArgs->rootFileName.resize(nStrSize);
        if( nStrSize!=recv(m_pipes[byPipe::pipePurpose::Cntr],const_cast<char*>(a_pArgs->rootFileName.c_str()),nStrSize,MSG_WAITALL) ){return callbackN::retType::Stop;}

        if( (4!=recv(m_pipes[byPipe::pipePurpose::Cntr],(char*)(&nStrSize),4,MSG_WAITALL))||(nStrSize<1) ){return callbackN::retType::Stop;}
        a_pArgs->branches.resize(nStrSize);
        if( nStrSize!=recv(m_pipes[byPipe::pipePurpose::Cntr],const_cast<char*>(a_pArgs->branches.c_str()),nStrSize,MSG_WAITALL) ){return callbackN::retType::Stop;}

        break;
    case filter::Type::ByTime2:

        if( (4!=recv(m_pipes[byPipe::pipePurpose::Cntr],(char*)(&nStrSize),4,MSG_WAITALL))||(nStrSize<1) ){return callbackN::retType::Stop;}
        a_pArgs->branches.resize(nStrSize);
        if( nStrSize!=recv(m_pipes[byPipe::pipePurpose::Cntr],const_cast<char*>(a_pArgs->branches.c_str()),nStrSize,MSG_WAITALL) ){return callbackN::retType::Stop;}

        break;
    case filter::Type::ByEvent2:
        break;
    default:
        return callbackN::retType::Stop;
    }


    return callbackN::retType::Continue;
}
