//
//  Notification.cpp
//  AirFloat
//
//  Created by Kristian Trenskow on 2/8/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#include <string.h>
#include "Notification.h"

Notification::Notification(void* sender, const char* name, void* context) {
    
    _sender = sender;
    _name = name;
    _notificationInfo = context;    
    
}

void* Notification::getSender() {
    
    return _sender;
    
}

const char* Notification::getName() {
    
    return _name;
    
}

void* Notification::getNotificationInfo() {
    
    return _notificationInfo;
    
}

bool Notification::operator==(const char* name) {
    
    return (0 == strcmp(name, _name));
    
}