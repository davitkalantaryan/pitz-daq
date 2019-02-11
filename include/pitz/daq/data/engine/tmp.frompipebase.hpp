//
// file:        pitz/daq/data/engine/pipebase.hpp
//

#ifndef __pitz_daq_data_engine_pipebase_hpp__
#define __pitz_daq_data_engine_pipebase_hpp__

#include <pitz/daq/data/engine/base.hpp>
#ifdef _WIN32
#include <stddef.h>
#ifndef ssize_t_defined
typedef size_t ssize_t;
#endif
#else
#include <unistd.h>
#endif


namespace pitz{ namespace daq { namespace data { namespace engine{

typedef ssize_t (*TypeReadPipe)(int,void*,size_t);

class FromPipeBase : public engine::Base
{
public:
    FromPipeBase( TypeReadPipe a_fpReader );
    virtual ~FromPipeBase();

    int  GetEntriesInfo( const char* rootFileName) __OVERRIDE__ __FINAL__;
    int  GetMultipleEntries( const char* rootFileName, ::common::List<TBranchItemPrivate*>* pBranches) __OVERRIDE__ __FINAL__;

    virtual void SetFilter(filter::Data* a_pFilter) __OVERRIDE__ {m_pFilter = a_pFilter;}

protected:
    virtual int  StartPipes(const ::std::string & branchNamesAll, const char* rootFileName)=0;
    virtual void StopPipes()=0;


protected:
    TypeReadPipe                                        m_fpReader;
    int                                                 m_nPipesDataRd;
    int                                                 m_nPipesCtrlRd;
    int                                                 m_nPipesInfoRd;
    volatile int                                        m_nWork2;
    filter::Data*                                       m_pFilter;
    uint32_t                                            m_isRunning;

};


}}}}  // namespace pitz{ namespace daq { namespace data { namespace engine{


#endif  // #ifndef __pitz_daq_data_engine_pipebase_hpp__


