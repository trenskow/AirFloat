//
//  Settings.cpp
//  AirFloat
//
//  Created by Kristian Trenskow on 3/22/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#include <CoreFoundation/CoreFoundation.h>
#include "Settings.h"

bool Settings::isAuthenticationEnabled() {
    
    return _boolValueForKey("kAirFloatUserDefaultsAuthenticationEnabledKey", false);
    
}

const char* Settings::getPassword() {
    
    CFStringRef password = (CFStringRef) CFPreferencesCopyAppValue(CFSTR("kAirFloatUserDefaultsPasswordKey"), kCFPreferencesCurrentApplication);
    
    if (password != NULL) {
        
        const char* ret = NULL;
        
        if (CFStringGetLength(password) > 0)
            ret = CFStringGetCStringPtr(password, kCFStringEncodingUTF8);

        CFRelease(password);
        
        return ret;
        
    }
    
    return NULL;
    
}

bool Settings::isSyncronizationEnabled() {
    
    return _boolValueForKey("kAirFloatUserDefaultsSynchronizeAudioKey", false);
    
}

bool Settings::_boolValueForKey(const char* key, bool defaultValue) {
    
    CFStringRef keyString = CFStringCreateWithCString(NULL, key, kCFStringEncodingUTF8);
    
    Boolean isSet = false;    
    Boolean ret = CFPreferencesGetAppBooleanValue(keyString, kCFPreferencesCurrentApplication, &isSet);

    CFRelease(keyString);
    
    if (isSet)
        return ret;
    
    return defaultValue;
    
}