//
//  Settings.h
//  AirFloat
//
//  Created by Kristian Trenskow on 3/22/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#ifndef AirFloat_Settings_h
#define AirFloat_Settings_h

class Settings {
    
public:
    static bool isAuthenticationEnabled();
    static const char* getPassword();
    
    static bool isSyncronizationEnabled();
    
private:
    
    static bool _boolValueForKey(const char* key, bool defaultValue);
    
};

#endif
