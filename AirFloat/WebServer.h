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

class WebServer;
class WebConnection;

typedef bool(*acceptCallback)(WebServer* server, WebConnection* connection, void* ctx);

class WebServer {
    
    friend class WebConnection;
    
public:
    WebServer(SocketEndPointType socketTypes);
    ~WebServer();
    
    bool startServer(uint16_t port = 80, uint16_t portRange = 1);
    void waitServer();
    void stopServer();
    
    bool isRunning();
    uint32_t getConnectionCount();
    
    SocketEndPoint* getLocalEndPoint(SocketEndPointType socketType = kSocketEndPointTypeIPv4);
    
    void setAcceptConnectionCallback(acceptCallback callback, void* ctx);
    
private:
    
    Socket* _bindSocket(uint16_t port, SocketEndPointType socketType);
    
    static void* _serverLoopKickStarter(void* t);
    void _serverLoop(Socket* socket);
    void _startServerLopp(pthread_t* thread, Socket* socket);
    
    void _connectionClosed(WebConnection* connection);
    
    Socket* _socketIPv4;
    Socket* _socketIPv6;
    SocketEndPointType _socketTypes;
    
    bool _isRunning;
    
    pthread_t _serverLoopThreadV4;
    pthread_t _serverLoopThreadV6;
    
    acceptCallback _acceptConnectionCallback;
    void* _acceptConnectionCallbackCtx;
    
    WebConnection** _connections;
    uint32_t _connectionCount;
    
    Mutex _mutex;

};

#endif
