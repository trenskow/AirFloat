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

#include "NotificationCenter.h"
#include "Log.h"
#include "WebConnection.h"
#include "RAOPConnection.h"
#include "RAOPServer.h"

const char* RAOPServer::localhostConnectedErrorNotificationName = "serverLocalhostConnectedError";

RAOPServer::RAOPServer(uint16_t port) {
    
    _port = port;
    
    _server = new WebServer((SocketEndPointType)(kSocketEndPointTypeIPv4 | kSocketEndPointTypeIPv6));
    _server->setAcceptConnectionCallback(RAOPServer::_acceptConnectionCallback, this);
    
}

RAOPServer::~RAOPServer() {
    
    stopServer();
    waitServer();
    
    delete _server;
    
}

bool RAOPServer::startServer() {
    
    bool ret = _server->startServer(_port, 10);
    
    if (ret)
        log(LOG_INFO, "Server started at port %d", _server->getLocalEndPoint()->getPort());
    else
        log(LOG_INFO, "Unable to start server");
    
    return ret;
    
}

void RAOPServer::waitServer() {
    
    _server->waitServer();
        
}

void RAOPServer::stopServer() {
    
    _server->stopServer();
    
}

bool RAOPServer::isRunning() {
    
    return _server->isRunning();
    
}

uint32_t RAOPServer::getConnectionCount() {
    
    return _server->getConnectionCount();
    
}

SocketEndPoint* RAOPServer::getLocalEndPoint() {
    
    return _server->getLocalEndPoint();
    
}

bool RAOPServer::_acceptConnectionCallback(WebServer* server, WebConnection* newConnection, void* ctx) {
    
    char localIp[50];
    char remoteIp[50];
    
    newConnection->getLocalEndPoint()->getHost(localIp, 50);
    newConnection->getRemoteEndPoint()->getHost(remoteIp, 50);

//    if (strcmp(localIp, remoteIp) == 0 && RTPReceiver::getStreamingReceiver() == NULL) {
        
//        log(LOG_ERROR, "Refused connection from localhost.");
//        NotificationCenter::defaultCenter()->postNotification(RAOPServer::localhostConnectedErrorNotificationName, ctx, NULL);
        
//        return false;
        
//    } else {

        new RAOPConnection(newConnection);
        
        return true;
        
//    }
    
}
