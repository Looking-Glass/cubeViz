//
//  AppDelegate.m
//  CubeViz
//
//  Created by luozhuang on 16-4-28.
//  Copyright (c) 2016年 CubeTube. All rights reserved.
//

#import "AppDelegate.h"

#import "MyView.h"
#import "BinView.h"
#import "BinInfo.h"
#import "MyOperation.h"
#import "NetManager.h"
#import "NSImage+RoundedRectImage.h"

@interface AppDelegate()<BinViewDelegate,MyOperationDelegate>
{
    // 进度
    int progress;
    // 定时器
    NSTimer *t;
    
}

// 1. 滑动视图
// 滑动视图
@property (weak) IBOutlet NSScrollView *scrollView;
@property (nonatomic,strong) NSMutableArray *binViewArr;

// 2. 弹出视图
// 弹出视图
@property (weak) IBOutlet MyView *popInBgView;
// 信息
@property (weak) IBOutlet NSTextField *info1;
@property (weak) IBOutlet NSTextField *info2;
@property (weak) IBOutlet NSTextField *info3;
// 进度条
@property (weak) NSLevelIndicator *indicator;

@end

@implementation AppDelegate

#pragma mark - 页面生命周期
- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // 1. 创建CubeViz缓存目录
    NSFileManager *fm = [NSFileManager defaultManager];
    NSString *path = [NSHomeDirectory() stringByAppendingPathComponent:@"CubeViz"];
    if(![fm fileExistsAtPath:path])
    {
        [fm createDirectoryAtPath:path withIntermediateDirectories:YES attributes:nil error:nil];
    }
    
    self.binViewArr = [NSMutableArray array];
    
    // 2. 检测验证
//    if([self checkAvailable])
    {
        // 3. 设置界面
        [self setupPopView];
        
        // 4. 读取数据
        NSArray *list = [NetManager loadList];
        [self loadList:list];
        [self performSelector:@selector(requestList) withObject:nil afterDelay:2];
    }
}

- (BOOL)applicationShouldHandleReopen:(NSApplication *)theApplication
                    hasVisibleWindows:(BOOL)flag{
    if (!flag)
    {
        NSWindow *w = theApplication.windows[0];
        [w makeKeyAndOrderFront:self];
    }
    return YES;
}

#pragma mark - 设置界面
- (void)setupPopView
{
    MyView *view = [[MyView alloc] initWithFrame:CGRectMake(205, 140, 450, 220)];
    view.backgoundColor = [NSColor blackColor];
    [self.window.contentView addSubview:view];
    self.popInBgView = view;
    
    MyView *view2 = [[MyView alloc] initWithFrame:CGRectMake(3, 3, 444, 214)];
    view2.backgoundColor = [NSColor whiteColor];
    [view addSubview:view2];
    
    MyView *view3 = [[MyView alloc] initWithFrame:CGRectMake(6, 125, 430, 3)];
    view3.backgoundColor = [NSColor blackColor];
    [view2 addSubview:view3];
    
    NSTextField *text1 = [[NSTextField alloc] initWithFrame:CGRectMake(50, 140, 100, 30)];
    [view2 addSubview:text1];
    text1.stringValue = @"Updating";
    text1.bordered = NO;
    text1.editable = NO;
    text1.font = [NSFont fontWithName:@"Arial" size:18];
    self.info1 = text1;
    
    NSLevelIndicator *indicator = [[NSLevelIndicator alloc] initWithFrame:CGRectMake(140, 150, 280, 50)];
    indicator.maxValue = 13;
    [indicator setIntValue:1];
    [view2 addSubview:indicator];
    self.indicator = indicator;
    
    NSTextField *text2 = [[NSTextField alloc] initWithFrame:CGRectMake(60, 80, 200, 30)];
    [view2 addSubview:text2];
    text2.stringValue = @"You have selected to play";
    text2.bordered = NO;
    text2.editable = NO;
    text2.font = [NSFont fontWithName:@"Arial-BoldMT" size:14];
    text2.alignment = kCTTextAlignmentRight;
    
    NSTextField *text3 = [[NSTextField alloc] initWithFrame:CGRectMake(260, 80, 150, 30)];
    [view2 addSubview:text3];
    text3.stringValue = @"demo application";
    text3.textColor = [NSColor blueColor];
    text3.bordered = NO;
    text3.editable = NO;
    text3.font = [NSFont fontWithName:@"Arial-BoldMT" size:14];
    text3.alignment = kCTTextAlignmentLeft;
    self.info2 = text3;
    
    NSTextField *text4 = [[NSTextField alloc] initWithFrame:CGRectMake(30, 10, 400, 80)];
    [view2 addSubview:text4];
    text4.stringValue = @"You are now saving this visualization to your cube.\nIt will replace the existing visualization.";
    text4.bordered = NO;
    text4.editable = NO;
    text4.font = [NSFont fontWithName:@"Arial" size:14];
    text4.alignment = kCTTextAlignmentCenter;
    
    NSTextField *text5 = [[NSTextField alloc] initWithFrame:CGRectMake(5,5, 450, 30)];
    [view2 addSubview:text5];
    text5.stringValue = @"";
    text5.bordered = NO;
    text5.editable = NO;
    text5.font = [NSFont fontWithName:@"Arial" size:10];
    text5.alignment = kCTTextAlignmentCenter;
    
    self.info3 = text5;
    
    [self.popInBgView setHidden:YES];
    
    progress = 1;
    
}

