//
//  MyView.m
//  CubetubeTool
//
//  Created by qiang on 16/4/8.
//  Copyright © 2016年 Cubetube. All rights reserved.
//

#import "MyView.h"

@implementation MyView

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];
    
    [self.backgoundColor set];
    NSRectFill(dirtyRect);
}

- (void)setBackgoundColor:(NSColor *)backgoundColor
{
    _backgoundColor = backgoundColor;
    [self setNeedsDisplay:YES];
}
@end
