/* 
 * File:   zmqsubscriber.hpp
 * Author: Davit Kalantaryan
 *
 * Created on 19. Sep 2017
 */

#ifndef __COMMON_ZMQSUBSCRIBER_HPP__
#define	__COMMON_ZMQSUBSCRIBER_HPP__

#include <stddef.h>

namespace common{

class ZmqSubscriber
{
public:
    ZmqSubscriber();
    virtual ~ZmqSubscriber();

    bool ConnectToPublisher(const char* publisher);
    void DisconnectFromPublisher();
    int receiveData(void* data, int size);
    
protected:
    static void*    mg_zmqContex;
    static int      mg_nNumberOfInstances;

protected:
    void*           m_socket;
    
};


}  // namespace common{

#endif // #ifndef __COMMON_ZMQSUBSCRIBER_HPP__
