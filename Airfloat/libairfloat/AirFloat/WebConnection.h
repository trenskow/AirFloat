//
//  WebConnection.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/14/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#ifndef AirFloat_WebConnection_h
#define AirFloat_WebConnection_h

#include "WebTools.h"
#include "Socket.h"

#define READ_SIZE 16384

class WebServer;
class WebConnection;
class WebRequest;

typedef void(*processRequestCallback)(WebConnection* connection, WebRequest* request, void* ctx);
typedef void(*connectionClosedCallback)(WebConnection* connection, void* ctx);

class WebConnection : public WebTools {
    
    friend class WebServer;
    
public:    
    WebConnection(Socket* socket);
    ~WebConnection();
    
    SocketEndPoint* getLocalEndPoint();
    SocketEndPoint* getRemoteEndPoint();
    
    void setProcessRequestCallback(processRequestCallback callback);
    void setConnectionClosed(connectionClosedCallback callback);
    void setCallbackCtx(void* ctx);
    
    void setCallbackReady();
    
private:
    
    static void* _connectionLoopKickStarter(void* t);
    void _connectionLoop();
    
    pthread_t _connectionLoopThread;
    
    Socket* _socket;
    
    processRequestCallback _processRequestCallback;
    connectionClosedCallback _connectionClosedCallback;
    void* _callbackCtx;
    
};

#endif
