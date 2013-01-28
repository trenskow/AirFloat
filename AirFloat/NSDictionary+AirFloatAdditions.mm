//
//  NSDictionary+AirFloatAdditions.m
//  AirFloat
//
//  Created by Kristian Trenskow on 2/18/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <AirFloatLibrary/Library.h>
#import "NSDictionary+AirFloatAdditions.h"

@interface NSDictionary (AirFloatPrivate)

@property (nonatomic, readonly) DMAP* _dmap;

- (id)_initWithDMAP:(DMAP*)dmap;

@end

@implementation NSDictionary (AirFloatAdditions)

#pragma mark - Public Methods

+ (NSDictionary*)dictionaryWithParametersFromURLQuery:(NSString *)query {
    
    return [[[self alloc] initWithParametersFromURLQuery:query] autorelease];
    
}

+ (NSDictionary*)dictionaryWithDmapTaggedData:(NSData *)dmap {
    
    return [[[self alloc] initWithDmapTaggedData:dmap] autorelease];
    
}

- (id)initWithParametersFromURLQuery:(NSString *)query {
    
    NSMutableDictionary* queryDictionary = [[NSMutableDictionary alloc] init];
    
    if (queryDictionary) {
        
        NSString* parse = [query stringByReplacingOccurrencesOfString:@"&amp;" withString:@"____amp;"];
        NSArray* params = [parse componentsSeparatedByString:@"&"];
        
        for (NSString* param in params) {
            NSArray* value = [param componentsSeparatedByString:@"="];
            [queryDictionary setObject:[[value objectAtIndex:1] stringByReplacingOccurrencesOfString:@"____amp;" withString:@"&amp;"]
                                forKey:[[value objectAtIndex:0] stringByReplacingOccurrencesOfString:@"____amp;" withString:@"&amp;"]];
        }
        
        NSDictionary* ret = [queryDictionary copy];
        [queryDictionary release];
        
        return ret;
        
    }
    
    return nil;
    
}

- (id)initWithDmapTaggedData:(NSData *)data {
    
    NSDictionary* ret = nil;
    
    DMAP* dmap = new DMAP([data bytes], [data length]);
    
    if (dmap != NULL) {
        
        ret = [self _initWithDMAP:dmap];        
        delete dmap;

    }
    
    return ret;
    
}

- (NSData*)dmapTaggedData {
    
    DMAP* dmap = self._dmap;
    
    uint32_t len = dmap->getLength();
    void* bytes = malloc(len);
    
    dmap->getContent(bytes, len);
    
    NSData* ret = [NSData dataWithBytes:bytes length:len];
    
    free (bytes);
    delete dmap;
    
    return ret;
    
}

#pragma mark - - Private Methods

- (DMAP*)_dmap {
    
    DMAP* dmap = new DMAP();
    
    for (NSString* key in self.allKeys) {
        
        NSUInteger tag;
        if ((tag = DMAP::tagForTagIdentifier([key cStringUsingEncoding:NSASCIIStringEncoding])) != 0) {
            
            id obj = [self objectForKey:key];
            
            switch (DMAP::typeForTag(tag)) {
                    
                case kDMAPTypeUnknown:
                    dmap->addBytes([obj bytes], [obj length], tag);
                    break;
                    
                case kDMAPTypeChar:
                    dmap->addChar([obj charValue], tag);
                    break;

                case kDMAPTypeShort:
                    dmap->addShort([obj shortValue], tag);
                    break;
                    
                case kDMAPTypeLong:
                    dmap->addLong([obj integerValue], tag);
                    break;
                    
                case kDMAPTypeLongLong:
                    dmap->addLonglong([obj longLongValue], tag);
                    break;
                    
                case kDMAPTypeDate:
                    dmap->addDate([obj timeIntervalSince1970], tag);
                    break;
                    
                case kDMAPTypeVersion: {
                    
                    const void* bytes = [obj bytes];
                    DMAPVersion version = *((DMAPVersion*)bytes);
                    dmap->addVersion(version, tag);
                    break;
                    
                }
                                        
                case kDMAPTypeString:
                    dmap->addString([obj cStringUsingEncoding:NSUTF8StringEncoding], tag);
                    break;
                    
                case kDMAPTypeContainer: {
                    
                    // This is a list
                    if ([obj isKindOfClass:[NSArray class]]) {
                        
                        DMAP* list = new DMAP();
                        
                        for (NSDictionary* dictionary in obj) {
                            
                            DMAP* dmapItem = dictionary._dmap;
                            list->addContainer(dmap, DMAP::tagForTagIdentifier("dmap.listingitem"));
                            delete dmapItem;
                            
                        }
                        
                        dmap->addContainer(list, DMAP::tagForTagIdentifier("dmap.listing"));
                        
                        delete list;
                        
                    } else {
                        
                        DMAP* dmapContainer = ((NSDictionary*)obj)._dmap;
                        dmap->addContainer(dmapContainer, tag);
                        delete dmapContainer;
                        
                    }
                    
                    break;
                    
                }
                    
            }
            
        }
        
    }
    
    return dmap;
    
}

