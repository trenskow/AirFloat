//
//  NotificationCenter.cpp
//  AirFloat
//
//  Created by Kristian Trenskow on 2/8/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#include <string.h>
#include "Log.h"
#include "Notification.h"
#include "NotificationObserver.h"
#include "NotificationCenter.h"

static NotificationCenter* __defaultCenter = NULL;

NotificationCenter::NotificationCenter() {
    
    _observers = NULL;
    _observerCount = 0;
    
}

NotificationCenter* NotificationCenter::defaultCenter() {
    
    if (__defaultCenter == NULL)
        __defaultCenter = new NotificationCenter;
    
    return __defaultCenter;
    
}

void NotificationCenter::addObserver(NotificationObserver* observer, const char* name, void* object) {
    
    _addObserver(observer, NULL, NULL, name, object);
    
}

void NotificationCenter::addObserver(notificationClbk callback, void* callbackContext, const char* name, void* object) {
    
    _addObserver(NULL, callback, callbackContext, name, object);
    
}

void NotificationCenter::removeObserver(void* observer) {
    
    _mutex.lock();
    
    for (uint32_t i = 0 ; i < _observerCount ; i++)
        if (_observers[i].observer == observer) {
            if (_observers[i].name != NULL)
                free(_observers[i].name);
            
            memcpy(&_observers[i], &_observers[i + 1], sizeof(NotificationCenterObserver) * (_observerCount - (i + 1)));
            i--;
            _observerCount--;
        }

    _mutex.unlock();
    
}

void NotificationCenter::postNotification(const char* name, void* sender, void* notificationInfo) {
    
    Notification* notification = new Notification(sender, name, notificationInfo);
    
    _mutex.lock();
    
    bool handled = false;
    
    for (uint32_t i = 0 ; i < _observerCount ; i++) {
        
        bool objectMatch = (_observers[i].object == sender);
        bool nameMatch = (_observers[i].name != NULL && 0 == strcmp(_observers[i].name, name));
        
        handled = (handled || objectMatch || nameMatch);
        
        if ((_observers[i].name == NULL && _observers[i].object == NULL) || (objectMatch && _observers[i].name == NULL) || (nameMatch && _observers[i].object == NULL)) {
            if (_observers[i].observer != NULL)
                _observers[i].observer->_notificationReceived(notification);
            else if (_observers[i].callback != NULL)
                _observers[i].callback(sender, name, notificationInfo, _observers[i].callbackContext);
        }
        
    }
    
    _mutex.unlock();
    
    delete notification;
    
    if (!handled)
        log(LOG_INFO, "Notification \"%s\" had no observers.", name);
    
}

void NotificationCenter::_addObserver(NotificationObserver* observer, notificationClbk callback, void* callbackContext, const char* name, void* object) {
    
    _mutex.lock();
    
    NotificationCenterObserver newObserver;
    newObserver.observer = observer;
    newObserver.callback = callback;
    newObserver.callbackContext = callbackContext;
    if (name != NULL) {
        newObserver.name = (char*)malloc(strlen(name) + 1);
        strcpy(newObserver.name, name);
    } else
        newObserver.name = NULL;
    newObserver.object = object;
        
    _observers = (NotificationCenterObserver*)realloc(_observers, sizeof(NotificationCenterObserver) * (_observerCount + 1));
    _observers[_observerCount] = newObserver;
    _observerCount++;
    
    _mutex.unlock();
    
}
