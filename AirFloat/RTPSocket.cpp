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

RTPSocket::RTPSocket(const char* name, SocketEndPoint *allowedRemoteEndPoint) {
    
    _callback = NULL;
    _ctx = NULL;
    _sockets = NULL;
    _socketsCount = 0;
    
    _allowedRemoteEndPoint = allowedRemoteEndPoint->copy();
    
    _name = (char *)malloc(strlen(name) + 1);
    strcpy(_name, name);
    
}

RTPSocket::~RTPSocket() {
    
    _mutex.lock();
    
    for (uint32_t i = 0 ; i < _socketsCount ; i++)
        _sockets[i]->socket->Close();
    while (_socketsCount > 0) {
        _mutex.unlock();
        pthread_join(_sockets[0]->thread, NULL);
        _mutex.lock();
    }
    
    _mutex.unlock();
    
    delete _allowedRemoteEndPoint;
    free(_name);
    if (_sockets)
        free(_sockets);
        
}

bool RTPSocket::setup(SocketEndPoint *localEndPoint) {
    
    Socket* udpSocket = new Socket(true);
    Socket* tcpSocket = new Socket();
    
    if (udpSocket->Bind(localEndPoint) && tcpSocket->Bind(localEndPoint) && tcpSocket->Listen()) {
        _kickStart("UDP", udpSocket, true);
        _kickStart("TCP Listener", tcpSocket, false);
        return true;
    }
    
    delete udpSocket;
    delete tcpSocket;
    
    return false;
    
}

void RTPSocket::setDataReceivedCallback(dataReceivedCallback callback, void* ctx) {
    
    _callback = callback;
    _ctx = ctx;
    
}

void RTPSocket::sendTo(SocketEndPoint* dst, const char* buffer, long size) {
    
    for (uint32_t i = 0 ; i < _socketsCount ; i++)
        if (_sockets[i]->dataSocket)
            _sockets[i]->socket->SendTo(dst, buffer, size);
    
}

void RTPSocket::_acceptLoop(Socket* socket) {
    
    for (;;) {
        
        Socket* newSocket = socket->Accept();
        
        if (!newSocket)
            break;
        
        if (newSocket->GetRemoteEndPoint()->compareWithAddress(_allowedRemoteEndPoint))
            _kickStart("TCP Connection", newSocket, true);
        else {
            newSocket->Close();
            delete newSocket;
        }
        
    }
    
}

void RTPSocket::_receiveLoop(Socket* socket) {
    
    uint32_t offset = 0;
    unsigned char buffer[32768];
    
    for (;;) {
        
        if (offset == 32768)
            break;
        
        int read = socket->Receive(&buffer[offset], 32768 - offset);
        
        if (read <= 0)
            break;
        
        uint32_t used = read;
        
        if (socket->GetRemoteEndPoint()->compareWithAddress(_allowedRemoteEndPoint) && _callback != NULL)
            used = _callback(this, socket, (const char*)buffer, read + offset, _ctx);
        
        assert(used <= read + offset);
        
        offset = read + offset - used;
        if (used > 0 && offset > 0)
            memcpy(buffer, &buffer[used], offset);
        
    }
    
}

void* RTPSocket::_loopKickStarter(void* ctx) {
    
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

void RTPSocket::_kickStart(const char *name, Socket* socket, bool dataSocket) {
    
    SocketInfo* info = (SocketInfo*)malloc(sizeof(SocketInfo));
    bzero(info, sizeof(SocketInfo));
    info->owner = this;
    info->socket = socket;
    info->dataSocket = dataSocket;
    info->name = (char*)malloc(strlen(_name) + 4 + strlen(name));
    sprintf(info->name, "%s - %s", _name, name);
    
    _mutex.lock();
    
    _sockets = (SocketInfo**)realloc(_sockets, sizeof(SocketInfo*) * (_socketsCount + 1));
    _sockets[_socketsCount] = info;
    _socketsCount++;
    
    _mutex.unlock();
    
    pthread_create(&info->thread, NULL, _loopKickStarter, info);
    
}

void RTPSocket::_removeSocket(SocketInfo* info) {
    
    _mutex.lock();
    
    for (uint32_t i = 0 ; i < _socketsCount ; i++)
        if (_sockets[i] == info) {
            for (uint32_t a = i + 1 ; a < _socketsCount ; a++)
                _sockets[a - 1] = _sockets[a];
            _socketsCount--;
            break;
        }
    
    _mutex.unlock();
    
    delete info->socket;
    free(info->name);
    free(info);
    
}
