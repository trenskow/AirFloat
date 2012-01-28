//
//  RAOPServer.cpp
//  AirFloatCF
//
//  Created by Kristian Trenskow on 5/10/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#include "Log.h"
#include "RAOPConnection.h"
#include "RAOPServer.h"

RAOPServer::RAOPServer(const char* host, int port) {
    
    _setup();
    
    _localEndPoint = new SocketEndPoint(host, port);
    
}

RAOPServer::RAOPServer(int port) {
    
    _setup();
    
    _localEndPoint = new SocketEndPoint();
    _localEndPoint->SetupIPv6(port);
    
}

RAOPServer::RAOPServer() {
    
    _setup();
    
    _localEndPoint = new SocketEndPoint();
    _localEndPoint->SetupIPv6(5000);
    
}

RAOPServer::~RAOPServer() {
    
    delete _localEndPoint;
    
}

void RAOPServer::_setup() {
    
    _connCreatedClbk = NULL;
    _connCreatedCtx = NULL;
    _isRunning = false;
    
}

void* RAOPServer::_serverLoopKickStarter(void* t) {
    
    pthread_setname_np("Server Socket");
    ((RAOPServer*)t)->_serverLoop();
    pthread_exit(0);
    
}

void RAOPServer::_serverLoop() {
    
    _isRunning = true;
    
    for (;;) {

        Socket* newSocket = _socket->Accept();
        if (newSocket == NULL) {
            log(LOG_INFO, "Server was stopped");
            break;
        }
        
        char localip[50];
        char remoteip[50];
        newSocket->LocalEndPoint()->GetHost(localip, 50);
        newSocket->RemoteEndPoint()->GetHost(remoteip, 50);
        
//        if (strcmp(localip, remoteip) != 0) {
            
            log(LOG_INFO, "Accepted connection from %s (%s)", remoteip, (IN6_IS_ADDR_V4MAPPED(&((struct sockaddr_in6*)newSocket->RemoteEndPoint()->SocketAdress())->sin6_addr) ? "IPv4in6" : "IPv6"));
            
            RAOPConnection* connection = new RAOPConnection(newSocket);
            
            connection->_takeOver();
            
            if (_connCreatedClbk != NULL)
                _connCreatedClbk(this, connection, _connCreatedCtx);
            
//        } else {
//            newSocket->Close();
//            log(LOG_ERROR, "Refused connection from localhost.");
//        }
        
        
    }
    
    _isRunning = false;
    
}

bool RAOPServer::startServer() {
    
    _socket = new Socket(false);
    
    if (_socket != NULL) {
        
        if (_socket->Bind(_localEndPoint))
            if (_socket->Listen()) {
                
                pthread_create(&_serverLoopThread, NULL, _serverLoopKickStarter, this);
                log(LOG_INFO, "Server started");
                return true;
                
            } else
                log(LOG_ERROR, "Unable to listen on socket");
        
        else
            log(LOG_ERROR, "Unable to bind socket");
        
        
    } else
        log(LOG_ERROR, "Unable to create socket");
    
    return false;
    
}

void RAOPServer::setConnectionCallback(connectionCreatedClbk clbk, void* ctx) {
    
    _connCreatedClbk = clbk;
    _connCreatedCtx = ctx;
    
}

void RAOPServer::waitServer() {
    
    pthread_join(_serverLoopThread, NULL);
    
}

void RAOPServer::stopServer() {
    
    delete _socket;

    pthread_join(_serverLoopThread, NULL);
    
    log(LOG_INFO, "Server stopped");
    
}

bool RAOPServer::isRunning() {
    
    return _isRunning;
    
}
