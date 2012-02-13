//
//  AirFloatNoticationsHub.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/9/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <Foundation/Foundation.h>

#define kAirFloatSenderOriginKey @"kAirFloatSenderOriginKey"

#define AirFloatClientConnectedNotification @"AirFloatClientConnectedNotification"
#define AirFloatClientDisconnectedNotification @"AirFloatClientDisconnectedNotification"
#define AirFloatRecordingStartedNotification @"AirFloatRecordingStartedNotification"
#define AirFloatRecordingEndedNotification @"AirFloatRecordingEndedNotification"

#define AirFloatTrackInfoUpdatedNotification @"AirFloatTrackInfoUpdatedNotification"
#define kAirFloatTrackInfoTrackTitleKey @"kAirFloatTrackInfoTrackTitleKey"
#define kAirFloatTrackInfoArtistNameKey @"kAirFloatTrackInfoArtistNameKey"
#define kAirFloatTrackInfoAlbumNameKey @"kAirFloatTrackInfoAlbumNameKey"

#define AirFloatMetadataUpdatedNotification @"AirFloatMetadataUpdatedNotification"
#define kAirFloatMetadataDataKey @"kAirFloatMetadataDataKey"
#define kAirFloatMetadataContentType @"kAirFloatMetadataContentType"

@interface AirFloatNotificationsHub : NSObject

@end
