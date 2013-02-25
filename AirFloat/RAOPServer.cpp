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
#include "RAOPConnection.h"
#include "RAOPServer.h"

extern "C" {
#include "log.h"
#include "webconnection.h"
}

const char* RAOPServer::localhostConnectedErrorNotificationName = "serverLocalhostConnectedError";

RAOPServer::RAOPServer(uint16_t port) {
    
    _port = port;
    
    _server = web_server_create((sockaddr_type)(sockaddr_type_inet_4 | sockaddr_type_inet_6));
    web_server_set_accept_callback(_server, RAOPServer::_acceptConnectionCallback, this);
    
}

RAOPServer::~RAOPServer() {
    
    stopServer();
    waitServer();
    
    web_server_destroy(_server);
    
}

bool RAOPServer::startServer() {
    
    bool ret = web_server_start(_server, _port, 10);
    
    if (ret)
        log_message(LOG_INFO, "Server started at port %d", sockaddr_get_port(web_server_get_local_end_point(_server, sockaddr_type_inet_4)));
    else
        log_message(LOG_INFO, "Unable to start server");
    
    return ret;
    
}

void RAOPServer::waitServer() {
    
    web_server_wait_stop(_server);
    
}

void RAOPServer::stopServer() {
    
    web_server_stop(_server);
    
}

bool RAOPServer::isRunning() {
    
    return web_server_is_running(_server);
    
}

uint32_t RAOPServer::getConnectionCount() {
    
    return web_server_get_connection_count(_server);
    
}

struct sockaddr* RAOPServer::getLocalEndPoint() {
    
    return web_server_get_local_end_point(_server, sockaddr_type_inet_4);
    
}

bool RAOPServer::_acceptConnectionCallback(web_server_p server, web_connection_p newConnection, void* ctx) {
    
#ifndef DEBUG
    if (sockaddr_equals_host(newConnection->getLocalEndPoint(), newConnection->getRemoteEndPoint()) && RTPReceiver::getStreamingReceiver() == NULL) {
        
        log_message(LOG_ERROR, "Refused connection from localhost");
        NotificationCenter::defaultCenter()->postNotification(RAOPServer::localhostConnectedErrorNotificationName, ctx, NULL);
        
        return false;
        
    } else {
#endif
        new RAOPConnection(newConnection);
        
        return true;
#ifndef DEBUG
    }
#endif
    
}
