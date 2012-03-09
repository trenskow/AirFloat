//
//  SocketEndPoint.cpp
//  AirFloat
//
//  Created by Kristian Trenskow on 3/8/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#include "SocketEndPoint.h"

#include "SocketEndPointIPv4.h"
#include "SocketEndPointIPv6.h"

SocketEndPoint* SocketEndPoint::createSocket(uint16_t port, SocketEndPointType socketType) {
    
    if (socketType == kSocketEndPointTypeIPv4)
        return new SocketEndPointIPv4(port);
    else if (socketType == kSocketEndPointTypeIPv6)
        return new SocketEndPointIPv6(port);
    
    return NULL;
    
}

SocketEndPoint* SocketEndPoint::createSocket(struct sockaddr* addr) {
    
    if (addr->sa_family == AF_INET)
        return new SocketEndPointIPv4(addr);
    else if (addr->sa_family == AF_INET6)
        return new SocketEndPointIPv6(addr);
    
    return NULL;
    
}
