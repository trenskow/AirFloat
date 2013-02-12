//
//  Socket.cpp
//  AirFloat
//
//  Created by Kristian Trenskow on 3/5/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include "Log.h"
#include "Socket.h"

#include "SocketEndPointIPv4.h"
#include "SocketEndPointIPv6.h"

Socket::Socket(bool isUDP) {
    
    _isUDP = isUDP;
    _socket = -1;
    
    _localEndPoint = _remoteEndPoint = NULL;
    
}

Socket::Socket(int socket) {
    
    assert(socket != 0);
    
    _isUDP = false;
    
    _socket = socket;
    
    _localEndPoint = _remoteEndPoint = NULL;
    
}


Socket::~Socket() {
    
    if (_socket > -1)        
        Close();
    
    if (_localEndPoint != NULL) {
        delete _localEndPoint;
        _localEndPoint = NULL;
    }
    if (_remoteEndPoint != NULL) {
        delete _remoteEndPoint;
        _remoteEndPoint = NULL;
    }
    
}

bool Socket::Bind(SocketEndPoint* ep) {
    
    assert(ep != NULL);
    
    if (_localEndPoint != NULL)
        delete _localEndPoint;
    
    _localEndPoint = ep->copy();
    struct sockaddr* addr = _localEndPoint->getSocketAddress();
    
    if (_socket < 0) {
        
        _socket = socket(addr->sa_family, (_isUDP ? SOCK_DGRAM : SOCK_STREAM), (_isUDP ? IPPROTO_UDP : IPPROTO_TCP));
        
        if (_socket < 0)
            log(LOG_INFO, "Socket creation error: %s", strerror(errno));

        if (ep->isIPv6()) {
            int32_t on = 1;
            setsockopt(_socket, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(on));
        }
        
    }
    
    if (bind(_socket, addr, addr->sa_len) == 0)
        return true;
    
    return false;
        
}

bool Socket::Listen() {
    
    if (listen(_socket, 5) == 0)
        return true;
    
    return false;
        
}

Socket* Socket::Accept() {
    
    struct sockaddr_storage client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int newSocket = accept(_socket, (struct sockaddr*)&client_addr, &addr_len);
    if (newSocket >= 0)
        return new Socket(newSocket);

    return NULL;
    
}

long Socket::Receive(unsigned char* buffer, long size) {
    
    assert(buffer != NULL && size > 0);
    
    struct sockaddr_storage remoteAddr;
    socklen_t remoteAddrLen = sizeof(sockaddr_storage);
    int ret = recvfrom(_socket, buffer, size, 0, (struct sockaddr*) &remoteAddr, &remoteAddrLen);
    
    if (_remoteEndPoint != NULL)
        delete _remoteEndPoint;
    
    _remoteEndPoint = SocketEndPoint::createSocket((struct sockaddr*) &remoteAddr);
    
    return ret;
    
    
}

long Socket::Send(const char* buffer, long size) {
    
    return send(_socket, buffer, size, 0);
    
}

long Socket::SendTo(SocketEndPoint* dst, const char* buffer, long size) {
    
    assert(dst != NULL && buffer != NULL && size > 0);
    
    if (!_isUDP)
        return Send(buffer, size);
    
    struct sockaddr* remoteSocketEndPoint = dst->getSocketAddress();
    socklen_t len = remoteSocketEndPoint->sa_len;
    
    assert(remoteSocketEndPoint->sa_family == _localEndPoint->getSocketAddress()->sa_family);
    
    long ret = sendto(_socket, buffer, size, 0, (struct sockaddr*) remoteSocketEndPoint, len);
    if (ret < 0)
        log(LOG_INFO, "Unable to send (errno: %d - %s)", errno, strerror(errno));
    
    return ret;
    
}

void Socket::Close() {
    
    close(_socket);
    _socket = -1;
    
}

SocketEndPoint* Socket::GetLocalEndPoint() {
    
    if (_localEndPoint == NULL && _socket > 0) {
        
        struct sockaddr_storage addr;
        socklen_t len = sizeof(sockaddr_storage);
        if (getsockname(_socket, (struct sockaddr*)&addr, &len) == 0)
            _localEndPoint = SocketEndPoint::createSocket((struct sockaddr*)&addr);
        
    }
    
    return _localEndPoint;
    
}

SocketEndPoint* Socket::GetRemoteEndPoint() {
    
    if (_remoteEndPoint == NULL && _socket > 0) {
        
        struct sockaddr_storage addr;
        socklen_t len = sizeof(sockaddr_storage);
        if (getpeername(_socket, (struct sockaddr*)&addr, &len) == 0)
            _remoteEndPoint = SocketEndPoint::createSocket((struct sockaddr*) &addr);
        
    }
    
    return _remoteEndPoint;
    
}
