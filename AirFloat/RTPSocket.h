//
//  RTPSocket.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/20/13.
//
//

#ifndef _AirFloat_RTPSocket_h
#define _AirFloat_RTPSocket_h

#include "Mutex.h"
#include "Socket.h"

class RTPSocket;

typedef uint32_t(*dataReceivedCallback)(RTPSocket* rtpSocket, Socket* socket, const char* buffer, uint32_t size, void* ctx);

class RTPSocket {
    
public:
    RTPSocket(const char* name, SocketEndPoint* allowedRemoteEndPoint);
    virtual ~RTPSocket();
    
    bool setup(SocketEndPoint* localEndPoint);
    void setDataReceivedCallback(dataReceivedCallback callback, void* ctx);
    void sendTo(SocketEndPoint* dst, const char* buffer, long size);
    
private:
    dataReceivedCallback _callback;
    void* _ctx;
    
    void _acceptLoop(Socket* socket);
    void _receiveLoop(Socket* socket);
    static void* _loopKickStarter(void* ctx);
    
    void _kickStart(const char* name, Socket* socket, bool dataSocket);
    
    char* _name;
    
    SocketEndPoint *_allowedRemoteEndPoint;
    
    typedef struct {
        
        RTPSocket* owner;
        Socket* socket;
        bool dataSocket;
        pthread_t thread;
        char* name;
        
    } SocketInfo;
    
    void _removeSocket(SocketInfo* info);
    
    SocketInfo** _sockets;
    uint32_t _socketsCount;
    
    Mutex _mutex;
    
};

#endif
