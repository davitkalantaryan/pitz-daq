//
// file:        mex_simple_root_reader.cpp
//

#include "pitz/daq/data/getter/noindexing.hpp"
#include "pitz_daq_data_engine_branchitemprivate.hpp"
#include "common/listspecialandhashtbl.hpp"


using namespace pitz::daq;


data::getter::NoIndexing::NoIndexing(  )
{
}


data::getter::NoIndexing::~NoIndexing(  )
{
}


int data::getter::NoIndexing::GetMultipleEntriesTI( const ::std::vector< ::std::string >& a_branchNames, int a_startTime, int a_endTime)
{
    int nReturn=-1;
    try{
        engine::TBranchItemPrivate* pBranchRaw;
        ::common::List<engine::TBranchItemPrivate*> listBranches;
        int nBranchIndex;
        const int cnNumOfBranches((int)a_branchNames.size());

        SetFilter(filter::Type::ByTime2,a_startTime, a_endTime);
        SetMultipleEntriesCallback();

        for(nBranchIndex=0;nBranchIndex<cnNumOfBranches;++nBranchIndex){
            pBranchRaw = new engine::TBranchItemPrivate(a_branchNames[nBranchIndex],nBranchIndex);
            pBranchRaw->item = listBranches.AddData(pBranchRaw);
        }

        nReturn = m_pEngine->GetMultipleEntries("", &listBranches);

        while(listBranches.first()){
            pBranchRaw = listBranches.first()->data;
            listBranches.RemoveData(listBranches.first());
            delete pBranchRaw;
        }
    }
    catch(...){
    }

    return nReturn;
}
