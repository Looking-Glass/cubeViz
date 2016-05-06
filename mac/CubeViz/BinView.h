//
//  BinView.h
//  CubetubeTool
//
//  Created by qiang on 16/4/7.
//  Copyright © 2016年 Cubetube. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import "MyView.h"

#import "BinInfo.h"

@class BinView;

@protocol BinViewDelegate <NSObject>

- (void)binViewBtnPressed:(BinView *)binView;

@end

@interface BinView : MyView

@property (nonatomic,strong) BinInfo *info;

@property (nonatomic,weak) id<BinViewDelegate> delegate;

@end
