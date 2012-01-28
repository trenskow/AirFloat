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
    
    void Setup(const char* host, unsigned short port, unsigned int scopeId = 0);
    void SetupIPv6(unsigned short port, unsigned int scopeId = 0);
    bool IsHost(const char* host);
    bool GetHost(char* buffer, long size);
    unsigned short Port();    
    unsigned int ScopeId();
    
    bool isHostsEqual(SocketEndPoint* ep2);
    
    struct sockaddr* SocketAdress();
    
    static in6_addr IPv4AddressToIPv6Address(in_addr addr);
    
private:
        
    struct sockaddr_in6 _ep;
    
};

#endif