/*
 * Remaining questions
 *  1) pitzbpmadc_rpc_server.cc, Lns: 1025-1042 (imast@?)
 */

#include "pitz_daq_eqfctudpmcast.hpp"
#include "printtostderr.h"
#include <sys/stat.h>
#include <sys/shm.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <libgen.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <TROOT.h>
#include <TString.h>
#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include <TDCacheFile.h>
#include <TBasket.h>
#include <TObject.h>
#include <TSystem.h>
#include <TError.h>
#include <TNetFile.h>
#include "eq_errors.h"
#include "eq_sts_codes.h"
#include "eq_fct_errors.h"
#include "mclistener.hpp"
#include "udpmcastdaq_commonheader.h"
#include "pitz_daq_globalfunctionsiniter.hpp"
#include "common_daq_definations.h"
#include "alog.h"
#include <TPluginManager.h> // https://root.cern.ch/phpBB3/viewtopic.php?t=9816
#ifdef _TEST_GUI_
#include <simple_plotter_c.h>
#endif

#define SPECIAL_KEY "channel"

namespace pitz{namespace daq{

class SingleEntryUdp : public SingleEntry
{
public:
    SingleEntryUdp(entryCreationType::Type creationType,const char* entryLine);
    ~SingleEntryUdp();

    const char* rootFormatString()const;
    pitz::daq::MemoryBase* CreateMemoryInherit();
    void ValueStringByKeyInherited(bool bReadAll, const char* request, char* buffer, int bufferLength)const;
    void PermanentDataIntoFile(FILE* fpFile)const;

    void SetFctParent(EqFctUdpMcast*  a_pFctParent);
    int channelNumber()const{return m_nChannelNumber;}

private:
    EqFctUdpMcast*  m_pFctParent;
    int             m_nChannelNumber;

};
}}


EqFct* eq_create(int a_eq_code, void* /*a_arg*/)
{
    ::EqFct* pRet = NULL;

    switch (a_eq_code)
    {
    case CODE_UDP_MCAST_DAQ:
        pRet = new pitz::daq::EqFctUdpMcast;
        break;
    default: break;
    }
    return pRet;
}


/*////////////////////////////////////////////////////////////////////*/


pitz::daq::EqFctUdpMcast::EqFctUdpMcast() :
       m_hostName("HOST__NAME for multicasting", this)

{
    memset(m_vMapper,0,sizeof(m_vMapper));
    m_hostName.set_ro_access();
}



pitz::daq::EqFctUdpMcast::~EqFctUdpMcast()
{
}


int pitz::daq::EqFctUdpMcast::fct_code()
{
    return CODE_UDP_MCAST_DAQ;
}


pitz::daq::SingleEntry* pitz::daq::EqFctUdpMcast::CreateNewEntry(entryCreationType::Type a_creationType,const char* a_entryLine)
{
    SingleEntryUdp* pEntry = new SingleEntryUdp(a_creationType,a_entryLine);
    int nChannelNumber ;
    
    if(!pEntry){return NULL;}
       
    nChannelNumber = pEntry->channelNumber();

    if(m_vMapper[nChannelNumber]){
        delete pEntry;
        return NULL;
    }
    pEntry->SetFctParent(this);

    return pEntry;
}


