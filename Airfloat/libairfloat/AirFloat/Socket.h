//
//  Socket.h
//  AirFloat
//
//  Created by Kristian Trenskow on 3/5/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#ifndef __SOCKET_H
#define __SOCKET_H

#include "SocketEndPointIPv6.h"

class Socket {
    
public:
    Socket(bool isUDP = false);
    ~Socket();
    
    bool Bind(SocketEndPoint* ep);
    bool Listen();
    Socket* Accept();
    
    long Receive(unsigned char* buffer, long size);
    long Send(const char* buffer, long size);
    long SendTo(SocketEndPoint* dst, const char* buffer, long size);
    
    void Close();
    
    SocketEndPoint* GetLocalEndPoint();
    SocketEndPoint* GetRemoteEndPoint();
    bool IsUDP();
    
private:
    
    Socket(int socket);
    
    bool _isUDP;
    int _socket;
    SocketEndPoint* _localEndPoint;
    SocketEndPoint* _remoteEndPoint;

};

#endif
