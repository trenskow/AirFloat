//
//  settings.h
//  AirFloat
//
//  Created by Kristian Trenskow on 3/22/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#ifndef _settings_h
#define _settings_h

typedef struct settings_t *settings_p;

typedef void(*settings_name_changed_callback)(settings_p settings, const char* new_name, void* ctx);
typedef void(*settings_password_changed_callback)(settings_p settings, const char* new_password, void* ctx);

settings_p settings_create(const char* name, const char* password);
void settings_destroy(settings_p s);
const char* settings_get_name(settings_p s);
void settings_set_name(settings_p s, const char* new_name);
void settings_set_password(settings_p s, const char* new_password);
const char* settings_get_password(settings_p s);

#endif
