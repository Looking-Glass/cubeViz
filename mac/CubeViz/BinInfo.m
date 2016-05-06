//
//  BinInfo.m
//  t1
//
//  Created by niit on 16/4/6.
//  Copyright © 2016年 NIIT. All rights reserved.
//

#import "BinInfo.h"

@implementation BinInfo

- (instancetype)initWithDict:(NSDictionary *)dict
{
    self = [super init];
    if (self) {
        [self setValuesForKeysWithDictionary:dict];
    }
    return self;
}

+ (instancetype)binInfoWithDict:(NSDictionary *)dict
{
    return [[self alloc] initWithDict:dict];
}

- (void)setValue:(id)value forUndefinedKey:(NSString *)key
{
    if([key isEqualToString:@"id"])
    {
        [self setValue:value forKey:@"binID"];
    }
}

@end
