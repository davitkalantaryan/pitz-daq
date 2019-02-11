/* 
 * File:   dataSender.cc
 * Author: bagrat
 * 
 * Created on 20. Juni 2014, 14:46
 */

#include "common/zmqsubscriber.hpp"
#include <zmq.h>

#define ATOMIC(_cond_) (_cond_)


void* common::ZmqSubscriber::mg_zmqContex = NULL;
int common::ZmqSubscriber::mg_nNumberOfInstances = 0;



common::ZmqSubscriber::ZmqSubscriber()
    :
      m_socket(NULL)
{
    if(ATOMIC(0 == mg_nNumberOfInstances++)){
        mg_zmqContex = zmq_ctx_new();
        if(!mg_zmqContex){
            throw "Unable to create zmq contex";
        }
    }
}


common::ZmqSubscriber::~ZmqSubscriber()
{
}


bool common::ZmqSubscriber::ConnectToPublisher(const char* a_cpcPublisher)
{
    int nTimeout(500);

    m_socket = zmq_socket(mg_zmqContex,ZMQ_SUB);
    if(!m_socket){goto returnPoint;}
    if (zmq_setsockopt(m_socket, ZMQ_SUBSCRIBE, "", 0)){goto returnPoint;}
    if (zmq_setsockopt(m_socket, ZMQ_RCVTIMEO, &nTimeout, sizeof(int))){ goto returnPoint;}
    if(zmq_connect(m_socket,a_cpcPublisher)){goto returnPoint;}
    return true;

returnPoint:
    DisconnectFromPublisher();
    return false;
}


void common::ZmqSubscriber::DisconnectFromPublisher()
{
    if(m_socket){
        zmq_close(m_socket);
        m_socket = NULL;
    }
}


int common::ZmqSubscriber::receiveData(void* a_buffer, int a_bufferLength)
{
    return zmq_recv(m_socket, a_buffer,a_bufferLength,0);
}
