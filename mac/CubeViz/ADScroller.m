//
//  ADScroller.m
//  AppDraft
//
//  Created by Leks on 13-9-19.
//  Copyright (c) 2013年 Leks Zhang. All rights reserved.
//

#import "ADScroller.h"

#import "ADTools.h"

@implementation ADScroller

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code here.
    }
    
    return self;
}

- (void)drawRect:(NSRect)dirtyRect
{
    [[ADTools colorWithRGB:@"47,47,47"] set];
    NSRectFill(dirtyRect);
    // Drawing code here.
//    [super drawRect:dirtyRect];
    [self drawKnob];
}

@end
