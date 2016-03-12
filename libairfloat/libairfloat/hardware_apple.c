//
//  hardware_apple.c
//  AirFloat
//
//  Copyright (c) 2013, Kristian Trenskow All rights reserved.
//
//  Redistribution and use in source and binary forms, with or
//  without modification, are permitted provided that the following
//  conditions are met:
//
//  Redistributions of source code must retain the above copyright
//  notice, this list of conditions and the following disclaimer.
//  Redistributions in binary form must reproduce the above
//  copyright notice, this list of conditions and the following
//  disclaimer in the documentation and/or other materials provided
//  with the distribution. THIS SOFTWARE IS PROVIDED BY THE
//  COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
//  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
//  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
//  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
//  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
//  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#ifdef __APPLE__

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <sys/socket.h>
#include <net/if_dl.h>
#include <ifaddrs.h>

#include <mach/mach_time.h>

#include "DeviceIDRetriver.h"

bool _hardware_time_initiated = false;
uint32_t _hardware_time_to_nanos_numerator;
uint32_t _hardware_time_to_nanos_denominator;

double hardware_host_time_to_seconds(double host_time) {
    
    if (!_hardware_time_initiated) {
        
        struct mach_timebase_info time_base_info;
        mach_timebase_info(&time_base_info);
        _hardware_time_to_nanos_numerator = time_base_info.numer;
        _hardware_time_to_nanos_denominator = time_base_info.denom;
        
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
    
    //return ret;
    
//}
    
    return iOSDeviceID();
  }
    
double hardware_get_time() {
    
    return hardware_host_time_to_seconds(mach_absolute_time());
    
}

#endif