//
//  zeroconf.h
//  AirFloat
//
//  Created by Kristian Trenskow on 3/10/13.
//
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
