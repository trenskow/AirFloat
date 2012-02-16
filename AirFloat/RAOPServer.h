//
//  RAOPServer.h
//  AirFloatCF
//
//  Created by Kristian Trenskow on 5/10/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#ifndef __RAOPSERVER_H
#define __RAOPSERVER_H

#include <pthread.h>
#include "WebServer.h"

class WebConnection;

class RAOPServer {
    
public:
    RAOPServer(const char* host, int port);
    RAOPServer(int port);
    ~RAOPServer();

    bool startServer();
    void waitServer();
    void stopServer();
    
    bool isRunning();
    
    static const char* clientConnectedNotificationName;
    static const char* localhostConnectedErrorNotificationName;
    
private:
    
    static bool _acceptConnectionCallback(WebConnection* newConnection, void* ctx);
    
    WebServer* _server;
    
};

#endif