//
//  NotificationReceiver.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/8/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#ifndef AirFloat_NotificationObserver_h
#define AirFloat_NotificationObserver_h

class Notification;
class NotificationCenter;

class NotificationObserver {
    
    friend class NotificationCenter;
    
protected:
    
    virtual void _notificationReceived(Notification* notification) { };
    
    virtual ~NotificationObserver();
    
};

#endif
