//
//  Socket.h
//  AirFloatCF
//
//  Created by Kristian Trenskow on 5/10/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#ifndef __SOCKET_H
#define __SOCKET_H

#include "SocketEndPoint.h"

class Socket {
    
public:
    Socket(bool isUDP);
    ~Socket();
    
    virtual bool Bind(SocketEndPoint* ep);
    virtual bool Listen();
    virtual Socket* Accept();
    
    virtual long Receive(unsigned char* buffer, long size);
    virtual long Send(const char* buffer, long size);
    virtual long SendTo(SocketEndPoint* dst, const char* buffer, long size);
    
    virtual void Close();
    
    virtual SocketEndPoint* LocalEndPoint();
    virtual SocketEndPoint* RemoteEndPoint();
    
private:
    
    Socket(int socket, struct sockaddr* addr);
    
    bool _isUDP;
    int _socket;
    SocketEndPoint _localEndPoint;
    SocketEndPoint _remoteEndPoint;
    
};

#endif
