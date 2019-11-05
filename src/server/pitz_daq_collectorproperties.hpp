// pitz_daq_collectorproperties
#ifndef PITZ_DAQ_COLLECTORPROPERTIES_HPP
#define PITZ_DAQ_COLLECTORPROPERTIES_HPP

#include <eq_fct.h>
#include <pitz/daq/data/memory/base.hpp>
#include "pitz_daq_singleentry.hpp"
#include "common/common_unnamedsemaphorelite.hpp"
#include "cpp11+/shared_mutex_cpp14.hpp"

#define writeLock lock
#define readLock lock_shared


extern int g_nLogLevel;

#define STUPID_NON_CONST
#define s_H_count 1200
#define DEBUG_APP_INFO(_logLevel,...)  \
    do{ \
        if((_logLevel)<=g_nLogLevel){printf("fn:%s,ln:%d -> ",__FUNCTION__,__LINE__);printf(__VA_ARGS__);printf("\n");} \
    }while(0)

#define ERROR_OUT_APP(...)  \
    do{ \
        fprintf(stderr,"fn:%s,ln:%d -> ",__FUNCTION__,__LINE__);fprintf(stderr,__VA_ARGS__);fprintf(stderr,"\n"); \
    }while(0)

#ifdef _WIN32
#define SleepMs    Sleep
#else
#include <unistd.h>
#define SleepMs(_x_) usleep(static_cast<useconds_t>(1000*(_x_)))
#endif

#define DUMMY_ARGS2(...)


namespace pitz{ namespace daq{

class D_addNewEntry : public D_text
{
public:
    D_addNewEntry(const char* pn, EqFct* loc);
    ~D_addNewEntry();

private:
     void    set (EqAdr * dcsAddr, EqData * dataFromUser, EqData * dataToUser, EqFct * location);
};


class D_logLevel : public D_int
{    
public:
    D_logLevel(const char* pn, EqFct* loc);
    void    set (EqAdr * dcsAddr, EqData * dataFromUser, EqData * dataToUser, EqFct * location);
};

class D_removeEntry : public D_string
{
public:
    D_removeEntry(const char* pn, EqFct* loc);
    ~D_removeEntry();

private:
     void    set (EqAdr * dcsAddr, EqData * dataFromUser, EqData * dataToUser, EqFct * location);
};


class D_loadOldConfig : public D_string
{    
public:
    D_loadOldConfig(const char* pn, EqFct* loc);
private:
    void    set (EqAdr * dcsAddr, EqData * dataFromUser, EqData * dataToUser, EqFct * location);
};


}}

typedef struct  H_struct
{
  int   seconds;
  int   microseconds;
  int   gen_event;
  int   rep_rate;
}H_struct;

extern struct H_struct* g_shareptr;
int mkdir_p(const char *a_path, mode_t a_mode);

#endif // PITZ_DAQ_COLLECTORPROPERTIES_HPP
