//
//  NSDictionary+AirFloatAdditions.m
//  AirFloat
//
//  Created by Kristian Trenskow on 2/18/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "dmap.h"

#import "NSDictionary+AirFloatAdditions.h"

@interface NSDictionary (AirFloatPrivate)

@property (nonatomic, readonly) dmap_p _dmap;

- (id)_initWithDMAP:(dmap_p)dmap;

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
    
    dmap_p dmap = dmap_create();
    dmap_parse(dmap, [data bytes], [data length]);
    
    if (dmap != NULL) {
        
        ret = [self _initWithDMAP:dmap];
        dmap_destroy(dmap);
        
    }
    
    return ret;
    
}

- (NSData*)dmapTaggedData {
    
    dmap_p dmap = self._dmap;
    
    uint32_t len = dmap_get_length(dmap);
    void* bytes = malloc(len);
    
    dmap_get_content(dmap, bytes, len);
    
    NSData* ret = [NSData dataWithBytes:bytes length:len];
    
    free (bytes);
    dmap_destroy(dmap);
    
    return ret;
    
}

#pragma mark - - Private Methods

- (dmap_p)_dmap {
    
    dmap_p dmap = dmap_create();
    
    for (NSString* key in self.allKeys) {
        
        NSUInteger tag;
        if ((tag = dmap_tag_for_identifier([key cStringUsingEncoding:NSASCIIStringEncoding])) != 0) {
            
            id obj = [self objectForKey:key];
            
            switch (dmap_type_for_tag(tag)) {
                    
                case dmap_type_unknown:
                    dmap_add_bytes(dmap, [obj bytes], [obj length], tag);
                    break;
                    
                case dmap_type_char:
                    dmap_add_char(dmap, [obj charValue], tag);
                    break;

                case dmap_type_short:
                    dmap_add_short(dmap, [obj shortValue], tag);
                    break;
                    
                case dmap_type_long:
                    dmap_add_long(dmap, [obj integerValue], tag);
                    break;
                    
                case dmap_type_longlong:
                    dmap_add_longlong(dmap, [obj longLongValue], tag);
                    break;
                    
                case dmap_type_date:
                    dmap_add_date(dmap, [obj timeIntervalSince1970], tag);
                    break;
                    
                case dmap_type_version: {
                    
                    const void* bytes = [obj bytes];
                    dmap_version version = *((dmap_version*)bytes);
                    dmap_add_version(dmap, version, tag);
                    break;
                    
                }
                                        
                case dmap_type_string:
                    dmap_add_string(dmap, [obj cStringUsingEncoding:NSUTF8StringEncoding], tag);
                    break;
                    
                case dmap_type_container: {
                    
                    // This is a list
                    if ([obj isKindOfClass:[NSArray class]]) {
                        
                        dmap_p list = dmap_create();
                        
                        for (NSDictionary* dictionary in obj) {
                            
                            dmap_p dmapItem = dictionary._dmap;
                            dmap_add_container(list, dmap, dmap_tag_for_identifier("dmap.listingitem"));
                            dmap_destroy(dmapItem);
                            
                        }
                        
                        dmap_add_container(dmap, list, dmap_tag_for_identifier("dmap.listing"));
                        
                        dmap_destroy(list);
                        
                    } else {
                        
                        dmap_p dmapContainer = ((NSDictionary*)obj)._dmap;
                        dmap_add_container(dmap, dmapContainer, tag);
                        dmap_destroy(dmapContainer);
                        
                    }
                    
                    break;
                    
                }
                    
            }
            
        }
        
    }
    
    return dmap;
    
}

- (id)_initWithDMAP:(dmap_p)dmap {
    
    NSUInteger count = dmap_get_count(dmap);
    NSMutableDictionary* dmapDictionary = [[NSMutableDictionary alloc] initWithCapacity:count];
    
    for (NSUInteger i = 0 ; i < count ; i++) {
        
        NSUInteger tag = dmap_get_tag_at_index(dmap, i);
        NSString* key = [[NSString alloc] initWithCString:dmap_identifier_for_tag(tag) encoding:NSUTF8StringEncoding];
        
        switch (dmap_type_for_tag(tag)) {
                
            case dmap_type_unknown:
                [dmapDictionary setObject:[NSData dataWithBytes:dmap_bytes_at_index(dmap, i) length:dmap_get_size_of_atom_at_index(dmap, i)] forKey:key];
                break;

            case dmap_type_char:
                [dmapDictionary setObject:[NSNumber numberWithChar:dmap_char_at_index(dmap, i)] forKey:key];
                break;
                
            case dmap_type_short:
                [dmapDictionary setObject:[NSNumber numberWithShort:dmap_short_at_index(dmap, i)] forKey:key];
                break;
                
            case dmap_type_long:
                [dmapDictionary setObject:[NSNumber numberWithInteger:dmap_long_at_index(dmap, i)] forKey:key];
                break;
                
            case dmap_type_longlong:
                [dmapDictionary setObject:[NSNumber numberWithLongLong:dmap_longlong_at_index(dmap, i)] forKey:key];
                break;
                
            case dmap_type_date:
                [dmapDictionary setObject:[NSDate dateWithTimeIntervalSinceNow:dmap_date_at_index(dmap, i)] forKey:key];
                break;
                
            case dmap_type_version: {
                
                dmap_version version = dmap_version_at_index(dmap, i);
                [dmapDictionary setObject:[NSData dataWithBytes:&version length:sizeof(dmap_version)] forKey:key];
                break;
                
            }
                
            case dmap_type_string:
                [dmapDictionary setObject:[NSString stringWithCString:dmap_string_at_index(dmap, i) encoding:NSUTF8StringEncoding] forKey:key];
                break;
                
            case dmap_type_container: {
                
                if (tag == dmap_tag_for_identifier("dmap.listing")) {
                    
                    dmap_p list = dmap_container_at_index(dmap, i);
                    NSUInteger count = dmap_get_count(list);
                    NSMutableArray* array = [[NSMutableArray alloc] initWithCapacity:count];
                    for (NSUInteger x = 0 ; x < count ; x++)
                        [array addObject:[[[NSDictionary alloc] _initWithDMAP:dmap_container_at_index(list, x)] autorelease]];
                    
                    [dmapDictionary setObject:[[array copy] autorelease] forKey:key];
                    [array release];
                    
                } else
                    [dmapDictionary setObject:[[[NSDictionary alloc] _initWithDMAP:dmap_container_at_index(dmap, i)] autorelease] forKey:key];
                    
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
