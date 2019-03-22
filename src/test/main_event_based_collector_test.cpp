/*
 *  Copyright (C) DESY
 *
 *  file:            main_event_based_collector_test.cpp
 *  created on:      2019 Mar 22
 *  created by:      D. Kalantaryan <davit.kalantaryan@desy.de>
 *
 */

/**
 *
 * @file       test/main_event_based_collector_test.cpp
 * @copyright  DESY
 * @brief      sourse for testing event based
 * @author     D. Kalantaryan <davit.kalantaryan@desy.de>
 * @date       2019 Mar 22
 * @details
 *             File demonstrates how event based DAQ collector works
 *
 */


#include <zmq/zmq.h>
#include <thread>
#include <chrono>

#define CENTRAL_TIMING_DETAILS  "tcp://mtcapitzcpu4:5566"

static void DoocsZmqThreadFunction(void);
static void TineThreadFunction(void);
static void AnyThreadFunction(void);

static volatile int s_nWork = 1;

int main()
{
    ::std::thread threadDoocsZmq(DoocsZmqThreadFunction);
    ::std::thread threadTine(TineThreadFunction);
    ::std::thread threadAny(AnyThreadFunction);

    while(s_nWork){
        ::std::this_thread::sleep_for( ::std::chrono::hours(1) );
    }

    threadAny.join();
    threadTine.join();
    threadDoocsZmq.join();

    return 0;
}




static void DoocsZmqThreadFunction(void)
{
    //
}


// /doocs/develop/kalantar/programs/matlab/ccppfiles/mexprojects/tine_subscriber
static void TineThreadFunction(void)
{
    //
}


/**
 * @brief
 *  function that demonstrates how any socket (ZMQ included) implementation will work.
 * @details
 *  For simplicity error codes on options setting ignored
 * @cite
 *  http://api.zeromq.org/2-1:zmq-poll
 *   - All ØMQ sockets passed to the zmq_poll() function must share the same ØMQ context and must belong to the thread calling zmq_poll().
 */
#if 0
void *socket;
#if defined _WIN32
SOCKET fd;
#else
int fd;
#endif
short events;
short revents;
#endif
static void AnyThreadFunction(void)
{
    void* pContext = nullptr;
    void* pZmqSocket = nullptr;
    zmq_pollitem_t vPollItems[1024];
    //int nitems = 0;

    pContext = zmq_ctx_new();
    if(!pContext){goto returnPoint;}

    pZmqSocket = zmq_socket (pContext, ZMQ_SUB);
    if(!pZmqSocket){goto returnPoint;}

    vPollItems[0].socket = pZmqSocket;
    //vPollItems[0].fd = -1; // commented because socket is tried when non null


    zmq_setsockopt (pZmqSocket,ZMQ_SUBSCRIBE, nullptr,0);
    zmq_connect (pZmqSocket, CENTRAL_TIMING_DETAILS);


returnPoint:
    if(pContext){zmq_ctx_destroy(pContext);}
}
