//
//  settings_mac.c
//  AirFloat
//
//  Created by Kristian Trenskow on 2/24/13.
//
//

#include <stdlib.h>
#include <string.h>

#include "settings.h"

struct settings_t {
    char* name;
    char* password;
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
    
    s->name = (char*)malloc(strlen(s_name + 1));
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