void pitz::daq::EqFctUdpMcast::DataGetterThread(SNetworkStruct* /*pNet*/)
{
    SingleEntryUdp* pCurEntry;
    pitz::daq::MemoryBase *pMemory, *pMemoryTmp;
    Memory19* pMemForRcv = new Memory19(NULL,2048,3*sizeof(int));
    MClistener      aListener;
    //MClistener
    int nBranchNum;
    int nbytes;
    int GH;
    int nError=0;
    int nEventNumber2(0);
    bool bErrorNoEntrySet(false);

    if(aListener.ConnectToTheMGroup(m_hostName.value())){
        std::cerr<<"Unable to connect to host\""<<m_hostName.value()<<"\"\n";
        exit(1);
    }
    aListener.SetSocketTimeout(2000);
    m_nWork = 1;

    while( m_nWork ){

        nbytes = aListener.recvC(pMemForRcv->rawBuffer(),sizeof(DATA_struct));

        //DEBUG_("nbytes=%d\n",nbytes);

        if( nbytes != sizeof(DATA_struct) ){

#ifdef __OLD__
            p->listener->init_socket_new();
#else
            if(m_nWork){
                if(nError==0){
                    printtostderr("ERROR","nbytes < 0");
                    ERR_LOG("nbytes<0");
                    fprintf(stderr,"Error during reading\n");
                    nError = 1;
                }
                SleepMs(5000);
            }
            continue;
#endif
        }

        if(nError==1){
            nError = 0;
            printf("Error recovered!\n");
        }
        nBranchNum = pMemForRcv->SwapDataIfNecessary()%MAX_CHANNELS_NUM;

        m_mutexForEntries.readLock();
        pCurEntry = (SingleEntryUdp*)m_vMapper[nBranchNum];
        if(!pCurEntry){m_mutexForEntries.unlock();continue;}

        if(pCurEntry->stack.GetFromStack(&pMemory)){
            nEventNumber2 = pMemForRcv->eventNumber();
            GH = nEventNumber2% s_H_count;
            pMemForRcv->time()=::g_shareptr[GH].seconds;

            pMemory->SetParent(NULL);
            pMemForRcv->SetParent(pCurEntry);

            pMemoryTmp = pMemForRcv;
            pMemForRcv = (Memory19*)pMemory;

            if(!AddJobForRootThread(pMemoryTmp)){
                pCurEntry->SetError(-2);
                if(!bErrorNoEntrySet){fprintf(stderr, "No place in root fifo!\n");bErrorNoEntrySet=true;}
            }
            else{ bErrorNoEntrySet = false; }

        }
        else{
            pCurEntry->SetError(-3);
            if(!bErrorNoEntrySet){fprintf(stderr,"!!!!!!!!!!!!!!! No any entry in the stack!!!!!!!!!!!!!!!!!!\n");bErrorNoEntrySet=true;}
        }

        m_mutexForEntries.unlock();

        m_genEvent.set_value(nEventNumber2);

    } // while( m_nWork )

    aListener.CloseSock();
    //delete pMemForRcv;
    //DEBUG_("!!!!!!!!!!!!!!!!!!!!! m_nWork=%d\n",m_nWork);

}



/*///////////////////////////////////////////////////////////////////////////////*/
pitz::daq::SingleEntryUdp::SingleEntryUdp(entryCreationType::Type a_creationType,const char* a_entryLine)
        :
        SingleEntry(a_creationType,a_entryLine),
        m_pFctParent(NULL)
{

    switch(a_creationType)
    {
    case entryCreationType::fromOldFile:
        {
            int from, samples, step;
            char doocs_url[256], daqName[256];

            sscanf(a_entryLine,"%s %s %d %d %d %d",
                   daqName,doocs_url,&from,&samples,&step,&m_nChannelNumber);
        }
        break;

    case entryCreationType::fromConfigFile: case entryCreationType::fromUser:
        {
            const char *pcNext = strstr(a_entryLine,SPECIAL_KEY "=");

            if(!pcNext){throw errorsFromConstructor::syntax;}
            pcNext += strlen(SPECIAL_KEY "=");
            m_nChannelNumber = atoi(pcNext);

        }
        break;

    default:
        throw errorsFromConstructor::type;
        break;
    }

    m_nChannelNumber<0 ? m_nChannelNumber=0 : m_nChannelNumber=m_nChannelNumber;
    m_nChannelNumber %= MAX_CHANNELS_NUM;

}


pitz::daq::SingleEntryUdp::~SingleEntryUdp()
{
    if(m_pFctParent){m_pFctParent->m_vMapper[m_nChannelNumber] = NULL;}
}


void pitz::daq::SingleEntryUdp::SetFctParent(EqFctUdpMcast*  a_pFctParent)
{
    m_pFctParent = a_pFctParent;
    m_pFctParent->m_vMapper[m_nChannelNumber] = this;
}


void pitz::daq::SingleEntryUdp::PermanentDataIntoFile(FILE* a_fpFile)const
{
    fprintf(a_fpFile,SPECIAL_KEY "=%d; ",m_nChannelNumber);
}


const char* pitz::daq::SingleEntryUdp::rootFormatString()const
{
    return "seconds/I:gen_event/I:array_value[2048]/F";
}


pitz::daq::MemoryBase* pitz::daq::SingleEntryUdp::CreateMemoryInherit()
{
    return new Memory19(this,2048,3*sizeof(int));
}


void pitz::daq::SingleEntryUdp::ValueStringByKeyInherited(bool a_bReadAll, const char* a_request, char* a_buffer, int a_bufferLength)const
{    
    char* pcBufToWrite(a_buffer);
    int nWritten,nBufLen(a_bufferLength);

    if(a_bReadAll ||strstr(SPECIAL_KEY,a_request)){
        nWritten = snprintf(pcBufToWrite,nBufLen,SPECIAL_KEY "=%d; ",m_nChannelNumber);
        pcBufToWrite += nWritten;
        nBufLen -= nWritten;
        if(nBufLen<=0){return;}
    }
}

/*/////////////////////////////////////////////////////////////////////////////////////*/

