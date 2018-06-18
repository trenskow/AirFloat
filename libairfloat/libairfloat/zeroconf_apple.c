//
//  zeroconf_apple.c
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

#if defined(__APPLE__)

#include <stdlib.h>
#include <stdint.h>

#include <CoreFoundation/CoreFoundation.h>
#include <CFNetwork/CFNetwork.h>

#include "log.h"
#include "hardware.h"
#include "settings.h"
#include "mutex.h"
#include "condition.h"
#include "thread.h"

#include "zeroconf.h"

struct zeroconf_raop_ad_t {
    object_p object;
    CFNetServiceRef service;
    CFRunLoopRef run_loop;
    thread_p thread;
    mutex_p mutex;
    condition_p condition;
    uint16_t port;
};

void zeroconf_raop_ad_destroy(struct zeroconf_raop_ad_t* za);

void _zeroconf_raop_ad_callback(CFNetServiceRef theService, CFStreamError* error, void* info) {
    
    struct zeroconf_raop_ad_t* b = (struct zeroconf_raop_ad_t*)info;
    
    if (error->error == 0)
        log_message(LOG_INFO, "Zeroconf advertising started on port %d", b->port);
    else
        log_message(LOG_ERROR, "Could not start Zeroconf advertisement.");
    
}

void _zeroconf_raop_ad_run_loop_ready(CFRunLoopTimerRef timer, void *info) {
    
    struct zeroconf_raop_ad_t* b = (struct zeroconf_raop_ad_t*)info;
    
    log_message(LOG_INFO, "Run loop ready");
    condition_signal(b->condition);
    
}

