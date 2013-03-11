//
//  hardware.c
//  AirFloat
//
//  Created by Kristian Trenskow on 3/5/13.
//
//

#ifdef __APPLE__

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <sys/socket.h>
#include <net/if_dl.h>
#include <ifaddrs.h>

#include <mach/mach_time.h>

bool _hardware_time_initiated = false;
uint32_t _hardware_time_to_nanos_numerator;
uint32_t _hardware_time_to_nanos_denominator;

double hardware_host_time_to_seconds(double host_time) {
    
    if (!_hardware_time_initiated) {
        
        struct mach_timebase_info theTimeBaseInfo;
        mach_timebase_info(&theTimeBaseInfo);
        _hardware_time_to_nanos_numerator = theTimeBaseInfo.numer;
        _hardware_time_to_nanos_denominator = theTimeBaseInfo.denom;
        
    }
    
    return host_time / (double)_hardware_time_to_nanos_denominator * (double)_hardware_time_to_nanos_numerator / 1000000000.0;
    
}

uint64_t hardware_identifier() {
    
    uint64_t ret = 0;
    
    struct ifaddrs* if_addrs = NULL;
    struct ifaddrs* if_addr = NULL;
    
    if (0 == getifaddrs(&if_addrs))
        for (if_addr = if_addrs ; if_addr != NULL ; if_addr = if_addr->ifa_next) {
            
            if (if_addr->ifa_name != NULL && if_addr->ifa_addr->sa_family == AF_LINK && strcmp("en0", if_addr->ifa_name) == 0) {
                
                struct sockaddr_dl* sdl = (struct sockaddr_dl*)if_addr->ifa_addr;
                if (sdl->sdl_alen == 6) {
                    memcpy(&((char*)&ret)[2], LLADDR(sdl), sdl->sdl_alen);
                    break;
                }
                
            }
            
        }
    
    freeifaddrs(if_addrs);
    
    return ret;
    
}

double hardware_get_time() {
    
    return hardware_host_time_to_seconds(mach_absolute_time());
    
}

#endif