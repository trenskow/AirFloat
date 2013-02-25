//
//  RTPSocket.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/20/13.
//
//

#ifndef _AirFloat_RTPSocket_h
#define _AirFloat_RTPSocket_h

extern "C" {
#include "mutex.h"
#include "thread.h"
#include "sockaddr.h"
#include "socket.h"
}

class RTPSocket;

typedef uint32_t(*dataReceivedCallback)(RTPSocket* rtpSocket, socket_p socket, const char* buffer, uint32_t size, void* ctx);

class RTPSocket {
    
public:
    RTPSocket(const char* name, struct sockaddr* allowedRemoteEndPoint);
    virtual ~RTPSocket();
    
    bool setup(struct sockaddr* localEndPoint);
    void setDataReceivedCallback(dataReceivedCallback callback, void* ctx);
    void sendTo(struct sockaddr* dst, const char* buffer, long size);
    
private:
    dataReceivedCallback _callback;
    void* _ctx;
    
    void _acceptLoop(socket_p socket);
    void _receiveLoop(socket_p socket);
    static void _loopKickStarter(void* ctx);
    
    void _kickStart(const char* name, socket_p socket, bool dataSocket);
    
    char* _name;
    
    struct sockaddr* _allowedRemoteEndPoint;
    
    typedef struct {
        
        RTPSocket* owner;
        socket_p socket;
        bool dataSocket;
        thread_p thread;
        char* name;
        
    } SocketInfo;
    
    void _removeSocket(SocketInfo* info);
    
    SocketInfo** _sockets;
    uint32_t _socketsCount;
    
    mutex_p _mutex;
    
};

#endif
