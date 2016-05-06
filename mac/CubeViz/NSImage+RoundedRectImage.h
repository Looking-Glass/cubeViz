//
//  UIImage+RoundedRectImage.h
//  QRCode
//
//  Created by qiang on 4/27/16.
//  Copyright © 2016 QiangTech. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface NSImage (RoundedRectImage)

+ (NSImage *)createRoundedRectImage:(NSImage *)image withSize:(CGSize)size withRadius:(NSInteger)radius;

@end