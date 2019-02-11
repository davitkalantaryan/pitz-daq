/*
 *	File      : pitz_daq_daqinterface_application.hpp
 *
 *	Created on: 22 Mar, 2017
 *	Author    : Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *
 */
#ifndef PITZ_DAQ_DAQINTERFACE_APPLICATION_HPP
#define PITZ_DAQ_DAQINTERFACE_APPLICATION_HPP

#include <QApplication>
#include <eq_client.h>
#include <string>
#include <thread>
#include <string.h>
#include <cpp11+/common_defination.h>
#include <cpp11+/thread_cpp11.hpp>
#include <common/common_unnamedsemaphorelite.hpp>
#include <common/lists.hpp>
#include <common/listspecialandhashtbl.hpp>
#include <string>
#include <cpp11+/shared_mutex_cpp14.hpp>
#include <stdint.h>
#include <common/mapandhash.hpp>

extern int g_nDebugApplication;

#define __ONLY__FILE__(__full_path__) \
    ( strrchr((__full_path__),'/') ? \
        (strrchr((__full_path__),'/') + 1): \
        ( strrchr((__full_path__),'\\') ?  (strrchr((__full_path__),'\\')+1) : (__full_path__) )  )

#define __DEBUG__APP__(__log_level__,...) \
    do{ \
        if((__log_level__)<=g_nDebugApplication){\
            printf("fl:\"%s\", ln:%d, fnc:%s, ",__ONLY__FILE__(__FILE__),__LINE__,__FUNCTION__); \
            printf(__VA_ARGS__); printf("\n");}}while(0)



namespace pitz{ namespace daq{ namespace daqinterface{

enum class TaskType{
    noCommand,
    setToDoocsAddressInt
};


struct SCallArgsAll{
    TaskType type; void* clb_data;std::string input;
    SCallArgsAll(TaskType a_type=TaskType::noCommand, std::string a_input="",void* a_clb_data=nullptr):type(a_type),clb_data(a_clb_data),input(a_input){}
};

}}}

typedef void (__THISCALL__ *TypeCallback1)(void* owner,pitz::daq::daqinterface::SCallArgsAll args, int64_t err);

namespace pitz{ namespace daq{ namespace daqinterface{

struct STaskStruct{TypeCallback1 fpClbk;void* owner;SCallArgsAll args;};
struct STaskDoneStruct{STaskStruct task; int64_t err;};

enum class collectorChangeCode{
    no,
    all,
    watchdogGoodBad,
    startStop,
    error,
    errorStr,
    onlineOffline,
    genEvent
};


class SCollemtorItem : public QObject
{
    Q_OBJECT

    friend class Application;
public:
    SCollemtorItem(int a_iteration,const std::string& a_serverNm);
    ~SCollemtorItem();

    bool Handle();  // true means changed

public:
    //::std::string serverName;
    QObject* slotObject;
    void* ownerData;
    int iteration;
public:
    int error;
    int genEvent;
    ::std::string  errorStr;
    uint32_t isRunning : 1;
    int isOnline : 10;
    uint32_t isWatchdogOk : 1;
public:
    ::std::string watchdogAddressBase;
    ::std::string  hostName2;
    ::std::string  locationBase;
    int rpcNumber;
public:
    SCollemtorItem *prev,*next;
    size_t keyLength;
    ::common::MapAndHash<SCollemtorItem*, ::std::string>::SMapAndHashItem* pHashItem;

public:
signals:
    void ChangedSignal(int);
};


class Application : public QApplication
{
    Q_OBJECT

public:
    Application(int& argc, char* argv[]);
    virtual ~Application();

    void SetNewTask(TypeCallback1 fpClb,void* owner,SCallArgsAll args);

    template <typename ClsType>
    void SetNewTask( void(ClsType::*fpClb)(SCallArgsAll,int64_t err),ClsType* owner,SCallArgsAll a_args);

    template <typename ClsType>
    void IterateOverEntries( ClsType* a_pOwner,void(ClsType::*fpClb)(SCollemtorItem*& ) );

protected:
    virtual void RegularFunction(int a_nIteration);
    void RemoveItemIfNeeded(SCollemtorItem*& a_pItem, int a_nIteration);

private:
    void TasksThreadFunction();
    void DeviceRegularThreadFunction();

private slots:
    void TaskDoneSlot1(pitz::daq::daqinterface::STaskDoneStruct tsk_done);

private:
signals:
    void TaskDoneSig1(pitz::daq::daqinterface::STaskDoneStruct tsk_done);
public:
signals:
    void InitializationDoneSignal();
    void NewCollectorFoundSignal(SCollemtorItem*,ptrdiff_t);
    void RemoveSingleCollectorSignal(SCollemtorItem*);

public:
    ::STDN::thread                      m_tasksThread;
    ::STDN::thread                      m_deviceRegularThread;
    ::common::UnnamedSemaphoreLite      m_semaphore;
    volatile int                        m_nWork;
    ::common::listN::Fifo<STaskStruct>  m_taskFifo;
    ::STDN::shared_mutex                m_mutexForEntries;

protected:
    ::common::MapAndHash<SCollemtorItem*, ::std::string >    m_listAndHash;
};


#define CurrentApp()    ((pitz::daq::daqinterface::Application*)QCoreApplication::instance())
#define CallInCurApp(_func,...)  ( ((pitz::daq::daqinterface::Application*)QCoreApplication::instance())->_func(__VA_ARGS__)  )
//#define SetNewTaskGlbMacro(_fpClb,_owner,_args)  (((pitz::daq::daqinterface::Application*)QCoreApplication::instance())->SetNewTask((_fpClb),(_owner),(_args)))
#define SetNewTaskGlbMacro(_fpClb,_owner,_args)  CallInCurApp((_fpClb),(_owner),(_args))

}}}

#define CurrentApp()    ((pitz::daq::daqinterface::Application*)QCoreApplication::instance())

#include "impl.pitz_daq_daqinterface_application.hpp"

#endif // PITZ_DAQ_DAQINTERFACE_APPLICATION_HPP
