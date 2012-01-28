//
//  AirFloatInterfaces.m
//  AirFloat
//
//  Created by Kristian Trenskow on 7/19/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <ifaddrs.h>
#import <sys/socket.h>
#import <netinet/in.h>
#import <string.h>
#import <arpa/inet.h>
#import <net/if_dl.h>

#import "AirFloatInterfaces.h"

NSDictionary* allInterfaces() {
    
    NSMutableDictionary* ret = [[NSMutableDictionary alloc] init];
    
    struct ifaddrs* if_addrs = NULL;
    struct ifaddrs* if_addr = NULL;
    char buf[100];
    
    if (0 == getifaddrs(&if_addrs)) {
        
        for (if_addr = if_addrs ; if_addr != NULL ; if_addr = if_addr->ifa_next) {
            
            NSString* name = [NSString stringWithCString:if_addr->ifa_name encoding:NSASCIIStringEncoding];
            
            if (![ret objectForKey:name])
                [ret setObject:[NSMutableDictionary dictionary] forKey:name];
            
            // Address (IPv4)
            if (if_addr->ifa_addr->sa_family == AF_INET) {
                struct in_addr* addr = &((struct sockaddr_in*)if_addr->ifa_addr)->sin_addr;
                if (addr && inet_ntop(AF_INET, addr, buf, sizeof(buf)))
                    [[ret objectForKey:name] setObject:[NSString stringWithCString:buf encoding:NSASCIIStringEncoding] forKey:@"address"];
            }
            
            // Address (IPv6)
            if (if_addr->ifa_addr->sa_family == AF_INET6) {
                struct in6_addr* addr = &((struct sockaddr_in6*)if_addr->ifa_addr)->sin6_addr;
                if (addr && inet_ntop(AF_INET6, addr, buf, sizeof(buf)))
                    [[ret objectForKey:name] setObject:[NSString stringWithCString:buf encoding:NSASCIIStringEncoding] forKey:@"address6"];
            }                
            
            // MAC
            if (if_addr->ifa_addr != NULL && if_addr->ifa_addr->sa_family == AF_LINK) {
                
                struct sockaddr_dl* sdl = (struct sockaddr_dl*)if_addr->ifa_addr;
                unsigned char mac[6];
                if (6 == sdl->sdl_alen) {
                    memcpy(mac, LLADDR(sdl), sdl->sdl_alen);
                    [[ret objectForKey:name] setObject:[NSString stringWithFormat:@"%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]] forKey:@"mac"];
                }
                
            }
            
        }
        
        freeifaddrs(if_addrs);
        
    }
    
    return [ret autorelease];
    
}
