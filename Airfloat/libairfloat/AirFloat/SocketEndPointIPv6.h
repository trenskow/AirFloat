//
//  SocketEndPointIPv6.h
//  AirFloat
//
//  Created by Kristian Trenskow on 3/7/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#ifndef __SOCKETENDPOINTIPV6_H
#define __SOCKETENDPOINTIPV6_H

#include <netinet/in.h>

#include "SocketEndPoint.h"

class SocketEndPointIPv6 : public SocketEndPoint {
    
public:
    SocketEndPointIPv6();
    SocketEndPointIPv6(uint16_t port);
    SocketEndPointIPv6(const char* host, uint16_t port, unsigned int scopeId = 0);
    SocketEndPointIPv6(struct sockaddr* addr);
    
    bool isHost(const char* host);
    bool getHost(char* buffer, long size);
    uint16_t getPort();
    
    bool compareWithAddress(struct sockaddr* endPoint);
    bool compareWithAddress(SocketEndPoint* endPoint);
    
    struct sockaddr* getSocketAddress();
    
    bool isIPv6() { return true; }
    
    SocketEndPoint* copy(uint16_t newPort = 0);
    
private:
    
    struct sockaddr_in6 _ep;
    
};

#endif
