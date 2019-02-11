//
// file:        mex_simple_root_reader.cpp
//

#include "pitz/daq/data/getter/withindexer.hpp"
#include "common/listspecialandhashtbl.hpp"
#include "pitz/daq/data/indexer.hpp"
#include "pitz_daq_data_engine_branchitemprivate.hpp"

using namespace pitz::daq;

/***********************************************************************************************************************/

namespace pitz{ namespace daq { namespace data{ namespace getter{ namespace privateN{

static daq::callbackN::retType::Type FileNameReader(void* a_clbkData,const char* a_rootFileName);


struct ForIndexer{
    ::common::List<const char*> list;
    ::common::HashTbl<bool>     hash;
};

}}}}} // namespace pitz{ namespace daq { namespace data{ namespace getter{ namespace privateN{


/***********************************************************************************************************************/


data::getter::WithIndexer::WithIndexer()
{
}


data::getter::WithIndexer::~WithIndexer(  )
{
}


int data::getter::WithIndexer::GetMultipleEntriesTI( const ::std::vector< ::std::string >& a_branchNames, int a_startTime, int a_endTime)
{
    int nReturn=-1;
    try{
        ::common::List<engine::TBranchItemPrivate*> listBranches;
        engine::TBranchItemPrivate* pBranchRaw;
        int nBranchIndex;
        const int cnNumOfBranches((int)a_branchNames.size());
        privateN::ForIndexer aIndexerData;

        for(nBranchIndex=0;nBranchIndex<cnNumOfBranches;++nBranchIndex){
            pBranchRaw = new engine::TBranchItemPrivate(a_branchNames[nBranchIndex],nBranchIndex);
            pBranchRaw->item = listBranches.AddData(pBranchRaw);
        }

        SetFilter(filter::Type::ByTime2,a_startTime,a_endTime);
        SetMultipleEntriesCallback();

        while(listBranches.first()){

            MAKE_REPORT_THIS(3,"listBranches.count()=%d, firstBranchName=\"%s\"",(int)listBranches.count(),listBranches.first()->data->branchName.c_str());
            if(  (indexer::GetListOfFilesForTimeInterval(&aIndexerData,&privateN::FileNameReader,listBranches.first()->data->branchName.c_str(),a_startTime,a_endTime)<0) ||
                 (aIndexerData.list.count()<1)   )
            {
                MAKE_WARNING_THIS("No file found for entry:\"%s\" time interval [%d-%d]",
                                 listBranches.first()->data->branchName.c_str(),a_startTime,a_endTime);
                listBranches.RemoveData(listBranches.first());
                continue;
            }

            MAKE_REPORT_THIS(3,"Number of files for entry:\"%s\" time interval [%d-%d] is %d",
                             listBranches.first()->data->branchName.c_str(),a_startTime,a_endTime,(int)aIndexerData.list.count());

            nReturn = 0;
            while( aIndexerData.list.first() && listBranches.first() ){
                if(nReturn>=0){nReturn = m_pEngine->GetMultipleEntries(aIndexerData.list.first()->data, &listBranches);}
                if(nReturn<0){MAKE_WARNING_THIS("Error during reading from file %s",aIndexerData.list.first()->data);}
                aIndexerData.list.RemoveData(aIndexerData.list.first());
            }
        }  // while(pBranchItem){

        while(listBranches.first()){
            pBranchRaw = listBranches.first()->data;
            listBranches.RemoveData(listBranches.first());
            delete pBranchRaw;
        }
    }
    catch(...){
    }

    SetFilter(data::filter::Type::NoFilter2);
    return nReturn;
}

/***********************************************************************************************************************/

namespace pitz{ namespace daq { namespace data{ namespace getter{ namespace privateN{

static daq::callbackN::retType::Type FileNameReader(void* a_clbkData,const char* a_rootFileName)
{
    bool bExist;
    ForIndexer* pIndexData = (ForIndexer*)a_clbkData;
    const uint32_t cunFileNameLenPlus1((uint32_t)strlen(a_rootFileName)+1);

    if( !pIndexData->hash.FindEntry(a_rootFileName,cunFileNameLenPlus1,&bExist) ){
        const char* cpcKey = (const char*)pIndexData->hash.AddEntry2(a_rootFileName,cunFileNameLenPlus1,true);
        pIndexData->list.AddData(cpcKey);
    }

    return daq::callbackN::retType::Continue;
}


}}}}}  // namespace pitz{ namespace daq { namespace data{ namespace getter{ namespace privateN{

