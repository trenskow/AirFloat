//
//  settings_mac.c
//  AirFloat
//
//  Created by Kristian Trenskow on 2/24/13.
//
//

#ifdef __APPLE__
#include <TargetConditionals.h>

#if (TARGET_OS_IPHONE == 1 || TARGET_OS_MAC == 1)

#include <CoreFoundation/CoreFoundation.h>

const char* settings_get_password() {
    
    const char* ret = NULL;
    
    CFStringRef cfPassword = (CFStringRef) CFPreferencesCopyAppValue(CFSTR("password"), kCFPreferencesCurrentApplication);
    
    if (cfPassword != NULL) {
        
        if (CFStringGetLength(cfPassword) > 0)
            ret = CFStringGetCStringPtr(cfPassword, kCFStringEncodingUTF8);
        
        CFRelease(cfPassword);
        
    }
    
    return ret;
    
}

#endif
#endif
