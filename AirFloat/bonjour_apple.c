//
//  bonjour_apple.c
//  AirFloat
//
//  Created by Kristian Trenskow on 3/10/13.
//
//

#if defined(__APPLE__)

#include <stdlib.h>
#include <stdint.h>

#include <CFNetwork/CFNetwork.h>

#include "log.h"
#include "hardware.h"
#include "settings.h"

struct bonjour_ad_t {
    CFNetServiceRef service;
    uint16_t port;
};

void bonjour_ad_destroy(struct bonjour_ad_t* b);

void _bonjour_ad_callback(CFNetServiceRef theService, CFStreamError* error, void* info) {
    
    struct bonjour_ad_t* b = (struct bonjour_ad_t*)info;
    
    if (error->error == 0)
        log_message(LOG_INFO, "Bonjour advertising started on port %d", b->port);
    else
        log_message(LOG_ERROR, "Could not start bonjour advertisement.");
    
}

struct bonjour_ad_t* bonjour_ad_create(uint16_t port) {
    
    struct bonjour_ad_t* b = (struct bonjour_ad_t*)malloc(sizeof(struct bonjour_ad_t));
    
    CFStringRef service_name = CFStringCreateWithCString(kCFAllocatorDefault, settings_get_name(), kCFStringEncodingASCII);
    uint64_t hardware_id = hardware_identifier();
    
    uint8_t* hardware_chars = (uint8_t*)&hardware_id;
    
    CFStringRef hardware_identifier = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%02X%02X%02X%02X%02X%02X"), hardware_chars[2], hardware_chars[3], hardware_chars[4], hardware_chars[5], hardware_chars[6], hardware_chars[7]);
    
    CFStringRef combined_name = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%@@%@"), hardware_identifier, service_name);
    
    b->service = CFNetServiceCreate(kCFAllocatorDefault, CFSTR(""), CFSTR("_raop._tcp"), combined_name, port);
    b->port = port;
    
    CFStringRef keys[16] = { CFSTR("txtvers"), CFSTR("et"), CFSTR("ek"), CFSTR("ss"), CFSTR("sr"), CFSTR("tp"), CFSTR("cn"), CFSTR("da"), CFSTR("sf"), CFSTR("vn"), CFSTR("md"), CFSTR("vs"), CFSTR("sv"), CFSTR("sm"), CFSTR("ch"), CFSTR("sr") };
    CFStringRef values[16] = { CFSTR("1"), CFSTR("0,1"), CFSTR("1"), CFSTR("16"), CFSTR("44100"), CFSTR("TCP,UDP"), CFSTR("1"), CFSTR("true"), CFSTR("0x4"), CFSTR("65537"), CFSTR("0,1,2"), CFSTR("104.29"), CFSTR("false"), CFSTR("false"), CFSTR("2"), CFSTR("44100") };
    
    CFDictionaryRef txt_dictionary = CFDictionaryCreate(kCFAllocatorDefault, (const void**)&keys, (const void**)&values, 16, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFDataRef txt_data = CFNetServiceCreateTXTDataWithDictionary(kCFAllocatorDefault, txt_dictionary);

    CFNetServiceSetTXTData(b->service, txt_data);
    
    CFNetServiceClientContext context = { 0, b, NULL, NULL, NULL };
    
    CFNetServiceSetClient(b->service, _bonjour_ad_callback, &context);
    
    CFNetServiceScheduleWithRunLoop(b->service, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
    CFNetServiceRegisterWithOptions(b->service, kCFNetServiceFlagNoAutoRename, NULL);
    
    CFRelease(txt_data);
    CFRelease(txt_dictionary);
    CFRelease(combined_name);
    CFRelease(hardware_identifier);
    CFRelease(service_name);
    
    return b;
    
}

void bonjour_ad_destroy(struct bonjour_ad_t* b) {
    
    CFNetServiceSetClient(b->service, NULL, NULL);
    CFNetServiceUnscheduleFromRunLoop(b->service, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
    CFNetServiceCancel(b->service);
    
    CFRelease(b->service);
    
    free(b);
    
}

#endif
