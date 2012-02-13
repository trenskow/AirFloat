//
//  SocketEndPoint.h
//  AirFloatCF
//
//  Created by Kristian Trenskow on 5/10/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#ifndef __SOCKETENDPOINT_H
#define __SOCKETENDPOINT_H

#include <netinet/in.h>

class Socket;

class SocketEndPoint {
    
    friend class Socket;
    
public:
    SocketEndPoint();
    SocketEndPoint(const char* host, unsigned short port, unsigned int scopeId = 0);
    SocketEndPoint(struct sockaddr* addr);
    
    SocketEndPoint& operator=(SocketEndPoint &ep);
    
    void setup(const char* host, unsigned short port, unsigned int scopeId = 0);
    void setupIPv6(unsigned short port, unsigned int scopeId = 0);
    bool isHost(const char* host);
    bool getHost(char* buffer, long size);
    unsigned short port();    
    unsigned int getScopeId();
    
    struct sockaddr* getSocketAddress();
    
    static in6_addr IPv4AddressToIPv6Address(in_addr addr);
    
private:
        
    struct sockaddr_in6 _ep;
    
};

#endif