#pragma mark - 请求列表及解析
// 请求列表
- (void)requestList
{
    if(self.popInBgView.hidden)
    {
        [NetManager requestListSuccessBlock:
         ^(NSArray *arr){
             // 请求成功,加载列表对象
             [self performSelector:@selector(loadList:) withObject:arr afterDelay:2 inModes:@[NSDefaultRunLoopMode]];
             // 60分钟后,再次获取
             [self performSelector:@selector(requestList) withObject:nil afterDelay:60*60];
         }
                                  failBlock:
         ^(NSError *error){
             // 获取失败,3分钟后重新尝试
             [self performSelector:@selector(requestList) withObject:nil afterDelay:3*60];
         }];
    }
    else
    {
        // 正在烧写,3分钟后重新尝试
        [self performSelector:@selector(requestList) withObject:nil afterDelay:3*60];
    }
    
}

// 加载信息列表
- (void)loadList:(NSArray *)tmpArr
{
    if(tmpArr.count < 9)
    {
        [self loadListLe10:tmpArr];
        [self.window.contentView addSubview:self.popInBgView];
    }
    else
    {
        [self loadListGt9:tmpArr];
    }
}

- (void)loadListLe10:(NSArray *)arr
{
    [self.scrollView setHidden:YES];
    [self removeAllBinViews];    
    
    int cellWidth = 120;
    int cellHeight = 120;
    int cellPaddingX = 5;
    int cellPaddingY = 5;
    
    // 添加子视图
    for(int i=0;i<arr.count;i++)
    {
        // 创建BinView对象
        BinView *binView = [self createBinView];
        // 设定信息
        binView.info = arr[i];
        // 设定坐标
        int x = 440 + (cellWidth + cellPaddingX) * (i % 3);
        int y = 450 - (cellHeight + cellPaddingY) * ((i/3) + 1);
        binView.frame = CGRectMake(x, y, cellWidth,cellHeight);
        // 设置代理对象
        binView.delegate = self;
        // 添加到scrollView
        [self.window.contentView addSubview:binView];
        
        [self.binViewArr addObject:binView];
    }
}

