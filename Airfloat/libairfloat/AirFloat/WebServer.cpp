//
//  WebServer.cpp
//  AirFloat
//
//  Created by Kristian Trenskow on 2/14/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "Log.h"
#include "WebConnection.h"
#include "WebServer.h"

typedef struct {
    
    WebServer* sender;
    Socket* socket;
    
} SocketKickStarterData;

WebServer::WebServer(SocketEndPointType socketTypes) {
    
    _socketTypes = socketTypes;
    
    _isRunning = false;    
    _acceptConnectionCallback = NULL;
    
    _connections = NULL;
    _connectionCount = 0;
    
}

WebServer::~WebServer() {
    
    stopServer();
    
}

Socket* WebServer::_bindSocket(uint16_t port, SocketEndPointType socketType) {
    
    Socket* socket = NULL;
    
    if (socketType && _socketTypes != 0) {
        
        socket = new Socket();
        SocketEndPoint* endPoint = SocketEndPoint::createSocket(port, socketType);
        
        bool ret = socket->Bind(endPoint);
        if (ret)
            ret = (ret && socket->Listen());
        
        delete endPoint;
        
        if (!ret) {
            delete socket;
            socket = NULL;
        }
        
    }
    
    return socket;
    
}

bool WebServer::startServer(uint16_t port, uint16_t portRange) {
    
    _mutex.lock();
    
    if (_isRunning) {
        log(LOG_ERROR, "Server already running");
        _mutex.unlock();
        return false;
    }
    
    for (uint16_t p = port ; p < port + portRange ; p++) {
        
        log(LOG_INFO, "Trying port %d", p);
        
        _socketIPv4 = _bindSocket(p, kSocketEndPointTypeIPv4);
        _socketIPv6 = _bindSocket(p, kSocketEndPointTypeIPv6);
        
        if ((((_socketTypes & kSocketEndPointTypeIPv4) == 0) || _socketIPv4 != NULL) && (((_socketTypes & kSocketEndPointTypeIPv6) == 0) || _socketIPv6 != NULL)) {
            
            if ((_socketTypes & kSocketEndPointTypeIPv4) != 0)
                _startServerLopp(&_serverLoopThreadV4, _socketIPv4);
            if ((_socketTypes & kSocketEndPointTypeIPv6) != 0)
                _startServerLopp(&_serverLoopThreadV6, _socketIPv6);
            
            _isRunning = true;
            
            _mutex.unlock();
            
            return true;
            
        }
        
        if (_socketIPv4)
            delete _socketIPv4;
        if (_socketIPv6)
            delete _socketIPv6;
        
    }
    
    _mutex.unlock();
    
    return false;
        
}

void WebServer::waitServer() {
    
    if ((_socketTypes & kSocketEndPointTypeIPv4) != 0)
        pthread_join(_serverLoopThreadV4, NULL);
    if ((_socketTypes & kSocketEndPointTypeIPv6) != 0)
        pthread_join(_serverLoopThreadV6, NULL);
    
}

void WebServer::stopServer() {
    
    _mutex.lock();
    
    if (_socketIPv4 != NULL)
        _socketIPv4->Close();
    if (_socketIPv6 != NULL)
        _socketIPv6->Close();
    
    waitServer();
    
    if (_socketIPv4 != NULL) {
        delete _socketIPv4;
        _socketIPv4 = NULL;
    }
    if (_socketIPv6 != NULL) {
        delete _socketIPv6;
        _socketIPv6 = NULL;
    }
    
    _isRunning = true;
    
    while (_connectionCount > 0) {
        _connections[0]->closeConnection();
        _connections[0]->waitConnection();
    }
    
    _mutex.unlock();
    
    log(LOG_INFO, "Server stopped");
    
}

bool WebServer::isRunning() {
    
    _mutex.lock();
    bool ret = _isRunning;
    _mutex.unlock();
    
    return ret;
    
}

uint32_t WebServer::getConnectionCount() {
    
    return _connectionCount;
    
}

SocketEndPoint* WebServer::getLocalEndPoint(SocketEndPointType socketType) {
    
    if (socketType == kSocketEndPointTypeIPv4)
        return _socketIPv4->GetLocalEndPoint();
    
    if (socketType == kSocketEndPointTypeIPv6)
        return _socketIPv6->GetLocalEndPoint();
    
    return NULL;
    
}

void WebServer::setAcceptConnectionCallback(acceptCallback callback, void* ctx) {
    
    _acceptConnectionCallback = callback;
    _acceptConnectionCallbackCtx = ctx;
    
}

void WebServer::_serverLoop(Socket* socket) {
        
    for (;;) {
        
        Socket* newSocket = socket->Accept();
        
        if (newSocket == NULL)
            break;
        
        _mutex.lock();
        
        WebConnection* newConnection = new WebConnection(newSocket, this);
        
        bool shouldLive = (_acceptConnectionCallback != NULL && _acceptConnectionCallback(this, newConnection, _acceptConnectionCallbackCtx));
        
        if (!shouldLive)
            delete newConnection;
        else {
            
            _connections = (WebConnection**)realloc(_connections, sizeof(WebConnection*) * (_connectionCount + 1));
            _connections[_connectionCount] = newConnection;
            _connectionCount++;
            
            newConnection->_takeOff();
            
        }
        
        _mutex.unlock();
        
    }
    
}

void* WebServer::_serverLoopKickStarter(void* t) {
    
    SocketKickStarterData* data = (SocketKickStarterData*)t;
    
    WebServer* sender = data->sender;
    Socket* socket = data->socket;
    
    free(data);
    
    pthread_setname_np("Server Socket");
    sender->_serverLoop(socket);
    pthread_exit(0);
    
}

void WebServer::_startServerLopp(pthread_t* thread, Socket* socket) {
    
    SocketKickStarterData* data = (SocketKickStarterData*)malloc(sizeof(SocketKickStarterData));
    data->sender = this;
    data->socket = socket;
    pthread_create(thread, NULL, _serverLoopKickStarter, data);
    
}

void WebServer::_connectionClosed(WebConnection* connection) {
    
    _mutex.lock();
    
    for (uint32_t i = 0 ; i < _connectionCount ; i++)
        if (_connections[i] == connection) {
            _connections[i]->_shouldSelfDestroy = true;
            for (uint32_t x = i ; x < _connectionCount - 1 ; x++)
                _connections[x] = _connections[x + 1];
            _connectionCount--;
            _mutex.unlock();
            return;
        }
    
    log(LOG_ERROR, "Server knows nothing about connection %p - Aborting", connection);
    assert(false);
    
}
