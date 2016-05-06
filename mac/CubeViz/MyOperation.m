//
//  MyOperation.m
//  CubetubeTool
//
//  Created by qiang on 16/4/8.
//  Copyright © 2016年 Cubetube. All rights reserved.
//

#import "MyOperation.h"

static MyOperation *instance = nil;

@implementation MyOperation
{
    NSString *dfuPath;
    
    NSString *tmpRbFilePath1;// install rb file
    NSString *tmpRbFilePath2;// uninstall rb file
    
    BOOL bNeedUninstall;
    BOOL bNeedInstallXRun;
    
    NSTask *unixTask;
    NSPipe *unixStandardOutputPipe;
    NSPipe *unixStandardErrorPipe;
    NSPipe *unixStandardInputPipe;
    NSFileHandle *fhOutput;
    NSFileHandle *fhError;
    NSFileHandle *fhInput;
    NSData *standardOutputData;
    NSData *standardErrorData;
    
    NSString *file1;

    // 设备名
    NSString *deviceName;
    
    NSString *logStr;
}

#pragma mark -
+ (MyOperation *)sharedOperation
{
    if(instance == nil)
    {
        instance = [[MyOperation alloc] init];
    }
    return instance;
}

- (void)allStep
{
    logStr = @"";
    [self performSelectorInBackground:@selector(step1) withObject:nil];
}

#pragma mark -
- (void)showLog:(NSString *)info
{
    if(info == nil||info.length < 1) return;
    DLog(@"%@",info);
    [self.delegate showInfo:info];

    NSString *filePath = [NSHomeDirectory() stringByAppendingPathComponent:@"CubeViz/log.txt"];
    logStr = [logStr stringByAppendingString:info];
    [logStr writeToFile:filePath atomically:YES encoding:NSUTF8StringEncoding error:nil];
}

#pragma mark - step1 安装相关工具dfu-util
- (void)step1
{
    DLog(@"step1 begin");
    [self.delegate updateProgress:3];
    if(![self checkDfuUtil])
    {
        [self.delegate closeDlg];
        return;
    }
    [self.delegate updateProgress:4];
    [self performSelectorInBackground:@selector(step2) withObject:nil];
}

#pragma mark -
- (NSString *)runCmd:(NSString *)cmd andArgs:(NSString *)args
{
    NSString *log = [NSString stringWithFormat:@"%@ %@\n",cmd,args];
//    [self showLog:log];
    @try
    {
        NSArray *arr = [args componentsSeparatedByString:@" "];
        
        NSTask *task;
        task = [[NSTask alloc ]init];
        [task setLaunchPath:cmd];
        [task setArguments:arr];
        
        NSPipe *pipe;
        pipe = [NSPipe pipe];
        [task setStandardOutput:pipe];
        
        NSFileHandle *file;
        file = [pipe fileHandleForReading];
        
        [task launch];
        
        NSData *data;
        data = [file readDataToEndOfFile];
        
        NSString *string;
        string = [[NSString alloc]initWithData:data encoding:NSUTF8StringEncoding];
        
        return string;
    }@catch (NSException * e)
    {
        NSString *log = [NSString stringWithFormat:@"Exception: %@", e];
        [self showLog:log];
        return nil;
    }
    
}

- (void)runCmd:(NSString *)cmd withArgs:(NSString *)args
{
    NSString *log = [NSString stringWithFormat:@"cmd:%@ %@\n",cmd,args];
    [self showLog:log];
    @try
    {
        //setup system pipes and filehandles to process output data
        unixStandardOutputPipe = [[NSPipe alloc] init];
        unixStandardErrorPipe = [[NSPipe alloc] init];
        unixStandardInputPipe = [[NSPipe alloc] init];
        fhOutput = [unixStandardOutputPipe fileHandleForReading];
        fhError = [unixStandardErrorPipe fileHandleForReading];
        fhInput = [unixStandardInputPipe fileHandleForWriting];
        //setup notification alerts
        NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
        [nc addObserver:self selector:@selector(notifiedForStdOutput:) name:NSFileHandleReadCompletionNotification object:fhOutput];
        [nc addObserver:self selector:@selector(notifiedForStdError:) name:NSFileHandleReadCompletionNotification object:fhError];
        [nc addObserver:self selector:@selector(notifiedForComplete:) name:NSTaskDidTerminateNotification object:unixTask];
        
        NSArray *arr = [args componentsSeparatedByString:@" "];
        
        unixTask = [[NSTask alloc ]init];
        [unixTask setLaunchPath:cmd];
        [unixTask setArguments:arr];
        
        [unixTask setStandardOutput:unixStandardOutputPipe];
        [unixTask setStandardError:unixStandardErrorPipe];
        [unixTask setStandardInput:unixStandardInputPipe];
        
        //note we are calling the file handle not the pipe
        [fhOutput readInBackgroundAndNotify];
        [fhError readInBackgroundAndNotify];
        
        [unixTask launch];
//        [unixTask waitUntilExit];
    }@catch (NSException * e)
    {
        NSString *log = [NSString stringWithFormat:@"cmd exception:%@", e];
        if([unixTask isRunning])
        {
            [unixTask terminate];
        }
        unixTask = nil;
        [[NSNotificationCenter defaultCenter] removeObserver:self];
        
        [self showLog:log];
    }
}

