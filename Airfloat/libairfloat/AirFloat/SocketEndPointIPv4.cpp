//
//  SocketEndPointIPv4.cpp
//  AirFloat
//
//  Created by Kristian Trenskow on 3/5/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#include <arpa/inet.h>
#include <string.h>
#include <assert.h>

#include "SocketEndPointIPv4.h"

SocketEndPointIPv4::SocketEndPointIPv4() {
    
    memset(&_ep, 0, sizeof(struct sockaddr_in));
    _ep.sin_len = sizeof(struct sockaddr_in);
    _ep.sin_family = AF_INET;
    
}

SocketEndPointIPv4::SocketEndPointIPv4(uint16_t port) {
    
    memset(&_ep, 0, sizeof(struct sockaddr_in));
    _ep.sin_len = sizeof(struct sockaddr_in);
    _ep.sin_family = AF_INET;
    _ep.sin_addr.s_addr = INADDR_ANY;
    _ep.sin_port = htons(port);
    
}

SocketEndPointIPv4::SocketEndPointIPv4(const char* host, uint16_t port, unsigned int scopeId) {
    
    memset(&_ep, 0, sizeof(struct sockaddr_in));
    _ep.sin_len = sizeof(struct sockaddr_in);
    _ep.sin_family = AF_INET;
    _ep.sin_addr.s_addr = INADDR_ANY;
    _ep.sin_port = htons(port);
    
    if (host != NULL)
        inet_pton(AF_INET, host, &_ep.sin_addr);
    
}

SocketEndPointIPv4::SocketEndPointIPv4(struct sockaddr* addr) {
    
    assert(addr != NULL && addr->sa_family == AF_INET);
    
    memcpy(&_ep, addr, MIN(sizeof(sockaddr_in), addr->sa_len));
    
}

bool SocketEndPointIPv4::isHost(const char* host) {
    
    struct sockaddr_in addr;
    if (inet_pton(AF_INET, host, &addr.sin_addr) != 1)
        return false;
    
    return (memcmp(&addr.sin_addr.s_addr, &_ep.sin_addr.s_addr, 4) == 0);
    
}

bool SocketEndPointIPv4::getHost(char* buffer, long size) {
    
    assert(buffer != NULL && size > 0);
    
    inet_ntop(AF_INET, &_ep.sin_addr, buffer, (socklen_t)size);
    
    return true;
    
}

uint16_t SocketEndPointIPv4::getPort() {
    
    return ntohs(_ep.sin_port);
    
}

bool SocketEndPointIPv4::compareWithAddress(struct sockaddr* endPoint) {
    
    if (endPoint->sa_family == AF_INET)
        return (((struct sockaddr_in*)endPoint)->sin_addr.s_addr == _ep.sin_addr.s_addr);
    
    return false;
    
}

bool SocketEndPointIPv4::compareWithAddress(SocketEndPoint* endPoint) {
    
    if (!endPoint->isIPv6())
        return compareWithAddress((struct sockaddr*)&((SocketEndPointIPv4*)endPoint)->_ep);
    
    return false;
    
}

struct sockaddr* SocketEndPointIPv4::getSocketAddress() {
    
    return (struct sockaddr*) &_ep;
    
}

SocketEndPoint* SocketEndPointIPv4::copy(uint16_t newPort) {
    
    SocketEndPointIPv4* ret = new SocketEndPointIPv4((struct sockaddr*) &_ep);
    
    if (newPort > 0)
        ret->_ep.sin_port = htons(newPort);
    
    return ret;
    
}
