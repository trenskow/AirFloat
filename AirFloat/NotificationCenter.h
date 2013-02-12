//
//  NotificationCenter.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/8/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#ifndef AirFloat_NotificationCenter_h
#define AirFloat_NotificationCenter_h

#include <stdlib.h>
#include <stdint.h>

#include "Mutex.h"
#include "Notification.h"
#include "NotificationObserver.h"

typedef void(*notificationClbk)(void* sender, const char* name, void* notificationInfo, void* callbackContext);

typedef struct {
       
    NotificationObserver* observer;
    notificationClbk callback;
    void* callbackContext;
    char* name;
    void* object;
    
} NotificationCenterObserver;

class NotificationCenter {
    
public:
    static NotificationCenter* defaultCenter();
    
    void addObserver(NotificationObserver* observer, const char* name, void* object);
    void addObserver(notificationClbk callback, void* callbackContext, const char* name, void* object);
    void removeObserver(notificationClbk callback, void* callbackContext);
    void removeObserver(void* callbackContext);
    void postNotification(const char* name, void* sender, void* notificationInfo);
    
private:
    
    NotificationCenter();
    
    void _addObserver(NotificationObserver* observer, notificationClbk callback, void* context, const char* name, void* object);
    
    Mutex _mutex;
    
    NotificationCenterObserver* _observers;
    uint32_t _observerCount;
    
};

#endif
