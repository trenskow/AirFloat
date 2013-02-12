//
//  SocketEndPointIPv6.cpp
//  AirFloat
//
//  Created by Kristian Trenskow on 3/7/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#include <arpa/inet.h>
#include <string.h>
#include <assert.h>

#include "SocketEndPointIPv6.h"

SocketEndPointIPv6::SocketEndPointIPv6() {
    
    memset(&_ep, 0, sizeof(struct sockaddr_in6));
    _ep.sin6_len = sizeof(struct sockaddr_in6);
    
}

SocketEndPointIPv6::SocketEndPointIPv6(uint16_t port) {
    
    memset(&_ep, 0, sizeof(struct sockaddr_in6));
    _ep.sin6_len = sizeof(struct sockaddr_in6);
    _ep.sin6_family = AF_INET6;
    _ep.sin6_addr = in6addr_any;
    _ep.sin6_port = htons(port);
    
}

SocketEndPointIPv6::SocketEndPointIPv6(const char* host, uint16_t port, unsigned int scopeId) {
    
    memset(&_ep, 0, sizeof(_ep));
    _ep.sin6_len = sizeof(struct sockaddr_in6);
    _ep.sin6_family = AF_INET6;
    _ep.sin6_addr = in6addr_any;
    _ep.sin6_port = htons(port);
    _ep.sin6_scope_id = scopeId;
    
    if (host != NULL)
        inet_pton(AF_INET6, host, &_ep.sin6_addr);
    
}

SocketEndPointIPv6::SocketEndPointIPv6(struct sockaddr* addr) {
    
    assert(addr != NULL && addr->sa_family == AF_INET6);
    
    memcpy(&_ep, addr, MIN(sizeof(sockaddr_in6), addr->sa_len));
    
}

bool SocketEndPointIPv6::isHost(const char* host) {
    
    struct sockaddr_in6 addr6;
    if (inet_pton(AF_INET6, host, &addr6.sin6_addr) != 1)
        return false;
    
    return (memcmp(&addr6.sin6_addr, &_ep.sin6_addr, 16) == 0);
    
}

bool SocketEndPointIPv6::getHost(char* buffer, long size) {
    
    assert(buffer != NULL && size > 0);
    
    inet_ntop(AF_INET6, &_ep.sin6_addr, buffer, (socklen_t)size);
    
    return true;
    
}

uint16_t SocketEndPointIPv6::getPort() {
    
    return ntohs(_ep.sin6_port);
    
}

bool SocketEndPointIPv6::compareWithAddress(struct sockaddr* endPoint) {
    
    if (endPoint->sa_family == AF_INET6)
        return (memcmp(&((struct sockaddr_in6*)endPoint)->sin6_addr, &_ep.sin6_addr, sizeof(in6_addr)) == 0);
    
    return false;
    
}

bool SocketEndPointIPv6::compareWithAddress(SocketEndPoint* endPoint) {
    
    if (endPoint != NULL && endPoint->isIPv6())
        return compareWithAddress((struct sockaddr*)&((SocketEndPointIPv6*)endPoint)->_ep);
    
    return false;
    
}

struct sockaddr* SocketEndPointIPv6::getSocketAddress() {
    
    return (struct sockaddr*) &_ep;
    
}

SocketEndPoint* SocketEndPointIPv6::copy(uint16_t newPort) {
    
    SocketEndPointIPv6* ret = new SocketEndPointIPv6((struct sockaddr*) &_ep);
    
    if (newPort > 0)
        ret->_ep.sin6_port = htons(newPort);
    
    return ret;
    
}
