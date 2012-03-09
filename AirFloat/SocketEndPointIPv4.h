//
//  SocketEndPointIPv4.h
//  AirFloat
//
//  Created by Kristian Trenskow on 3/5/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#ifndef __SOCKETENDPOINTIPV4_H
#define __SOCKETENDPOINTIPV4_H

#include <netinet/in.h>

#include "SocketEndPoint.h"

class SocketEndPointIPv4 : public SocketEndPoint {
    
public:
    
    SocketEndPointIPv4();
    SocketEndPointIPv4(uint16_t port);
    SocketEndPointIPv4(const char* host, uint16_t port, unsigned int scopeId = 0);
    SocketEndPointIPv4(struct sockaddr* addr);
    
    bool isHost(const char* host);
    bool getHost(char* buffer, long size);
    uint16_t getPort();
    
    bool compareWithAddress(struct sockaddr* endPoint);
    bool compareWithAddress(SocketEndPoint* endPoint);
    
    struct sockaddr* getSocketAddress();
    
    bool isIPv6() { return false; }
    
    SocketEndPoint* copy(uint16_t newPort = 0);

private:
    
    struct sockaddr_in _ep;
    
};

#endif
