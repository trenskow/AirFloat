//
//  sockaddr.c
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

#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "endpoint.h"

struct endpoint_t {
    object_p object;
    struct sockaddr_storage storage;
};

struct endpoint_t* endpoint_create(const char* host, uint16_t port, endpoint_type type, uint32_t scope_id) {
    
    if (!type) type = endpoint_type_inet_4;
    
    struct endpoint_t* ret = (struct endpoint_t*)object_create(sizeof(struct endpoint_t), NULL);
    
    if (type == endpoint_type_inet_4) {
  
        struct sockaddr_in* addr = ((struct sockaddr_in*)&ret->storage);
        
        addr->sin_len = sizeof(struct sockaddr_in);
        addr->sin_family = AF_INET;
        addr->sin_addr.s_addr = INADDR_ANY;
        addr->sin_port = htons(port);
        
        if (host != NULL) {
            inet_pton(AF_INET, host, &addr->sin_addr);
        }
        
    } else if (type == endpoint_type_inet_6) {
        
        struct sockaddr_in6* addr = ((struct sockaddr_in6*)&ret->storage);
        
        addr->sin6_len = sizeof(struct sockaddr_in6);
        addr->sin6_family = AF_INET6;
        addr->sin6_addr = in6addr_any;
        addr->sin6_port = htons(port);
        addr->sin6_scope_id = scope_id;
        
        if (host != NULL) {
            inet_pton(AF_INET6, host, &addr->sin6_addr);
        }
        
    }
    
    return ret;
    
}

struct endpoint_t* endpoint_create_sockaddr(struct sockaddr* addr) {
    
    struct endpoint_t* ret = (struct endpoint_t*)object_create(sizeof(struct endpoint_t), NULL);
    
    memcpy(&ret->storage, addr, addr->sa_len);
    
    return ret;
    
}

struct endpoint_t* endpoint_copy(struct endpoint_t* endpoint) {
    
    struct endpoint_t* ret = (struct endpoint_t*)object_create(sizeof(struct endpoint_t), NULL);
    
    memcpy(&ret->storage, &endpoint->storage, sizeof(struct sockaddr_storage));
    
    return ret;
    
}

bool endpoint_equals(struct endpoint_t* endpoint1, struct endpoint_t* endpoint2) {
    
    return (endpoint1->storage.ss_len == endpoint2->storage.ss_len && 0 == memcmp(&endpoint1->storage, &endpoint2->storage, endpoint1->storage.ss_len));
    
}

bool endpoint_equals_host(struct endpoint_t* endpoint1, struct endpoint_t* endpoint2) {
    
    if (endpoint1->storage.ss_family == endpoint2->storage.ss_family) {
        if (endpoint1->storage.ss_family == AF_INET) {
            return (memcmp(&((struct sockaddr_in*)&endpoint1->storage)->sin_addr, &((struct sockaddr_in*)&endpoint2->storage)->sin_addr, sizeof(struct in_addr)) == 0);
        }
        if (endpoint1->storage.ss_family == AF_INET6) {
            return (memcmp(&((struct sockaddr_in6*)&endpoint1->storage)->sin6_addr, &((struct sockaddr_in6*)&endpoint2->storage)->sin6_addr, sizeof(struct in6_addr)) == 0);
        }
    }
    
    return false;
    
}

const char* endpoint_get_host(struct endpoint_t* endpoint) {
    
    static char ret[1024];
    
    ret[0] = '\0';
    
    if (endpoint->storage.ss_family == AF_INET)
        inet_ntop(AF_INET, &((struct sockaddr_in*)&endpoint->storage)->sin_addr, ret, 1024);
    else if (endpoint->storage.ss_family == AF_INET6)
        inet_ntop(AF_INET6, &((struct sockaddr_in6*)&endpoint->storage)->sin6_addr, ret, 1024);
    else
        return NULL;
    
    return ret;
    
}

uint16_t endpoint_get_port(struct endpoint_t* endpoint) {
    
    if (endpoint->storage.ss_family == AF_INET)
        return ntohs(((struct sockaddr_in*)&endpoint->storage)->sin_port);
    else if (endpoint->storage.ss_family == AF_INET6)
        return ntohs(((struct sockaddr_in6*)&endpoint->storage)->sin6_port);
    
    return 0;
    
}

void endpoint_set_port(struct endpoint_t* endpoint, uint16_t new_port) {
    
    if (endpoint->storage.ss_family == AF_INET) {
        ((struct sockaddr_in*)&endpoint->storage)->sin_port = htons(new_port);
    }
    
    else if (endpoint->storage.ss_family == AF_INET6) {
        ((struct sockaddr_in6*)&endpoint->storage)->sin6_port = htons(new_port);
    }
    
}

bool endpoint_is_ipv6(struct endpoint_t* endpoint) {
    return (endpoint->storage.ss_family == AF_INET6);
}

void endpoint_copy_sockaddr(struct endpoint_t* endpoint, struct sockaddr* addr) {
    memcpy(addr, &endpoint->storage, endpoint->storage.ss_len);
}

const struct sockaddr* endpoint_get_sockaddr(struct endpoint_t* endpoint) {
    return (struct sockaddr *)&endpoint->storage;
}
