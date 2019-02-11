
// 2017 Sep 18
//

#include "pitz_daq_collectorproperties.hpp"
#include "pitz_daq_eqfctcollector.hpp"
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include "stringparser1.h"
#include <signal.h>
#include <TFile.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>


#define EMAIL_BUF_LEN   2047
#define SUBJ_BUF_LEN    511
#ifndef MAX_HOSTNAME_LENGTH
#define MAX_HOSTNAME_LENGTH 64
#endif

#define SOLVED

int g_nLogLevel=0;

/*///////////////////////////////////////////////////////////////////////////////////*/

pitz::daq::D_logLevel::D_logLevel(const char* a_pn, EqFct* a_loc)
        :
        D_int(a_pn,a_loc)
{
}


void pitz::daq::D_logLevel::set (EqAdr * a_dcsAddr, EqData * a_dataFromUser, EqData * a_dataToUser, EqFct * a_location)
{
    g_nLogLevel=a_dataFromUser->get_int();
    D_int::set(a_dcsAddr, a_dataFromUser, a_dataToUser, a_location);
}


/***********************************************************************************/

pitz::daq::D_addNewEntry::D_addNewEntry(const char* a_pn, EqFct* a_loc)
        :
        D_text(a_pn,a_loc)
{
}


pitz::daq::D_addNewEntry::~D_addNewEntry()
{
}

void pitz::daq::D_addNewEntry::set (EqAdr * /*a_dcsAddr*/,
                                     EqData * a_dataFromUser, EqData * a_dataToUser, EqFct * a_pFct)
{
    const char* cpcRet ;
    EqFctCollector* pFct = (EqFctCollector*)a_pFct;
    bool bRet;

#if 0
    {
        char vcBuffer[1024];
        a_dataFromUser->get_string(vcBuffer,1023);
        printf("!!!!!!!!! userString(strLen=%d)=\"%s\"\n",(int)strlen(vcBuffer),vcBuffer);
    }
#endif

    std::string strFromUser = a_dataFromUser->get_string();
    //std::cout<<strFromUser<<std::endl;
    bRet = pFct->AddNewEntry(entryCreationType::fromUser, strFromUser.c_str());
    //a_dataToUser->set_val(
    cpcRet = bRet ? "   ok" : "   problem";
    std::cout<<strFromUser<<cpcRet<<std::endl;
    set_value(strFromUser);
    //D_text::set(a_dcsAddr,a_dataFromUser,a_dataToUser,a_pFct);
    a_dataToUser->set(cpcRet);
    if(!bRet){a_dataToUser->error(1001, "Unable to add");}
    else {a_dataToUser->error(0, "Added");}

}


/*///////////////////////////////////////////////////////////////////////////////////*/

pitz::daq::D_removeEntry::D_removeEntry(const char* a_pn, EqFct* a_loc)
        :
        D_string(a_pn,a_loc)
{
}


pitz::daq::D_removeEntry::~D_removeEntry()
{
}


void pitz::daq::D_removeEntry::set (EqAdr * a_dcsAddr,
                                     EqData * a_dataFromUser, EqData * a_dataToUser, EqFct * a_pFct)
{
    const char* cpcRet ;
    EqFctCollector* pFct = (EqFctCollector*)a_pFct;
    SingleEntry* pEntry ;

    std::string strFromUser = a_dataFromUser->get_string();
    //std::cout<<strFromUser<<std::endl;
    pFct->m_mutexForEntries.lock();
    pEntry = pFct->FindEntry(strFromUser.c_str());
    if(pEntry){pEntry->RemoveDoocsProperty();pFct->RemoveOneEntry(pEntry);}
    pFct->m_mutexForEntries.unlock();
    cpcRet = pEntry ? "   ok" : "   problem";
    std::cout<<"remove: "<<strFromUser<<cpcRet<<std::endl;
    D_string::set(a_dcsAddr,a_dataFromUser,a_dataToUser,a_pFct);

}



/*////////////////////////////////////////////////////////////////////////////////////////////////*/

pitz::daq::D_loadOldConfig::D_loadOldConfig(const char* a_pn, EqFct* a_loc)
        :
        D_string(a_pn,a_loc)
{
}

void pitz::daq::D_loadOldConfig::set (EqAdr * a_adr, EqData * a_dataFromUser, EqData * a_dataToUser, EqFct * a_loc)
{
    EqFctCollector* pCollector = (EqFctCollector*)a_loc;
    std::string aOldConfigPath = a_dataFromUser->get_string();
    std::string aOldConfig = aOldConfigPath;

    if(!aOldConfigPath.length()){
        aOldConfig = value();
        pCollector->parse_old_config2(value());
    }

    if(!pCollector->parse_old_config2(aOldConfig)){
        D_string::set(a_adr, a_dataFromUser, a_dataToUser, a_loc);
    }
    else{
        //a_dataToUser->set
        //set_err
        a_dataToUser->error(5,"Unable to open the file");
        //a_dataToUser->set("Unable to open the file");
    }

}

