//
// file:        mex_simple_root_reader.cpp
//

#include "pitz/daq/data/getter/base.hpp"
#include "pitz_daq_data_engine_branchitemprivate.hpp"
#include <string.h>


using namespace pitz::daq;

/***********************************************************************************************************************/

namespace pitz{ namespace daq { namespace data{ namespace getter{

void GetBranchNames(const char* a_branchNamesRaw, ::std::vector< ::std::string >* a_pBranchNames)
{
    const char* cpcBranchName = a_branchNamesRaw;
    const char* cpcNextBranchName = strchr(cpcBranchName,';');

    while(cpcNextBranchName){
        a_pBranchNames->push_back(std::string(cpcBranchName,cpcNextBranchName-cpcBranchName));
        cpcBranchName = cpcNextBranchName+1;
        cpcNextBranchName = strchr(cpcBranchName,';');
    }
    a_pBranchNames->push_back(cpcBranchName);
}

namespace privateN{

daq::callbackN::retType::Type NumberOfEntriesStat(void* clbkData, int number);
daq::callbackN::retType::Type ReadEntryStat(void* clbkData, int index, const memory::Base&);
daq::callbackN::retType::Type InfoGetterStat(void* clbkData, int index, const data::EntryInfo& info);
daq::callbackN::retType::Type InfoGetterAdvStat(void* clbkData, int index, const engine::EntryInfoAdv& info);

class Util : private getter::Base
{
    friend daq::callbackN::retType::Type NumberOfEntriesStat(void* clbkData, int number);
    friend daq::callbackN::retType::Type ReadEntryStat(void* clbkData, int index, const memory::Base&);
    friend daq::callbackN::retType::Type InfoGetterStat(void* clbkData, int index, const data::EntryInfo& info);
    friend daq::callbackN::retType::Type InfoGetterAdvStat(void* clbkData, int index, const engine::EntryInfoAdv& info);
};


}}}}} // namespace pitz{ namespace daq { namespace data{ namespace getter{ namespace privateN{


/***********************************************************************************************************************/


data::getter::Base::Base( )
    :
      m_pEngine(NULL)
{
}


data::getter::Base::~Base(  )
{
}


data::engine::Base* data::getter::Base::operator->()
{
    return m_pEngine;
}


int data::getter::Base::GetEntriesInfo( const char* a_rootFileName)
{
    int nReturn;
    engine::callbackN::SFncsFileEntriesInfo aFncs = {&privateN::InfoGetterAdvStat,&privateN::NumberOfEntriesStat};
    m_pEngine->SetCallbacks(this,aFncs);
    nReturn = m_pEngine->GetEntriesInfo(a_rootFileName);
    return nReturn;
}


int data::getter::Base::GetMultipleEntries( const char* a_rootFileName, const ::std::vector< ::std::string >& a_branchNames)
{
    int nReturn;
    engine::callbackN::SFncsMultiEntries aFncs = {&privateN::InfoGetterStat,&privateN::ReadEntryStat};
    ::common::listN::ListItem<engine::TBranchItemPrivate*> *pBranchItemNext, *pBranchItem ;
    ::common::List<engine::TBranchItemPrivate*> listBranches;
    engine::TBranchItemPrivate* pBranchRaw;
    int nBranchIndex;
    const int cnNumOfBranches((int)a_branchNames.size());

    for(nBranchIndex=0;nBranchIndex<cnNumOfBranches;++nBranchIndex){
        pBranchRaw = new engine::TBranchItemPrivate(a_branchNames[nBranchIndex],nBranchIndex);
        pBranchRaw->item = listBranches.AddData(pBranchRaw);
    }

    this->SetFilter(data::filter::Type::MultyBranchFromFile);
    m_pEngine->SetCallbacks(this,aFncs);
    nReturn = m_pEngine->GetMultipleEntries(a_rootFileName,&listBranches);
    pBranchItem = listBranches.first();
    while(pBranchItem){
        pBranchItemNext = pBranchItem->next;
        pBranchRaw = pBranchItem->data;
        listBranches.RemoveData(pBranchItem);
        delete pBranchRaw;
        pBranchItem = pBranchItemNext;
    }
    SetFilter(data::filter::Type::NoFilter2);
    return nReturn;
}


void data::getter::Base::SetMultipleEntriesCallback()
{
    engine::callbackN::SFncsMultiEntries aFncs = {&privateN::InfoGetterStat,&privateN::ReadEntryStat};
    m_pEngine->SetCallbacks(this,aFncs);
}


void data::getter::Base::SetFilter(data::filter::Type::Type a_type, ...)
{
    va_list args;

    switch(a_type)
    {
    case data::filter::Type::NoFilter2:case data::filter::Type::FileEntriesInfo:case data::filter::Type::MultyBranchFromFile:
        m_filter.start = -1;
        m_filter.end = -1;
        break;
    case data::filter::Type::ByTime2:case data::filter::Type::ByEvent2:
        va_start(args,a_type);
        m_filter.start = va_arg(args,int);
        m_filter.end = va_arg(args,int);
        va_end(args);
        break;
    default:
        break;
    }

    m_filter.type = a_type;
}

/***********************************************************************************************************************/

namespace pitz{ namespace daq { namespace data{ namespace getter{ namespace privateN{


daq::callbackN::retType::Type NumberOfEntriesStat(void* a_clbkData, int a_number)
{
    Util* pP = (Util*)a_clbkData;
    return pP->NumberOfEntries(a_number);
    //if(clbkReturn!=daq::callbackN::retType::Continue){return clbkReturn;}
}


daq::callbackN::retType::Type ReadEntryStat(void* a_clbkData, int a_index, const memory::Base& a_mem)
{
    Util* pP = (Util*)a_clbkData;
    const filter::Data& aFilter = pP->filter();

    switch(aFilter.type){
    case filter::Type::MultyBranchFromFile:
        return pP->ReadEntry(a_index,a_mem);
    case filter::Type::ByTime2:
        if((a_mem.time()>=aFilter.start)&&(a_mem.time()<=aFilter.end)){
            return pP->ReadEntry(a_index,a_mem);
        }
        else if(a_mem.time()<aFilter.start){return daq::callbackN::retType::Continue;}
        else {return daq::callbackN::retType::StopForCurrent;}
        break;
    default:
        break;
    }

    return pP->ReadEntry(a_index,a_mem);
}


daq::callbackN::retType::Type InfoGetterStat(void* a_clbkData, int a_index, const data::EntryInfo& a_info)
{
    Util* pP = (Util*)a_clbkData;
    return pP->InfoGetter(a_index,a_info);
}


daq::callbackN::retType::Type InfoGetterAdvStat(void* a_clbkData, int a_index, const engine::EntryInfoAdv& a_info)
{
    Util* pP = (Util*)a_clbkData;
    return pP->AdvInfoGetter(a_index,a_info);
}


}}}}}  // namespace pitz{ namespace daq { namespace data{ namespace getter{ namespace privateN{

