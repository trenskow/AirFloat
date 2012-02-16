//
//  WebServer.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/14/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#ifndef AirFloat_WebServer_h
#define AirFloat_WebServer_h

#include <stdint.h>
#include "Mutex.h"
#include "Socket.h"

class WebConnection;

typedef bool(*acceptCallback)(WebConnection*, void* ctx);

class WebServer {
    
public:
    WebServer(const char* host, uint32_t port);
    WebServer(uint32_t port);
    ~WebServer();
    
    bool startServer();
    void waitServer();
    void stopServer();
    
    bool isRunning();
    
    void setAcceptConnectionCallback(acceptCallback callback, void* ctx);
    
private:
    
    static void* _serverLoopKickStarter(void* t);
    void _serverLoop();
    
    bool _isRunning;
    
    pthread_t _serverLoopThread;
    
    acceptCallback _acceptConnectionCallback;
    void* _acceptConnectionCallbackCtx;
    
    Socket* _socket;
    SocketEndPoint* _localEndPoint;
    
    Mutex _mutex;

};

#endif
