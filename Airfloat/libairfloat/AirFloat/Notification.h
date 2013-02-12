//
//  Notification.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/8/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#ifndef AirFloat_Notification_h
#define AirFloat_Notification_h

class NotificationCenter;

class Notification {
    
    friend class NotificationCenter;
    
public:
    
    void* getSender();
    const char* getName();
    void* getNotificationInfo();
    
    bool operator==(const char* name);
    
private:
    
    Notification(void* sender, const char* name, void* notificationInfo);
    
    void* _sender;
    const char* _name;
    void* _notificationInfo;
    
};

#endif
