//
//  zeroconf.h
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

#ifndef _zeroconf_h
#define _zeroconf_h

#include <stdint.h>

#include "sockaddr.h"

typedef struct zeroconf_raop_ad_t *zeroconf_raop_ad_p;

zeroconf_raop_ad_p zeroconf_raop_ad_create(uint16_t port, const char* name);
void zeroconf_raop_ad_destroy(zeroconf_raop_ad_p za);

typedef struct zeroconf_dacp_discover_t *zeroconf_dacp_discover_p;

typedef void(*zeroconf_dacp_discover_service_found_callback)(zeroconf_dacp_discover_p zeroconf_dacp_discover, const char* name, struct sockaddr** end_points, uint32_t end_point_counts, void* ctx);

zeroconf_dacp_discover_p zeroconf_dacp_discover_create();
void zeroconf_dacp_discover_destroy(zeroconf_dacp_discover_p zd);
void zeroconf_dacp_discover_set_callback(zeroconf_dacp_discover_p zd, zeroconf_dacp_discover_service_found_callback callback, void* ctx);

#endif
