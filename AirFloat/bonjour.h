//
//  bonjour.h
//  AirFloat
//
//  Created by Kristian Trenskow on 3/10/13.
//
//

#ifndef _bonjour_h
#define _bonjour_h

#include <stdint.h>

typedef struct bonjour_ad_t *bonjour_ad_p;

bonjour_ad_p bonjour_ad_create(uint16_t port);
void bonjour_ad_destroy(bonjour_ad_p b);

#endif