- (id)_initWithDMAP:(DMAP *)dmap {
    
    NSUInteger count = dmap->getCount();
    NSMutableDictionary* dmapDictionary = [[NSMutableDictionary alloc] initWithCapacity:count];
    
    for (NSUInteger i = 0 ; i < count ; i++) {
        
        NSUInteger tag = dmap->getTagAtIndex(i);
        NSString* key = [[NSString alloc] initWithCString:DMAP::tagIdentifierForTag(tag) encoding:NSUTF8StringEncoding];
        
        switch (DMAP::typeForTag(tag)) {
                
            case kDMAPTypeUnknown:
                [dmapDictionary setObject:[NSData dataWithBytes:dmap->bytesAtIndex(i) length:dmap->getSizeOfAtomAtIndex(i)] forKey:key];
                break;

            case kDMAPTypeChar:
                [dmapDictionary setObject:[NSNumber numberWithChar:dmap->charAtIndex(i)] forKey:key];
                break;
                
            case kDMAPTypeShort:
                [dmapDictionary setObject:[NSNumber numberWithShort:dmap->shortAtIndex(i)] forKey:key];
                break;
                
            case kDMAPTypeLong:
                [dmapDictionary setObject:[NSNumber numberWithInteger:dmap->longAtIndex(i)] forKey:key];
                break;
                
            case kDMAPTypeLongLong:
                [dmapDictionary setObject:[NSNumber numberWithLongLong:dmap->longlongAtIndex(i)] forKey:key];
                break;
                
            case kDMAPTypeDate:
                [dmapDictionary setObject:[NSDate dateWithTimeIntervalSinceNow:dmap->dateAtIndex(i)] forKey:key];
                break;
                
            case kDMAPTypeVersion: {
                
                DMAPVersion version = dmap->versionAtIndex(i);
                [dmapDictionary setObject:[NSData dataWithBytes:&version length:sizeof(DMAPVersion)] forKey:key];
                break;
                
            }
                
            case kDMAPTypeString:
                [dmapDictionary setObject:[NSString stringWithCString:dmap->stringAtIndex(i) encoding:NSUTF8StringEncoding] forKey:key];
                break;
                
            case kDMAPTypeContainer: {
                
                if (tag == DMAP::tagForTagIdentifier("dmap.listing")) {
                    
                    DMAP* list = dmap->containerAtIndex(i);
                    NSUInteger count = list->getCount();
                    NSMutableArray* array = [[NSMutableArray alloc] initWithCapacity:count];
                    for (NSUInteger x = 0 ; x < count ; x++)
                        [array addObject:[[[NSDictionary alloc] _initWithDMAP:list->containerAtIndex(x)] autorelease]];
                    
                    [dmapDictionary setObject:[[array copy] autorelease] forKey:key];
                    [array release];
                    
                } else
                    [dmapDictionary setObject:[[[NSDictionary alloc] _initWithDMAP:dmap->containerAtIndex(i)] autorelease] forKey:key];
                                
                break;
                
            }
                
        }
        
        [key release];
        
    }
    
    NSDictionary* ret = [dmapDictionary copy];
    [dmapDictionary release];
    
    return ret;
    
}

@end
