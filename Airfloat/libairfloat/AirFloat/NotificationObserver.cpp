//
//  NotificationReceiver.cpp
//  AirFloat
//
//  Created by Kristian Trenskow on 2/8/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#include "NotificationCenter.h"
#include "NotificationObserver.h"

NotificationObserver::~NotificationObserver() {
    
    NotificationCenter::defaultCenter()->removeObserver(this);
    
}
