//
// file:        pitz/daq/dataclientpipe.hpp
//

#ifndef __pitz_daq_data_getter_topipebase_hpp__
#define __pitz_daq_data_getter_topipebase_hpp__

#include <pitz/daq/data/getter/withindexer.hpp>
#include <pitz/daq/data/engine/local.hpp>
#include <pitz/daq/data/bypipe/base.hpp>


namespace pitz{ namespace daq { namespace data{ namespace getter{

struct SPipeCallArgs{
    ::std::string rootFileName;
    ::std::string branches;
    filter::Data filt;
};

typedef ssize_t (*TypeWritePipe)(int,const void*,size_t);

class ToPipe : public getter::BaseTmp<getter::WithIndexer,engine::Local>, public byPipe::Base
{
public:
    ToPipe( byPipe::pipeType::Type a_pipeType, TypeWritePipe fpWriter );
    virtual ~ToPipe();

    virtual int  GetEntriesInfo( const char* rootFileName) __OVERRIDE__;
    virtual int  GetMultipleEntries( const char* rootFileName, const ::std::vector< ::std::string >& branchNames) __OVERRIDE__;
    virtual int  GetMultipleEntriesTI( const ::std::vector< ::std::string >& branchNames, int startTime, int endTime) __OVERRIDE__;

    int  SetPipes(const char* a_cpcPipesString);
    void ClosePipes();
    void StartLoop();

protected:
    virtual callbackN::retType::Type DoNextStep(SPipeCallArgs* pCallArgs)=0;

protected:
    virtual daq::callbackN::retType::Type NumberOfEntries(int number);
    virtual daq::callbackN::retType::Type ReadEntry(int index, const memory::Base& mem);
    virtual daq::callbackN::retType::Type InfoGetter(int index, const data::EntryInfo& info);
    virtual daq::callbackN::retType::Type AdvInfoGetter(int index, const engine::EntryInfoAdv& info);


protected:
    void*                                               m_pFilter;
    TypeWritePipe                                       m_fpWriter;

};


class ToPipeSingle : public getter::ToPipe
{
public:
    ToPipeSingle(TypeWritePipe a_fpWriter):getter::ToPipe(byPipe::pipeType::Pipe,a_fpWriter){}
    virtual ~ToPipeSingle(){}
    callbackN::retType::Type DoNextStep(SPipeCallArgs*) __OVERRIDE__ {return callbackN::retType::Stop;}
};


}}}}  // namespace pitz{ namespace daq { namespace data{ namespace getter{


#endif  // #ifndef __pitz_daq_data_getter_topipebase_hpp__


