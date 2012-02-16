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

const char* RAOPServer::clientConnectedNotificationName = "serverConnectionCreated";
const char* RAOPServer::localhostConnectedErrorNotificationName = "serverLocalhostConnectedError";

RAOPServer::RAOPServer(const char* host, int port) {
    
    _server = new WebServer(host, port);
    _server->setAcceptConnectionCallback(RAOPServer::_acceptConnectionCallback, this);
    
}

RAOPServer::RAOPServer(int port) {
    
    _server = new WebServer(port);
    _server->setAcceptConnectionCallback(RAOPServer::_acceptConnectionCallback, this);
        
}

RAOPServer::~RAOPServer() {
    
    delete _server;
    
}

bool RAOPServer::startServer() {
    
    return _server->startServer();
    
}

void RAOPServer::waitServer() {
    
    _server->waitServer();
        
}

void RAOPServer::stopServer() {
    
    _server->startServer();
    
}

bool RAOPServer::isRunning() {
    
    return _server->isRunning();
    
}

bool RAOPServer::_acceptConnectionCallback(WebConnection* newConnection, void* ctx) {
    
    char localIp[50];
    char remoteIp[50];
    
    newConnection->getLocalEndPoint()->getHost(localIp, 50);
    newConnection->getRemoteEndPoint()->getHost(remoteIp, 50);

//    if (strcmp(localIp, remoteIp) == 0) {
        
//        log(LOG_ERROR, "Refused connection from localhost.");
//        NotificationCenter::defaultCenter()->postNotification(RAOPServer::localhostConnectedErrorNotificationName, ctx, NULL);
        
//        return false;
        
//    } else {
        
        RAOPConnection* newRAOPConnection = new RAOPConnection(newConnection);
        
        NotificationCenter::defaultCenter()->postNotification(RAOPServer::clientConnectedNotificationName, ctx, newRAOPConnection); // <-- NULL = connection
        
        return true;
        
//    }

    
}
