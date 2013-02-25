//
//  RTPSocket.cpp
//  AirFloat
//
//  Created by Kristian Trenskow on 2/20/13.
//
//

#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>

#include "RTPSocket.h"

RTPSocket::RTPSocket(const char* name, struct sockaddr* allowedRemoteEndPoint) {
    
    _callback = NULL;
    _ctx = NULL;
    _sockets = NULL;
    _socketsCount = 0;
    
    _allowedRemoteEndPoint = sockaddr_copy(allowedRemoteEndPoint);
    
    _name = (char *)malloc(strlen(name) + 1);
    strcpy(_name, name);
    
    _mutex = mutex_create();
    
}

RTPSocket::~RTPSocket() {
    
    mutex_lock(_mutex);
    
    for (uint32_t i = 0 ; i < _socketsCount ; i++)
        socket_close(_sockets[i]->socket);
    while (_socketsCount > 0) {
        mutex_unlock(_mutex);
        thread_join(_sockets[0]->thread);
        mutex_lock(_mutex);
    }
    
    mutex_unlock(_mutex);
    
    sockaddr_destroy(_allowedRemoteEndPoint);
    free(_name);
    if (_sockets)
        free(_sockets);
    
    mutex_destroy(_mutex);
    
}

bool RTPSocket::setup(struct sockaddr* localEndPoint) {
    
    socket_p udpSocket = socket_create(true);
    socket_p tcpSocket = socket_create(false);
    
    if (socket_bind(udpSocket, localEndPoint) && socket_bind(tcpSocket, localEndPoint) && socket_listen(tcpSocket)) {
        _kickStart("UDP", udpSocket, true);
        _kickStart("TCP Listener", tcpSocket, false);
        return true;
    }
    
    socket_destroy(udpSocket);
    socket_destroy(tcpSocket);
    
    return false;
    
}

void RTPSocket::setDataReceivedCallback(dataReceivedCallback callback, void* ctx) {
    
    _callback = callback;
    _ctx = ctx;
    
}

void RTPSocket::sendTo(struct sockaddr* dst, const char* buffer, long size) {
    
    for (uint32_t i = 0 ; i < _socketsCount ; i++)
        if (_sockets[i]->dataSocket)
            socket_send_to(_sockets[i]->socket, dst, buffer, size);
    
}

void RTPSocket::_acceptLoop(socket_p socket) {
    
    for (;;) {
        
        socket_p newSocket = socket_accept(socket);
        
        if (!newSocket)
            break;
        
        if (sockaddr_equals_host(socket_get_remote_end_point(newSocket), _allowedRemoteEndPoint))
            _kickStart("TCP Connection", newSocket, true);
        else {
            socket_close(newSocket);
            socket_destroy(newSocket);
        }
        
    }
    
}

void RTPSocket::_receiveLoop(socket_p socket) {
    
    uint32_t offset = 0;
    unsigned char buffer[32768];
    
    for (;;) {
        
        if (offset == 32768)
            break;
        
        int read = socket_receive(socket, &buffer[offset], 32768 - offset);
        
        if (read <= 0)
            break;
        
        uint32_t used = read;
        
        if (sockaddr_equals_host(socket_get_remote_end_point(socket), _allowedRemoteEndPoint) && _callback != NULL)
            used = _callback(this, socket, (const char*)buffer, read + offset, _ctx);
        
        assert(used <= read + offset);
        
        offset = read + offset - used;
        if (used > 0 && offset > 0)
            memcpy(buffer, &buffer[used], offset);
        
    }
    
}

void RTPSocket::_loopKickStarter(void* ctx) {
    
    SocketInfo* info = (SocketInfo*)ctx;
    RTPSocket* owner = info->owner;
    
    pthread_setname_np(info->name);
    
    if (info->dataSocket)
        owner->_receiveLoop(info->socket);
    else
        owner->_acceptLoop(info->socket);
    
    owner->_removeSocket(info);
    
    pthread_exit(0);
    
}

void RTPSocket::_kickStart(const char *name, socket_p socket, bool dataSocket) {
    
    SocketInfo* info = (SocketInfo*)malloc(sizeof(SocketInfo));
    bzero(info, sizeof(SocketInfo));
    info->owner = this;
    info->socket = socket;
    info->dataSocket = dataSocket;
    info->name = (char*)malloc(strlen(_name) + 4 + strlen(name));
    sprintf(info->name, "%s - %s", _name, name);
    
    mutex_lock(_mutex);
    
    _sockets = (SocketInfo**)realloc(_sockets, sizeof(SocketInfo*) * (_socketsCount + 1));
    _sockets[_socketsCount] = info;
    _socketsCount++;
    
    mutex_unlock(_mutex);
    
    info->thread = thread_create(RTPSocket::_loopKickStarter, info);
    
}

void RTPSocket::_removeSocket(SocketInfo* info) {
    
    mutex_lock(_mutex);
    
    for (uint32_t i = 0 ; i < _socketsCount ; i++)
        if (_sockets[i] == info) {
            for (uint32_t a = i + 1 ; a < _socketsCount ; a++)
                _sockets[a - 1] = _sockets[a];
            _socketsCount--;
            break;
        }
    
    mutex_unlock(_mutex);
    
    socket_destroy(info->socket);
    free(info->name);
    free(info);
    
}
