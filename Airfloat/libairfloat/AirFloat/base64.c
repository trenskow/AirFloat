//
//  Base64.cpp
//  AirFloatCF
//
//  Created by Kristian Trenskow on 5/10/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "base64.h"

static char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static int pos(char c) {
    char *p;
    for (p = base64_chars; *p; p++)
        if (*p == c)
            return (int)(p - base64_chars);
    return -1;
}

size_t base64_encode(const void *data, size_t size, char **str) {
    char *s, *p;
    int i;
    int c;
    const unsigned char *q;
    
    p = s = (char *) malloc(size * 4 / 3 + 4);
    if (p == NULL)
        return -1;
    q = (const unsigned char *) data;
    for (i = 0; i < size;) {
        c = q[i++];
        c *= 256;
        if (i < size)
            c += q[i];
        i++;
        c *= 256;
        if (i < size)
            c += q[i];
        i++;
        p[0] = base64_chars[(c & 0x00fc0000) >> 18];
        p[1] = base64_chars[(c & 0x0003f000) >> 12];
        p[2] = base64_chars[(c & 0x00000fc0) >> 6];
        p[3] = base64_chars[(c & 0x0000003f) >> 0];
        if (i > size)
            p[3] = '=';
        if (i > size + 1)
            p[2] = '=';
        p += 4;
    }
    *p = 0;
    *str = s;
    return (int)strlen(s);
}

#define DECODE_ERROR 0xffffffff

static unsigned int token_decode(const char *token) {
    int i;
    unsigned int val = 0;
    int marker = 0;
    if (strlen(token) < 4)
        return DECODE_ERROR;
    for (i = 0; i < 4; i++) {
        val *= 64;
        if (token[i] == '=')
            marker++;
        else if (marker > 0)
            return DECODE_ERROR;
        else
            val += pos(token[i]);
    }
    if (marker > 2)
        return DECODE_ERROR;
    return (marker << 24) | val;
}

size_t base64_decode(const char *str, void *data) {
    const char *p;
    unsigned char *q;
    
    q = (unsigned char*) data;
    for (p = str; *p && (*p == '=' || strchr(base64_chars, *p)); p += 4) {
        unsigned int val = token_decode(p);
        unsigned int marker = (val >> 24) & 0xff;
        if (val == DECODE_ERROR)
            return -1;
        *q++ = (val >> 16) & 0xff;
        if (marker < 2)
            *q++ = (val >> 8) & 0xff;
        if (marker < 1)
            *q++ = val & 0xff;
    }
    return (int)(q - (unsigned char *) data);
}

size_t base64_pad(const char* base64, size_t base64_size, char* out, size_t out_size) {
    
    assert(out_size >= base64_size + 3);
    
    memcpy(out, base64, base64_size);
    for (size_t i = 0 ; i < (base64_size % 4) ; i++)
        out[base64_size + i] = '=';
    out[base64_size + (base64_size % 4)] = '\0';
    
    return base64_size + (base64_size % 4);
    
}
