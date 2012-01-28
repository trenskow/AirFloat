//
//  Socket.cpp
//  AirFloatCF
//
//  Created by Kristian Trenskow on 5/10/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#include "Log.h"
#include "Socket.h"

Socket::Socket(bool isUDP) {
    
    _isUDP = isUDP;
    _socket = -1;
    
}

Socket::Socket(int socket, struct sockaddr* addr) {
    
    assert(socket != 0 && addr != NULL);
    
    _socket = socket;
    socklen_t len = sizeof(struct sockaddr_in6);
    getsockname(_socket, (struct sockaddr*) &_localEndPoint._ep, &len);
    memcpy(&_remoteEndPoint._ep, addr, len);
    
}

Socket::~Socket() {
    
    Close(); // Shutdown
    close(_socket);
    
}

bool Socket::Bind(SocketEndPoint* ep) {
    
    assert(ep != NULL);
    
    if (_socket == -1)
        _socket = socket(AF_INET6, (_isUDP ? SOCK_DGRAM : SOCK_STREAM), (_isUDP ? IPPROTO_UDP : IPPROTO_TCP));
    
    struct sockaddr_in6* addr6 = (struct sockaddr_in6*) ep->SocketAdress();
    if (bind(_socket, (struct sockaddr*)addr6, sizeof(struct sockaddr_in6)) == 0) {
        _localEndPoint = *ep;
        return true;
    }
    
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
        return new Socket(newSocket, (struct sockaddr*)&client_addr);
        
    return NULL;
    
}

long Socket::Receive(unsigned char* buffer, long size) {
    
    assert(buffer != NULL && size > 0);
    
    if (!_isUDP)
        return recv(_socket, buffer, size, 0);
        
    socklen_t len = sizeof(struct sockaddr_in6);
    int ret = recvfrom(_socket, buffer, size, 0, (struct sockaddr*) &_remoteEndPoint._ep, &len);
    
    return ret;
    
}

long Socket::Send(const char* buffer, long size) {
    
    return send(_socket, buffer, size, 0);
    
}

long Socket::SendTo(SocketEndPoint* dst, const char* buffer, long size) {
    
    assert(dst != NULL && buffer != NULL && size > 0);
    
    if (!_isUDP)
        return Send(buffer, size);
    
    socklen_t len = sizeof(struct sockaddr_in6);
    
    long ret = sendto(_socket, buffer, size, 0, (struct sockaddr*) &dst->_ep, len);
    if (ret < 0)
        log(LOG_INFO, "Unable to send (errno: %d - %s)", errno, strerror(errno));
    
    return ret;
    
}

void Socket::Close() {
    
    shutdown(_socket, SHUT_RDWR);
    if (_isUDP)
        close(_socket);
        
}

SocketEndPoint* Socket::LocalEndPoint() {
    
    return &_localEndPoint;
    
}

SocketEndPoint* Socket::RemoteEndPoint() {
    
    return &_remoteEndPoint;
    
}
