//
//  ADTools.h
//  AppDraft
//
//  Created by Leks on 13-8-31.
//  Copyright (c) 2013年 Leks Zhang. All rights reserved.
//

#import <Foundation/Foundation.h>

#define kADRulerBackgroundColor [ADTools colorWithRGB:@"48,48,48"]
#define kADRulerStepColor [ADTools colorWithRGB:@"152,152,152"]
#define kADRulerSideColor [ADTools colorWithRGB:@"79,79,79"]

#define kADSettingScale         @"kADSettingScale"
#define kADSettingSnap          @"kADSettingSnap"
#define kADSettingRuler         @"kADSettingRuler"
#define kADSettingHelperLine         @"kADSettingHelperLine"
#define kADSettingGrid          @"kADSettingGrid"

#define kADSettingPrimaryMenu       @"kADSettingPrimaryMenu"
#define kADSettingSecondaryMenu     @"kADSettingSecondaryMenu"

#define kADPrimaryMenuPage          @"kADPrimaryMenuPage"
#define kADPrimaryMenuControl       @"kADPrimaryMenuControl"
#define kADPrimaryMenuGroup         @"kADPrimaryMenuGroup"
#define kADPrimaryMenuMultiControl  @"kADPrimaryMenuMultiControl"


#define kADSecondaryMenuTransform   @"kADSecondaryMenuTransform"
#define kADSecondaryMenuFont        @"kADSecondaryMenuFont"
#define kADSecondaryMenuControlBackground  @"kADSecondaryMenuControlBackground"
#define kADSecondaryMenuPageBackground  @"kADSecondaryMenuPageBackground"
#define kADSecondaryMenuAction        @"kADSecondaryMenuAction"

//#define kADSecondaryMenuTransform   @"kADSecondaryMenuTransform"

@interface ADTools : NSObject

+(CGFloat)minYBetweenRect:(CGRect)r1 r2:(CGRect)r2;
+(CGFloat)maxYBetweenRect:(CGRect)r1 r2:(CGRect)r2;
+(CGFloat)minXBetweenRect:(CGRect)r1 r2:(CGRect)r2;
+(CGFloat)maxXBetweenRect:(CGRect)r1 r2:(CGRect)r2;

+(CGFloat)screenScaleFactor;

//r,g,b
+(NSColor*)colorWithRGB:(NSString*)rgb;

+(void)setConfigValue:(id)value forKey:(NSString*)key;
+(id)configValueForKey:(NSString*)key;
@end
