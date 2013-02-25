//
//  notificationcenter.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/8/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#ifndef _notificationcenter_h
#define _notificationcenter_h

#include <stdlib.h>
#include <stdint.h>

typedef struct notification_t *notification_p;

typedef void(*notification_observer_callback)(notification_p notification, void* ctx);

const char* notification_get_name(notification_p n);
void* notification_get_sender(notification_p n);
void* notification_get_info(notification_p n);

void notification_center_add_observer(notification_observer_callback notification_observer, void* ctx, const char* name, void* object);
void notification_center_remove_observer(notification_observer_callback notificaiton_observer, void* ctx);
void notification_center_post_notification(const char* name, void* sender, void* notification_info);

#endif
