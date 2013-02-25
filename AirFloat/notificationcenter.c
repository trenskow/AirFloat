//
//  notificationcenter.cpp
//  AirFloat
//
//  Created by Kristian Trenskow on 2/8/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#include <string.h>
#include <stdbool.h>

#include "log.h"
#include "mutex.h"
#include "notificationcenter.h"

struct notification_t {
    const char* name;
    void* sender;
    void* notification_info;
};

struct notification_center_observer {
    
    notification_observer_callback callback;
    void* ctx;
    char* name;
    void* object;
    
};

struct notification_center_observer* _observers;
uint32_t _observerCount;

mutex_p _mutex;

const char* notification_get_name(struct notification_t* n) {
    
    return n->name;
    
}

void* notification_get_sender(struct notification_t* n) {
    
    return n->sender;
    
}

void* notification_get_info(struct notification_t* n) {
    
    return n->notification_info;
    
}

mutex_p _notification_mutex() {
    
    if (_mutex == NULL)
        _mutex = mutex_create();
    
    return _mutex;
    
}

void notification_center_add_observer(notification_observer_callback notification_observer, void* ctx, const char* name, void* object) {
    
    mutex_lock(_notification_mutex());
    
    struct notification_center_observer new_observer;
    new_observer.callback = notification_observer;
    new_observer.ctx = ctx;
    if (name != NULL) {
        new_observer.name = (char*)malloc(strlen(name) + 1);
        strcpy(new_observer.name, name);
    } else
        new_observer.name = NULL;
    new_observer.object = object;
    
    _observers = (struct notification_center_observer*)realloc(_observers, sizeof(struct notification_center_observer) * (_observerCount + 1));
    _observers[_observerCount] = new_observer;
    _observerCount++;
    
    mutex_unlock(_mutex);
    
}

void notification_center_remove_observer(notification_observer_callback notificaiton_observer, void* ctx) {
    
    mutex_lock(_mutex);
    
    for (uint32_t i = 0 ; i < _observerCount ; i++)
        if (_observers[i].callback == notificaiton_observer || _observers[i].ctx == ctx) {
            if (_observers[i].name != NULL)
                free(_observers[i].name);
            
            memcpy(&_observers[i], &_observers[i + 1], sizeof(struct notification_center_observer) * (_observerCount - (i + 1)));
            i--;
            _observerCount--;
        }
    
    mutex_unlock(_mutex);
    
}

void notification_center_post_notification(const char* name, void* sender, void* notification_info) {
    
    struct notification_t notification;
    notification.name = name;
    notification.sender = sender;
    notification.notification_info = notification_info;
    
    mutex_lock(_notification_mutex());
    
    bool handled = false;
    
    for (uint32_t i = 0 ; i < _observerCount ; i++) {
        
        bool objectMatch = (_observers[i].object == sender);
        bool nameMatch = (_observers[i].name != NULL && 0 == strcmp(_observers[i].name, name));
        
        handled = (handled || objectMatch || nameMatch);
        
        if ((_observers[i].name == NULL && _observers[i].object == NULL) || (objectMatch && _observers[i].name == NULL) || (nameMatch && _observers[i].object == NULL))
            _observers[i].callback(&notification, _observers[i].ctx);
        
    }
    
    mutex_unlock(_mutex);
    
    if (!handled)
        log_message(LOG_INFO, "Notification \"%s\" had no observers.", name);
    
}

/*
#include "Notification.h"
#include "NotificationObserver.h"
#include "NotificationCenter.h"

static NotificationCenter* __defaultCenter = NULL;

NotificationCenter::NotificationCenter() {
    
    _observers = NULL;
    _observerCount = 0;
    
    _mutex = mutex_create();
    
}

NotificationCenter::~NotificationCenter() {
    
    mutex_destroy(_mutex);
    
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

void NotificationCenter::removeObserver(notificationClbk callback, void* callbackContext) {
    
}

void NotificationCenter::removeObserver(void* callbackContext) {
    
    removeObserver(NULL, callbackContext);
    
}

void NotificationCenter::postNotification(const char* name, void* sender, void* notificationInfo) {
    
    
}

void NotificationCenter::_addObserver(NotificationObserver* observer, notificationClbk callback, void* callbackContext, const char* name, void* object) {
        
}
*/
