
// 2017 Nov 24
// pitz_daq_collector_global.cpp
// int mkdir_p(const char *a_path, mode_t a_mode) defined here

#include <TPluginManager.h>
#include <TROOT.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "pitz_daq_collectorproperties.hpp"
#include "pitz_daq_eqfctcollector.hpp"

#define mkdir_p_debug(...)

static key_t            s_SHMKEY = (key_t)0;
static int              s_shmid = 0;
static int mkdir_p_raw(const char *a_path, mode_t a_mode);

struct H_struct* g_shareptr = NULL;

const char* object_name = "daqcollector";

void eq_init_prolog() 	// called once before init of all EqFct's
{

    printf("version 4\n");
    //printf("Press ay key, then press Enter to continue\n");fflush(stdout);
    //getchar();
    if(s_SHMKEY){return;}

#ifdef DEBUG_APP
    getchar();
#endif

    /*
     * https://root.cern.ch/phpBB3/viewtopic.php?t=9816
     */
    gROOT->GetPluginManager()->AddHandler("TVirtualStreamerInfo",
       "*",
       "TStreamerInfo",
       "RIO",
       "TStreamerInfo()");

    s_SHMKEY = ftok("/export/doocs/server/daqtimeZMQ_server/adc_daqtimeZMQ",'a');
    s_shmid = shmget(s_SHMKEY, (sizeof(H_struct))*(s_H_count + 1), 0664);
    if (s_shmid < 0)
    {
        fprintf(stderr," shared memory is not created... Exit !!!\n");
        goto exitPoint;
    }
    if ( (g_shareptr = (struct H_struct *) shmat(s_shmid, NULL, 0)) == (struct H_struct *) -1)
    {
        fprintf(stderr," can't attach to shared memory... Exit !!!\n");
        goto exitPoint;
    }

    printf("!!!!!! s_shareptr=%p\n",g_shareptr);
    return;

exitPoint:

    fprintf(stderr, "Problem to connect shared memory!\n");

#ifdef TEST_VERSION112
    static int s_COUNT = 250;
    g_shareptr = new H_struct[s_COUNT];
#else
    printf("!!!!!! exiting!\n");
    exit (2);
#endif

}

void eq_cancel()
{
    // shared memory should be released
}

void refresh_prolog() {}
void refresh_epilog() {}
void post_init_epilog(void) {}
void interrupt_usr1_epilog(int) {}
void interrupt_usr1_prolog(int) {}
void eq_init_epilog() {}
void post_init_prolog() {}

int mkdir_p(const char *a_path, mode_t a_mode)
{
    int nError = mkdir_p_raw(a_path,a_mode);

    switch(nError)
    {
    case 0:
        break;
    case ENOENT:
        mkdir_p_debug("(ENOENT)");
    {
        std::string aNewPath(a_path);
        char* basePath = const_cast<char*>(aNewPath.c_str());
        char *lastDir = strrchr(basePath,'/');
        int i,nDeep=0;

        while(lastDir && (nError==ENOENT)){
            ++nDeep;
            *lastDir = 0;
            mkdir_p_debug("new_try=%s",aNewPath.c_str());
            nError=mkdir_p_raw(basePath,a_mode);
            lastDir = strrchr(basePath,'/');
        }

        for(i=0;(i<nDeep)&&(nError==0);++i){
            basePath[strlen(basePath)]='/';
            nError=mkdir_p_raw(basePath,a_mode);
        }

    }
        break;
    case ELOOP:
        mkdir_p_debug("(ELOOP)");break;
    case EMLINK:
        mkdir_p_debug("(EMLINK)");break;
    default:
        mkdir_p_debug("(unknown)");break;
    }

    return nError;
}


static int mkdir_p_raw(const char *a_path, mode_t a_mode)
{
    int nReturn = mkdir(a_path,a_mode);

    if(nReturn){
        nReturn = errno;
        mkdir_p_debug("errno=%d ",nReturn);
        switch(nReturn)
        {
        case EEXIST:
            mkdir_p_debug("(EEXIST)");
            nReturn = 0;
            break;
        default:
            mkdir_p_debug("(unknown)");
            break;
        }

        mkdir_p_debug("\n");

    } // if(nReturn){

    return nReturn;
}