#pragma mark -
-(void)notifiedForStdOutput: (NSNotification *)notified
{
    NSData * data = [[notified userInfo] valueForKey:NSFileHandleNotificationDataItem];
//    DLog(@"standard data ready %ld bytes",data.length);
    if ([data length]){
        NSString * outputString = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
        [self showLog:[NSString stringWithFormat:@"%@",outputString]];
    }
    if (unixTask != nil) {
        [fhOutput readInBackgroundAndNotify];
    }
}

-(void) notifiedForStdError: (NSNotification *)notified
{
    NSData * data = [[notified userInfo] valueForKey:NSFileHandleNotificationDataItem];
//    DLog(@"standard error ready %ld bytes",data.length);
    if ([data length])
    {
        NSString * outputString = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
        [self showLog:[NSString stringWithFormat:@"%@",outputString]];
    }
    if (unixTask != nil) {
        [fhError readInBackgroundAndNotify];
    }
}

-(void) notifiedForComplete:(NSNotification *)anotification
{
    
    if ([unixTask terminationStatus] == 0)
    {
        [self.delegate updateProgress:13];
        [self showLog:@"[cmd succeeded]\n"];
        [self.delegate closeDlg];
    }
    else
    {
        NSString *str = [NSString stringWithFormat:@"cmd failed exit code=%d\n",[unixTask terminationStatus]];
        [self showLog:str];
        [self.delegate closeDlg];
        
//        if(bNeedInstallXRun)
//        {
//            [self showLog:@"\nAfter Install xcrun,push update button again.\n"];
//        }
    }
    
    unixTask = nil;
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

#pragma mark -
- (BOOL)checkDfuUtil
{
    NSString *tmpPath = [[NSBundle mainBundle] pathForResource:@"dfu-util" ofType:@""];
    if([[NSFileManager defaultManager] fileExistsAtPath:tmpPath])
    {
        dfuPath = tmpPath;
        return YES;
    }
    
    tmpPath = [[NSHomeDirectory() stringByAppendingPathComponent:@"CubeViz"] stringByAppendingPathComponent:@"opt/local/bin/dfu-util"];
    if([[NSFileManager defaultManager] fileExistsAtPath:tmpPath])
    {
        dfuPath = tmpPath;
        return YES;
    }
    if([[NSFileManager defaultManager] fileExistsAtPath:@"/opt/local/bin/dfu-util"])
    {
        dfuPath = @"/opt/local/bin/dfu-util";
        return YES;
    }
    if([[NSFileManager defaultManager] fileExistsAtPath:@"/usr/local/bin/dfu-util"])
    {
        dfuPath = @"/usr/local/bin/dfu-util";
        return YES;
    }
    return NO;
}

#pragma mark - step2 下载bin
// 下载bin
//http://6x.cubetube.org/apps/
//[{"id":1, "title": "demo application", "binary":"http://6x.cubetube.org/binaries/demo.bin", "icon": "http://6x.cubetube.org/images/demo.png", "lastModified": "2016-01-21 14:21:13"}, {"id":2, "title": "music pack", "binary":"http://6x.cubetube.org/binaries/music.bin", "icon": "http://6x.cubetube.org/images/music.png", "lastModified": "2016-01-19 10:15:09"}, ]
- (void)step2
{
    DLog(@"step2 begin");
    [self.delegate updateProgress:5];
    [self showLog:@"Downloading firmware.\n"];
    
    NSString *urlStr = self.info.binary;
    NSURL *url = [NSURL URLWithString:urlStr];
    NSString *filePath = [NSTemporaryDirectory() stringByAppendingPathComponent:url.lastPathComponent];
//    [self showLog:filePath];
    
    NSURLRequest *request = [NSURLRequest requestWithURL:url];
    [NSURLConnection sendAsynchronousRequest:request queue:[NSOperationQueue mainQueue] completionHandler:^(NSURLResponse *response, NSData *data, NSError *connectionError) {
        if(connectionError == nil)
         {
             if(data.length<1)
             {
                 [self showLog:@"Download failed."];
                 [self.delegate closeDlg];
                 return;
             }
             if(data.length>108*1024)
             {
                 [self showLog:@"This bin is too big.Can't write"];
                 [self.delegate closeDlg];
                 return;
             }
             [data writeToFile:filePath  atomically:YES];
             file1=filePath;
             [self.delegate updateProgress:7];
             [self performSelector:@selector(step3) withObject:nil afterDelay:1];
         }
         else
         {
             NSString *msg = [NSString stringWithFormat:@"Download Failed:%@",[connectionError localizedDescription]];
             [self showLog:msg];
             
             file1 = [[NSHomeDirectory() stringByAppendingPathComponent:@"CubeViz"] stringByAppendingPathComponent:url.lastPathComponent];
             if([[NSFileManager defaultManager] fileExistsAtPath:file1])
             {
                 [self showLog:@"Download Failed:use local file."];
                 [self.delegate updateProgress:7];
                 [self performSelector:@selector(step3) withObject:nil afterDelay:1];
             }
             else
             {
                 [self.delegate closeDlg];
             }
         }
    }];
    DLog(@"step2 end");
}


#pragma mark - step3 查找设备,打开dfu模式
//- (void)step3
//{
//    DLog(@"step3 begin");
//    [self.delegate updateProgress:8];
//    if(![self checkInitDeviceStatus])
//    {
//        if([self findDevice])
//        {
//            [self showLog:[NSString stringWithFormat:@"Find device:%@",deviceName]];
//            char *cDeviceName = [deviceName UTF8String];
//            myProcess2(cDeviceName);
//            
//            if(![self checkInitDeviceStatus])
//            {
//                [self showLog:@"Can't Init Device!"];
//                [self.delegate closeDlg];
//                return;
//            }
//        }
//        else
//        {
//            [self showLog:@"Can't Find Device!"];
//            [self.delegate closeDlg];
//            return;
//        }
//    }
//    [self performSelector:@selector(step4) withObject:nil afterDelay:0.01];
//}

- (void)step3
{
    dispatch_async(dispatch_get_global_queue(0, 0), ^{
        [self showLog:@"please connect L3DCube"];
        while(![self checkInitDeviceStatus])
        {
            if([self findDevice])
            {
                [self showLog:[NSString stringWithFormat:@"Find device:%@",deviceName]];
                char *cDeviceName = [deviceName UTF8String];
                myProcess2(cDeviceName);
            }
            [NSThread sleepForTimeInterval:0.2];
        }
        dispatch_async(dispatch_get_main_queue(), ^{
            [self step4];
        });
    });
}

- (BOOL)checkInitDeviceStatus
{
    NSString *result;
    result = [self runCmd:dfuPath andArgs:@"-l"];
//    [self showLog:result];
    if(result!=nil && [result rangeOfString:@"1eaf:0003"].location != NSNotFound)
    {
        return YES;
    }
    return NO;
}

- (BOOL)findDevice
{
//    [self showLog:@"Find device.\n"];
    NSString *string = [self runCmd:@"/bin/ls" andArgs:@"/dev/"];
    NSCharacterSet *cs = [NSCharacterSet whitespaceAndNewlineCharacterSet];
    NSArray *arr = [string componentsSeparatedByCharactersInSet:cs];
    for(NSString *name in arr)
    {
        NSRange range = [name rangeOfString:@"tty.usbmodem"];
        NSRange range2 = [name rangeOfString:@"tty.usbserial"];
        if(range.location != NSNotFound || range2.location != NSNotFound)
        {
            NSString *log=[NSString stringWithFormat:@"Finded:%@\n",name];
            [self showLog:log];
            deviceName = [NSString stringWithFormat:@"/dev/%@",name];
            return YES;
        }
    }
//    [self showLog:@"Can't find Cubetube device.\n"];
    return NO;
}

#pragma mark - step4 烧写
- (void)step4
{
    DLog(@"step4 begin");
    [self.delegate updateProgress:10];
    [self showLog:@"Write firmware to device.\n"];
    NSString *str = [NSString stringWithFormat:@"-d 1eaf:0003 -a 1 -D %@ -R",file1];
    [self runCmd:dfuPath withArgs:str];
    DLog(@"step4 end");
}


@end
