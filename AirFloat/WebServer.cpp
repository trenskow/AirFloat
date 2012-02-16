//
//  WebServer.cpp
//  AirFloat
//
//  Created by Kristian Trenskow on 2/14/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#include <pthread.h>
#include <string.h>
#include "Log.h"
#include "WebConnection.h"
#include "WebServer.h"

WebServer::WebServer(const char* host, uint32_t port) {
    
    _isRunning = false;    
    _acceptConnectionCallback = NULL;
    
    _localEndPoint = new SocketEndPoint(host, port);
        
}

WebServer::WebServer(uint32_t port) {
    
    _isRunning = false;
    _acceptConnectionCallback = NULL;
    
    _localEndPoint = new SocketEndPoint();
    _localEndPoint->setupIPv6(port);

}

WebServer::~WebServer() {
    
    delete _localEndPoint;
    
}

bool WebServer::startServer() {
    
    _mutex.lock();
    
    if (_isRunning) {
        log(LOG_ERROR, "Server already running");
        _mutex.unlock();
        return false;
    }
    
    _socket = new Socket(false);
    
    if (_socket != NULL) {
        
        if (_socket->Bind(_localEndPoint))
            if (_socket->Listen()) {
                
                _mutex.unlock();
                
                pthread_create(&_serverLoopThread, NULL, _serverLoopKickStarter, this);
                log(LOG_INFO, "Server started");
                return true;
                
            } else
                log(LOG_ERROR, "Unable to listen on socket");
        
            else
                log(LOG_ERROR, "Unable to bind socket");
        
        
    } else
        log(LOG_ERROR, "Unable to create socket");
    
    _mutex.unlock();
    
    return false;
        
}

void WebServer::waitServer() {
    
    pthread_join(_serverLoopThread, NULL);
    
}

void WebServer::stopServer() {
    
    delete _socket;
    
    waitServer();
    
    log(LOG_INFO, "Server stopped");
    
}

bool WebServer::isRunning() {
    
    _mutex.lock();
    bool ret = _isRunning;
    _mutex.unlock();
    
    return ret;
    
}

void WebServer::setAcceptConnectionCallback(acceptCallback callback, void* ctx) {
    
    _acceptConnectionCallback = callback;
    _acceptConnectionCallbackCtx = ctx;
    
}

void WebServer::_serverLoop() {
    
    _mutex.lock();
    _isRunning = true;
    
    for (;;) {
        
        _mutex.unlock();
        Socket* newSocket = _socket->Accept();
        _mutex.lock();
        if (newSocket == NULL) {
            break;
        }
        
        char localip[50];
        char remoteip[50];
        newSocket->GetLocalEndPoint()->getHost(localip, 50);
        newSocket->GetRemoteEndPoint()->getHost(remoteip, 50);
        
        WebConnection* newConnection = new WebConnection(newSocket);
        
        bool shouldLive = (_acceptConnectionCallback != NULL && _acceptConnectionCallback(newConnection, _acceptConnectionCallbackCtx));
        
        if (!shouldLive)
            delete newConnection;
        
    }
    
    _isRunning = false;
    
    _mutex.unlock();
        
}

void* WebServer::_serverLoopKickStarter(void* t) {
    
    pthread_setname_np("Server Socket");
    ((WebServer*)t)->_serverLoop();
    pthread_exit(0);
    
}
