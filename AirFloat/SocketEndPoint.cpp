//
//  SocketEndPoint.cpp
//  AirFloatCF
//
//  Created by Kristian Trenskow on 5/10/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#include <string.h>
#include <arpa/inet.h>
#include <assert.h>

#include "Log.h"
#include "SocketEndPoint.h"

SocketEndPoint::SocketEndPoint() {
    
    memset(&_ep, 0, sizeof(_ep));
    _ep.sin6_family = AF_INET6;
    
}

SocketEndPoint::SocketEndPoint(const char* host, uint16_t port, unsigned int scopeId) {
    
    setup(host, port, scopeId);
    
}

SocketEndPoint::SocketEndPoint(struct sockaddr* addr) {
    
    memcpy(&_ep, addr, sizeof(struct sockaddr_in6));
    
}

SocketEndPoint& SocketEndPoint::operator=(SocketEndPoint &ep) {
    
    memcpy(&_ep, &ep._ep, sizeof(_ep));
    return *this;
    
}

void SocketEndPoint::setup(const char* host, uint16_t port, unsigned int scopeId) {
    
    assert(port > 0);
    
    memset(&_ep, 0, sizeof(_ep));
    _ep.sin6_family = AF_INET6;
    _ep.sin6_addr = in6addr_any;
    _ep.sin6_port = htons(port);
    _ep.sin6_scope_id = scopeId;
    
    if (host != NULL) {
        struct in6_addr addr;
        if (inet_pton(AF_INET6, host, &addr) != 1)
            log(LOG_ERROR, "Error inet_pton ipv6");
        _ep.sin6_addr = addr;
        
    }
    
}

void SocketEndPoint::setupIPv6(uint16_t port, unsigned int scopeId) {
    
    struct sockaddr_in6 addr6;
    memset(&addr6, 0, sizeof(addr6));
    addr6.sin6_family = AF_INET6;
    addr6.sin6_addr = in6addr_any;
    addr6.sin6_port = htons(port);
    addr6.sin6_scope_id = scopeId;
    
    memcpy(&_ep, &addr6, sizeof(struct sockaddr_in6));
    
}

bool SocketEndPoint::isHost(const char* host) {
    
    assert(host != NULL);
    
    struct sockaddr_in6 addr6;
    if (inet_pton(AF_INET6, host, &addr6.sin6_addr) != 1)
        return false;
            
    return (memcmp(&addr6.sin6_addr, &_ep.sin6_addr, 16) == 0);
            
}

bool SocketEndPoint::getHost(char* buffer, long size) {
    
    assert(buffer != NULL && size > 0);
    
    if (size >= 50) {
        inet_ntop(AF_INET6, &_ep.sin6_addr, buffer, (socklen_t)size);
        return true;
    }
                        
    return false;
    
}

uint16_t SocketEndPoint::getPort() {
    
    return ntohs(_ep.sin6_port);
    
}

unsigned int SocketEndPoint::getScopeId() {
    
    return _ep.sin6_scope_id;
    
}

struct sockaddr* SocketEndPoint::getSocketAddress() {
    
    return (sockaddr*) &_ep;
    
}

in6_addr SocketEndPoint::IPv4AddressToIPv6Address(in_addr addr) {
    
    in6_addr ret;
    
    memset(&ret, 0, sizeof(in6_addr));
    
    ret.__u6_addr.__u6_addr8[10] = ret.__u6_addr.__u6_addr8[11] = 0xFF;
    ret.__u6_addr.__u6_addr32[3] = addr.s_addr;
    
    return ret;
    
}
