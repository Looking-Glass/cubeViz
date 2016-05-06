//
//  MyOperation.h
//  CubetubeTool
//
//  Created by qiang on 16/4/8.
//  Copyright © 2016年 Cubetube. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "BinInfo.h"

@protocol  MyOperationDelegate <NSObject>

// 显示信息
- (void)showInfo:(NSString *)info;
// 关闭显示框
- (void)closeDlg;
// 更新
- (void)updateProgress:(int)tmpProgress;

@end

@interface MyOperation : NSObject

+ (MyOperation *)sharedOperation;

@property (nonatomic,strong) BinInfo *info;

@property (nonatomic,weak) id<MyOperationDelegate> delegate;

- (void)allStep;

@end
