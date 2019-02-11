/*
 *	File: pitz_daq_globalfunctionsiniter.cpp
 *
 *	Created on: 02 Feb 2017
 *	Created by: Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *  This file implements ...
 *
 */
#include "pitz_daq_globalfunctionsiniter.hpp"
#include <stddef.h>
#include <TROOT.h>
#include <TPluginManager.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "common_daq_definations.h"

#ifdef USE_H_STRUCT
namespace pitz{namespace daq{
static H_struct     s_Hstruct;
H_struct * g_shareptr = &s_Hstruct;
}}
#endif  // #ifdef USE_H_STRUCT


void pitz::daq::GlobalFunctionsIniter::eq_init_prolog()
{
    const key_t  kShMemKey = ftok("/export/doocs/server/daqtimeZMQ_server/adc_daqtimeZMQ",'a');
    int nShmId;
    /*
     * https://root.cern.ch/phpBB3/viewtopic.php?t=9816
     */
    gROOT->GetPluginManager()->AddHandler("TVirtualStreamerInfo",
       "*",
       "TStreamerInfo",
       "RIO",
       "TStreamerInfo()");

    nShmId = shmget(kShMemKey, (sizeof(struct H_struct))*(H_count + 1), 0664);
    if (nShmId < 0)
    {
        int nError =errno;
        printf("nError=%d\n",nError);
        switch(nError)
        {
        case EACCES:printf("EACCES\n");break;
        case EINVAL:printf("EINVAL\n");break;
        case ENFILE:printf("ENFILE\n");break;
        case ENOENT:printf("ENOENT\n");break;
        case ENOMEM:printf("ENOMEM\n");break;
        case ENOSPC:printf("ENOSPC\n");break;
        case EPERM: printf("EPERM\n");break;
        default:printf("UNNOWN\n");break;
        }
        printf(" shared memory is not created... Exit !!!\n");
        exit(1);
    }
    if ( (g_shareptr = (struct H_struct *) shmat(nShmId, NULL, 0)) == (struct H_struct *) -1)
    {
        printf(" can't attach to shared memory... Exit !!!\n");
        exit(1);
    }
}