- (void)loadListGt9:(NSArray *)arr
{
    
    // 1.
    [self removeAllBinViews];
    
    // 2. 添加子视图
    // 宽高间隔
    int cellWidth = 120;
    int cellHeight = 120;
    int cellPaddingX = 5;
    int cellPaddingY = 5;
    
    // 计算行数
    unsigned long lineCount = (arr.count - 1) / 3 + 1;
    DLog(@"共有%i行",lineCount);
    
    // 计算总高度
    CGFloat height = (cellHeight + cellPaddingY) * lineCount;
    NSLog(@"%f",height);
//    self.scrollView.contentView.bounds = NSMakeRect(0, 0, 400, height);
    
    NSImage *theImage = [NSImage imageNamed:@"bg.png"];
    theImage = [NSImage createRoundedRectImage:theImage withSize:CGSizeMake(400, height) withRadius:0];
    NSRect imageRect=NSMakeRect(0.0,0.0,[theImage size].width,[theImage size].height);
    NSImageView *bgImageView=[[NSImageView alloc] initWithFrame:imageRect];
    [bgImageView setBounds:imageRect];
    [bgImageView setImage:theImage];
    [self.scrollView setDocumentView:bgImageView];
    
    // 添加子视图
    for(int i=0;i<arr.count;i++)
    {
        // 创建BinView对象
        BinView *binView = [self createBinView];
        // 设定信息
        binView.info = arr[i];
        // 设定坐标
        int x = (cellWidth + cellPaddingX) * (i % 3);
        int y = (cellHeight + cellPaddingY) * (lineCount - i/3 -1);
        binView.frame = CGRectMake(x, y, cellWidth,cellHeight);
        // 设置代理对象
        binView.delegate = self;
        // 添加到scrollView
        [self.scrollView.contentView addSubview:binView];
        
        [self.binViewArr addObject:binView];
    }
    
    // 滑动到顶部
    [self.scrollView.contentView scrollToPoint:NSMakePoint(0, height-125*3)];

    [self.scrollView setHidden:NO];
}

- (BinView *)createBinView
{
    NSArray *arr;
    [[NSBundle mainBundle] loadNibNamed:@"BinView" owner:nil topLevelObjects:&arr];
    BinView *binView;
    for(int i=0;i<arr.count;i++)
    {
        if([arr[i] isKindOfClass:[BinView class]])
        {
            binView = arr[i];
        }
    }
    return binView;
}

- (void)removeAllBinViews
{
    for(BinView *v in self.binViewArr)
    {
        [v removeFromSuperview];
    }
    self.binViewArr = [NSMutableArray array];
}
// 移除scrollView子视图
//- (void)removeScrollViewSubviews
//{
//    NSMutableArray *mArr = [NSMutableArray array];
//    for(NSView *v in self.scrollView.contentView.subviews)
//    {
//        if([v isKindOfClass:[BinView class]])
//        {
//            [mArr addObject:v];
//        }
//    }
//    for(NSView *v in mArr)
//    {
//        [v removeFromSuperview];
//    }
//}

#pragma mark - binView按钮事件
- (void)binViewBtnPressed:(BinView *)binView
{
    if(![self.popInBgView isHidden])
    {
        return;
    }
    
    BinInfo *info = binView.info;
    self.info2.stringValue = info.title;
    
    [self.window.contentView addSubview:self.popInBgView];
    [self.popInBgView setHidden:NO];
    progress = 2;
    t = [NSTimer scheduledTimerWithTimeInterval:0.1 target:self selector:@selector(updateIndicator:) userInfo:nil repeats:YES];

    MyOperation *operation = [MyOperation sharedOperation];
    operation.info = info;
    operation.delegate = self;
    [operation allStep];
}

// 进度条动画
- (void)updateIndicator:(NSTimer *)tmpTimer
{
    if([self.popInBgView isHidden])
    {
        [tmpTimer invalidate];
    }
    
    static int p = 0;
    p++;
    [self.indicator setIntValue:p];
    if(p>=progress)
    {
        p=0;
    }
}

#pragma mark - 自定义方法
// 检测可用
//- (BOOL)checkAvailable
//{
//    NSDateFormatter *dateFormatter=[[NSDateFormatter alloc] init];
//    [dateFormatter setDateFormat:@"yyyy-MM-dd"];
//    NSString *dateStr2=@"2016-05-15";
//    NSDate *date2=[dateFormatter dateFromString:dateStr2];
//    NSTimeInterval i = [[NSDate date] timeIntervalSinceDate:date2];
//    int d = i / 60 / 60 / 24;
//    if(d > -15&&d < 15)
//    {
//        return YES;
//    }
//    return NO;
//}

#pragma mark - MyOperationDelegate Method
- (void)showInfo:(NSString *)info
{
    dispatch_async(dispatch_get_main_queue(), ^{
        self.info3.stringValue = info;
    });
}

- (void)closeDlg
{
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        [t invalidate];
        [self.indicator setIntValue:progress];
    });
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        [self.popInBgView setHidden:YES];
    });
}

- (void)updateProgress:(int)tmpProgress
{
    progress = tmpProgress;
}


- (void)scrollWheel:(NSEvent *)theEvent
{
    NSLog(@"%s",__func__);
}

@end