void _zeroconf_raop_ad_run_loop_thread(void* ctx) {
    
    thread_set_name("RAOP Zeroconf advertising run loop");
    
    struct zeroconf_raop_ad_t* za = (struct zeroconf_raop_ad_t*)ctx;
    
    CFNetServiceClientContext context = { 0, za, NULL, NULL, NULL };
    
    CFNetServiceSetClient(za->service, _zeroconf_raop_ad_callback, &context);    
    CFNetServiceScheduleWithRunLoop(za->service, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
    CFNetServiceRegisterWithOptions(za->service, kCFNetServiceFlagNoAutoRename, NULL);
    
    za->run_loop = CFRunLoopGetCurrent();
    
    CFRunLoopTimerContext timer_context = { 0, za, NULL, NULL, NULL };
    CFRunLoopTimerRef timer = CFRunLoopTimerCreate(kCFAllocatorDefault, CFAbsoluteTimeGetCurrent() + .2, 0, 0, 0, _zeroconf_raop_ad_run_loop_ready, &timer_context);
    CFRunLoopAddTimer(za->run_loop, timer, kCFRunLoopCommonModes);
    CFRelease(timer);
    
    CFRunLoopRun();
    
    CFNetServiceUnscheduleFromRunLoop(za->service, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
    CFNetServiceSetClient(za->service, NULL, NULL);
    CFNetServiceCancel(za->service);
    
}

void _zeroconf_raop_ad_destroy(void* object) {
    
    struct zeroconf_raop_ad_t* za = (struct zeroconf_raop_ad_t*)object;
    
    CFRunLoopStop(za->run_loop);
    
    condition_destroy(za->condition);
    mutex_destroy(za->mutex);
    thread_join(za->thread);
    
    CFRelease(za->service);
    
    thread_destroy(za->thread);
    
}

struct zeroconf_raop_ad_t* zeroconf_raop_ad_create(uint16_t port, const char *name) {
    
    struct zeroconf_raop_ad_t* za = (struct zeroconf_raop_ad_t*)object_create(sizeof(struct zeroconf_raop_ad_t), _zeroconf_raop_ad_destroy);
        
    CFStringRef service_name = CFStringCreateWithCString(kCFAllocatorDefault, name, kCFStringEncodingASCII);
    uint64_t hardware_id = hardware_identifier();
    
    uint8_t* hardware_chars = (uint8_t*)&hardware_id;
    
    CFStringRef hardware_identifier = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%02X%02X%02X%02X%02X%02X"), hardware_chars[2], hardware_chars[3], hardware_chars[4], hardware_chars[5], hardware_chars[6], hardware_chars[7]);
    
    CFStringRef combined_name = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%@@%@"), hardware_identifier, service_name);
    
    za->service = CFNetServiceCreate(kCFAllocatorDefault, CFSTR(""), CFSTR("_raop._tcp"), combined_name, port);
    za->port = port;
    
    CFStringRef keys[16] = { CFSTR("txtvers"), CFSTR("et"), CFSTR("ek"), CFSTR("ss"), CFSTR("sr"), CFSTR("tp"), CFSTR("cn"), CFSTR("da"), CFSTR("sf"), CFSTR("vn"), CFSTR("md"), CFSTR("vs"), CFSTR("sv"), CFSTR("sm"), CFSTR("ch"), CFSTR("sr") };
    CFStringRef values[16] = { CFSTR("1"), CFSTR("0,1"), CFSTR("1"), CFSTR("16"), CFSTR("44100"), CFSTR("TCP,UDP"), CFSTR("1"), CFSTR("true"), CFSTR("0x4"), CFSTR("65537"), CFSTR("0,1,2"), CFSTR("104.29"), CFSTR("false"), CFSTR("false"), CFSTR("2"), CFSTR("44100") };
    
    CFDictionaryRef txt_dictionary = CFDictionaryCreate(kCFAllocatorDefault, (const void**)&keys, (const void**)&values, 16, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFDataRef txt_data = CFNetServiceCreateTXTDataWithDictionary(kCFAllocatorDefault, txt_dictionary);

    CFNetServiceSetTXTData(za->service, txt_data);
    
    za->mutex = mutex_create();
    za->condition = condition_create();
    
    mutex_lock(za->mutex);
    za->thread = thread_create_a(_zeroconf_raop_ad_run_loop_thread, za);
    condition_wait(za->condition, za->mutex);
    mutex_unlock(za->mutex);
    
    log_message(LOG_INFO, "Zeroconf configured");
    
    CFRelease(txt_data);
    CFRelease(txt_dictionary);
    CFRelease(combined_name);
    CFRelease(hardware_identifier);
    CFRelease(service_name);
    
    return za;
    
}

struct zeroconf_dacp_discover_t {
    object_p object;
    CFNetServiceBrowserRef domain_browser;
    CFNetServiceBrowserRef* service_browsers;
    uint32_t service_browsers_count;
    CFRunLoopRef run_loop;
    thread_p thread;
    mutex_p mutex;
    zeroconf_dacp_discover_service_found_callback service_found_callback;
    void* service_found_callback_ctx;
};

void _zeroconf_dacp_discover_resolve_callback(CFNetServiceRef service, CFStreamError* error, void* info) {
    
    struct zeroconf_dacp_discover_t* zd = (struct zeroconf_dacp_discover_t*)info;
    
    CFArrayRef addresses = CFNetServiceGetAddressing(service);
    
    if (addresses != NULL && zd->service_found_callback != NULL) {
        
        uint32_t addresses_count = (uint32_t)CFArrayGetCount(addresses);
        endpoint_p endpoints[addresses_count];
        
        for (uint32_t i = 0 ; i < addresses_count ; i++) {
            CFDataRef sockaddr_data = (CFDataRef)CFArrayGetValueAtIndex(addresses, i);
            struct sockaddr* addr = (struct sockaddr*)CFDataGetBytePtr(sockaddr_data);
            endpoints[i] = endpoint_create_sockaddr(addr);
        }
        
        zd->service_found_callback(zd, CFStringGetCStringPtr(CFNetServiceGetName(service), kCFStringEncodingMacRoman), endpoints, addresses_count, zd->service_found_callback_ctx);
        
        for (uint32_t i = 0 ; i < addresses_count ; i++) {
            object_release(endpoints[i]);
        }
        
    }
    
    log_message(LOG_INFO, "Found DACP Service: %s", CFStringGetCStringPtr(CFNetServiceGetName(service), kCFStringEncodingMacRoman));
    
}

void _zeroconf_dacp_discover_browse_callback(CFNetServiceBrowserRef browser, CFOptionFlags flags, CFTypeRef domainOrService, CFStreamError* error, void* info) {
    
    struct zeroconf_dacp_discover_t* zd = (struct zeroconf_dacp_discover_t*)info;
    
    if (browser == zd->domain_browser) {
        
        mutex_lock(zd->mutex);
        
        zd->service_browsers = (CFNetServiceBrowserRef*)realloc(zd->service_browsers, sizeof(CFNetServiceBrowserRef) * (zd->service_browsers_count + 1));
        CFNetServiceBrowserRef* service_browser = &zd->service_browsers[zd->service_browsers_count++];
        
        CFNetServiceClientContext context = { 0, zd, NULL, NULL, NULL };
        
        *service_browser = CFNetServiceBrowserCreate(kCFAllocatorDefault, _zeroconf_dacp_discover_browse_callback, &context);
        
        CFNetServiceBrowserScheduleWithRunLoop(*service_browser, zd->run_loop, kCFRunLoopCommonModes);
        CFNetServiceBrowserSearchForServices(*service_browser, domainOrService, CFSTR("_dacp._tcp."), NULL);
        
        mutex_unlock(zd->mutex);
        
        log_message(LOG_INFO, "Domain found: %s", CFStringGetCStringPtr(domainOrService, kCFStringEncodingMacRoman));
        
    } else if (CFNetServiceGetTypeID() == CFGetTypeID(domainOrService)) {
        
        CFNetServiceRef service = (CFNetServiceRef)domainOrService;
        
        CFArrayRef addresses = CFNetServiceGetAddressing(service);
        
        if (addresses == NULL) {
            
            CFNetServiceClientContext content = { 0, zd, NULL, NULL, NULL };
            
            CFNetServiceSetClient(service, _zeroconf_dacp_discover_resolve_callback, &content);
            CFNetServiceScheduleWithRunLoop(service, zd->run_loop, kCFRunLoopCommonModes);
            CFNetServiceResolveWithTimeout(service, 30.0, NULL);
            
        } else
            _zeroconf_dacp_discover_resolve_callback(service, NULL, zd);
        
    }
    
}

void _zeroconf_dacp_discover_run_loop_thread(void* ctx) {
    
    thread_set_name("DACP Zeroconf discover run loop");
    
    struct zeroconf_dacp_discover_t* zd = (struct zeroconf_dacp_discover_t*)ctx;
    
    zd->run_loop = CFRunLoopGetCurrent();
    
    CFNetServiceBrowserScheduleWithRunLoop(zd->domain_browser, zd->run_loop, kCFRunLoopCommonModes);
    CFNetServiceBrowserSearchForDomains(zd->domain_browser, FALSE, NULL);
    
    CFRunLoopRun();
    
    CFNetServiceBrowserUnscheduleFromRunLoop(zd->domain_browser, zd->run_loop, kCFRunLoopCommonModes);
    CFNetServiceBrowserInvalidate(zd->domain_browser);
    
    zd->run_loop = NULL;
    
}

void _zeroconf_dacp_discover_destroy(void* object) {
    
    struct zeroconf_dacp_discover_t* zd = (struct zeroconf_dacp_discover_t*)object;
    
    mutex_lock(zd->mutex);
    
    for (uint32_t i = 0 ; i < zd->service_browsers_count ; i++) {
        CFNetServiceBrowserUnscheduleFromRunLoop(zd->service_browsers[i], zd->run_loop, kCFRunLoopCommonModes);
        CFNetServiceBrowserInvalidate(zd->service_browsers[i]);
        CFRelease(zd->service_browsers[i]);
    }
    
    free(zd->service_browsers);
    
    zd->service_browsers_count = 0;
    
    mutex_unlock(zd->mutex);
    
    if (zd->run_loop != NULL) {
        CFRunLoopStop(zd->run_loop);
        thread_join(zd->thread);
    }
    
    thread_destroy(zd->thread);
    mutex_destroy(zd->mutex);
    
    CFRelease(zd->domain_browser);
    
}

struct zeroconf_dacp_discover_t* zeroconf_dacp_discover_create() {
    
    struct zeroconf_dacp_discover_t* zd = (struct zeroconf_dacp_discover_t*)object_create(sizeof(struct zeroconf_dacp_discover_t), _zeroconf_dacp_discover_destroy);
    
    CFNetServiceClientContext context = { 0, zd, NULL, NULL, NULL };
    
    zd->domain_browser = CFNetServiceBrowserCreate(kCFAllocatorDefault, _zeroconf_dacp_discover_browse_callback, &context);
    zd->mutex = mutex_create();
    zd->thread = thread_create_a(_zeroconf_dacp_discover_run_loop_thread, zd);
    
    return zd;
    
}

void zeroconf_dacp_discover_set_callback(struct zeroconf_dacp_discover_t* zd, zeroconf_dacp_discover_service_found_callback callback, void* ctx) {
    
    zd->service_found_callback = callback;
    zd->service_found_callback_ctx = ctx;
    
}

#endif
