//
//  ADTools.m
//  AppDraft
//
//  Created by Leks on 13-8-31.
//  Copyright (c) 2013年 Leks Zhang. All rights reserved.
//

#import "ADTools.h"

@implementation ADTools

+(CGFloat)minYBetweenRect:(CGRect)r1 r2:(CGRect)r2
{
    return (r1.origin.y < r2.origin.y)?r1.origin.y:r2.origin.y;
}

+(CGFloat)maxYBetweenRect:(CGRect)r1 r2:(CGRect)r2
{
    CGFloat y1 = r1.origin.y + r1.size.height;
    CGFloat y2 = r2.origin.y + r2.size.height;
    
    return (y1 > y2)?y1:y2;
}

+(CGFloat)minXBetweenRect:(CGRect)r1 r2:(CGRect)r2
{
    return (r1.origin.x < r2.origin.x)?r1.origin.x:r2.origin.x;
}

+(CGFloat)maxXBetweenRect:(CGRect)r1 r2:(CGRect)r2
{
    CGFloat x1 = r1.origin.x + r1.size.width;
    CGFloat x2 = r2.origin.x + r2.size.width;
    
    return (x1 > x2)?x1:x2;
}

//判断是否retina屏
+(CGFloat)screenScaleFactor
{
    float displayScale = 1;

    if ([[NSScreen mainScreen] respondsToSelector:@selector(backingScaleFactor)])
    {
        displayScale = [[NSScreen mainScreen] backingScaleFactor];
    }
    
    return displayScale;
}

+(NSColor*)colorWithRGB:(NSString*)rgb
{
    NSArray *rgbArray = [rgb componentsSeparatedByString:@","];
    if (rgbArray.count == 3)
    {
        NSString *r = [rgbArray objectAtIndex:0];
        NSString *g = [rgbArray objectAtIndex:1];
        NSString *b = [rgbArray objectAtIndex:2];
        
        return [NSColor colorWithDeviceRed:r.floatValue/256.0f green:g.floatValue/256.0f blue:b.floatValue/256.0f alpha:1.0f];
    }
    
    return nil;
}

+(void)setConfigValue:(id)value forKey:(NSString*)key
{
    NSMutableDictionary *settings = [ADTools settings];
    if (!value) {
        [settings removeObjectForKey:key];
    }
    else
    {
        [settings setObject:value forKey:key];
    }
    
    NSString *spath = [NSString stringWithFormat:@"%@/settings", NSHomeDirectory()];
    [settings writeToURL:[NSURL fileURLWithPath:spath] atomically:YES];
}

+(id)configValueForKey:(NSString*)key
{
    NSMutableDictionary *settings = [ADTools settings];
    return [settings objectForKey:key];
}

+(NSMutableDictionary*)settings
{
    NSString *spath = [NSString stringWithFormat:@"%@/settings", NSHomeDirectory()];
    NSMutableDictionary *md = [NSMutableDictionary dictionaryWithContentsOfURL:[NSURL fileURLWithPath:spath]];
    
    if (!md) {
        md = [NSMutableDictionary dictionary];
    }
    
    return md;
}
@end
