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
#include "Socket.h"

class RAOPServer;
class RAOPConnection;

typedef void(*connectionCreatedClbk)(RAOPServer* server, RAOPConnection* newConnection, void* ctx);

class RAOPServer {
    
public:
    RAOPServer(const char* host, int port);
    RAOPServer(int port);
    RAOPServer();
    ~RAOPServer();

    bool startServer();
    void waitServer();
    void stopServer();
    
    bool isRunning();
    
    void setConnectionCallback(connectionCreatedClbk clbk, void* ctx);
    
private:
    void _setup();
    static void* _serverLoopKickStarter(void* t);
    void _serverLoop();
    
    bool _isRunning;
    
    pthread_t _serverLoopThread;
    
    Socket* _socket;
    SocketEndPoint* _localEndPoint;
    
    connectionCreatedClbk _connCreatedClbk;
    void* _connCreatedCtx;
    
};

#endif