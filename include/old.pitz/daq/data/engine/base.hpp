//
// file:        pitz/daq/data/engine/base.hpp
//

#ifndef __pitz_daq_data_engine_base_hpp__
#define __pitz_daq_data_engine_base_hpp__

#include "pitz/daq/data/memory/base.hpp"
#include "pitz/daq/base.hpp"
#include "pitz/daq/callbackn.hpp"
#include <cpp11+/common_defination.h>
#include <vector>
#include <string>
#include <common/lists.hpp>

#define ADV_INF_INT_BYTES   16

namespace pitz{ namespace daq { namespace data { namespace engine{

typedef struct BranchItemPrivate TBranchItemPrivate;
struct EntryInfoAdv : data::EntryInfo{int firstTime,lastTime,firstEvent,lastEvent; ::std::string name; const int* ptr()const{return &firstTime;} int* ptr(){return &firstTime;}};

namespace callbackN{

namespace Type{enum Type{NoClbk,Info,MultiEntries};}
namespace Func{
typedef   daq::callbackN::retType::Type (*NumberOfEntries)(void* clbkData, int number);
typedef   daq::callbackN::retType::Type (*ReadEntry)(void* clbkData, int index, const memory::Base&);
typedef   daq::callbackN::retType::Type (*InfoGetter)(void* clbkData, int index, const EntryInfo& info);
typedef   daq::callbackN::retType::Type (*InfoGetterAdv)(void* clbkData, int index, const EntryInfoAdv& info);
}  // namespace Func{


// this callbacks are used in the case if provided
// 1. rootFileName
// and should be found all daq Entries advanced info
struct SFncsFileEntriesInfo{
    Func::InfoGetterAdv   infoGetter;
    Func::NumberOfEntries numOfEntries;
};

// this callbacks are used in the case if provided
// 1. multipleEntries
// 2. multipleDaqEntryName
// and should be found all daq Entries (criterieToSaveEntry is provided via callbacks)
struct SFncsMultiEntries{
    Func::InfoGetter  infoGetter;
    Func::ReadEntry   readEntry;
};

}  // namespace callback{


class Base : public ::pitz::daq::Base
{
public:
    Base();
    virtual ~Base(){}

    virtual int  Initialize()=0;
    virtual void Cleanup()=0;
    virtual int  GetEntriesInfo( const char* rootFileName)=0;
    virtual int  GetMultipleEntries( const char* rootFileName, ::common::List<TBranchItemPrivate*>* pBranches)=0;

    void SetCallbacks(void* clbkData, const callbackN::SFncsFileEntriesInfo& clbk);
    void SetCallbacks(void* clbkData, const callbackN::SFncsMultiEntries& clbk);
    callbackN::Type::Type callbackType()const{return m_clbkType;}

protected:
    callbackN::Type::Type m_clbkType;
    void*   m_clbkData;
    union{
        callbackN::SFncsFileEntriesInfo  m_flInfo;
        callbackN::SFncsMultiEntries     m_multiEntries;
        struct{void *p1,*p2;}sp;
    }clbk;

};


}}}}  // namespace pitz{ namespace daq { namespace data { namespace engine{



#endif  // #ifndef __pitz_daq_data_engine_base_hpp__


