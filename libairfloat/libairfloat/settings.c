//
//  settings_mac.c
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
#include <stdbool.h>

#include "settings.h"

struct settings_t {
    char* name;
    char* password;
    bool ignore_source_volume;
};

struct settings_t* settings_create(const char* name, const char* password) {
    
    struct settings_t* s = (struct settings_t*)malloc(sizeof(struct settings_t));
    bzero(s, sizeof(struct settings_t));
    
    settings_set_name(s, name);
    settings_set_password(s, password);
    
    return s;
    
}

void settings_destroy(struct settings_t* s) {
    
    free(s->name);
    if (s->password != NULL)
        free(s->password);
    
    return free(s);
    
}

const char* settings_get_name(struct settings_t* s) {
    
    return s->name;
    
}

void settings_set_name(struct settings_t* s, const char* new_name) {
    
    if (s->name != NULL)
        free(s->name);
    
    const char* s_name = new_name;
    if (new_name == NULL || strlen(new_name) == 0)
        s_name = "AirFloat";
    
    s->name = (char*)malloc(strlen(s_name) + 1);
    strcpy(s->name, s_name);

}

const char* settings_get_password(struct settings_t* s) {
    
    return s->password;
    
}

void settings_set_password(struct settings_t* s, const char* new_password) {
    
    if (s->password != NULL) {
        free(s->password);
        s->password = NULL;
    }
    
    if (new_password != NULL) {
        s->password = (char*)malloc(strlen(new_password) + 1);
        strcpy(s->password, new_password);
    }
    
}

bool settings_get_ignore_source_volume(struct settings_t* s) {
    
    return s->ignore_source_volume;
    
}

void settings_set_ignore_source_volume(struct settings_t*s, bool ignore_source_volume) {
    
    s->ignore_source_volume = ignore_source_volume;
    
}
