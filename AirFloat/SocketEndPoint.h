//
//  SocketEndPoint.h
//  AirFloat
//
//  Created by Kristian Trenskow on 3/5/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#ifndef __SOCKETENDPOINT_H
#define __SOCKETENDPOINT_H

#include <sys/socket.h>
#include <stdlib.h>
#include <stdint.h>

#if !defined (MIN)
#define MIN(x,y) (x < y ? x : y)
#endif

typedef enum {
    
    kSocketEndPointTypeIPv4 = 1,
    kSocketEndPointTypeIPv6 = 1 << 1
    
} SocketEndPointType;

class SocketEndPoint {
    
public:
    
    virtual ~SocketEndPoint() {}
    
    virtual bool isHost(const char* host) = 0;
    virtual bool getHost(char* buffer, long size) = 0;
    virtual uint16_t getPort() = 0;
    
    virtual bool compareWithAddress(struct sockaddr* endPoint) = 0;
    virtual bool compareWithAddress(SocketEndPoint* endPoint) = 0;
    
    virtual struct sockaddr* getSocketAddress() = 0;
    
    virtual bool isIPv6() = 0;
    
    virtual SocketEndPoint* copy(uint16_t newPort = 0) = 0;
    
    static SocketEndPoint* createSocket(uint16_t port, SocketEndPointType socketType = kSocketEndPointTypeIPv4);
    static SocketEndPoint* createSocket(struct sockaddr* addr);
    
};

#endif
