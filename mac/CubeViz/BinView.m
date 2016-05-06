//
//  BinView.m
//  CubetubeTool
//
//  Created by qiang on 16/4/7.
//  Copyright © 2016年 Cubetube. All rights reserved.
//

#import "BinView.h"

#import "NSImage+RoundedRectImage.m"
@interface BinView()

@property (weak) IBOutlet NSImageView *imageView;
@property (weak) IBOutlet NSTextField *nameLabel;
@property (weak) IBOutlet NSButton *btn;

@end
@implementation BinView

- (void)setInfo:(BinInfo *)info
{
    _info = info;
    self.nameLabel.stringValue = info.title;
    
    NSURL *url = [NSURL URLWithString:info.icon];
    
    __weak BinView *weakSelf = self;
    
    NSString *imgPath = [[NSHomeDirectory() stringByAppendingPathComponent:@"CubeViz"] stringByAppendingPathComponent:url.lastPathComponent];
    if([[NSFileManager defaultManager] fileExistsAtPath:imgPath])
    {
        NSImage *img = [[NSImage alloc] initWithContentsOfFile:imgPath];
        self.btn.image = [NSImage createRoundedRectImage:img withSize:CGSizeMake(115, 115) withRadius:0];
    }

    NSURLRequest *request = [NSURLRequest requestWithURL:url];
    [NSURLConnection sendAsynchronousRequest:request queue:[NSOperationQueue mainQueue] completionHandler:^(NSURLResponse *response, NSData *data, NSError *connectionError) {
        if(connectionError == nil)
        {
            NSImage *img = [[NSImage alloc] initWithData:data];
            weakSelf.btn.image = [NSImage createRoundedRectImage:img withSize:CGSizeMake(115, 115) withRadius:0];
            [data writeToFile:imgPath atomically:YES];
        }
        else
        {
            DLog(@"%@",[connectionError localizedDescription]);
        }
    }];

    
    url = [NSURL URLWithString:info.binary];
    NSMutableURLRequest *mRequest = [[NSMutableURLRequest alloc] initWithURL:url];
    mRequest.HTTPMethod = @"HEAD";
    NSString *binPath = [[NSHomeDirectory() stringByAppendingPathComponent:@"CubeViz"] stringByAppendingPathComponent:url.lastPathComponent];
    
    [NSURLConnection sendAsynchronousRequest:mRequest queue:[[NSOperationQueue alloc] init] completionHandler:^(NSURLResponse *response, NSData *data, NSError *connectionError) {
        if(connectionError != nil)
        {
            return ;
        }
        
        DLog(@"%lld",response.expectedContentLength);
        NSFileManager *fm = [NSFileManager defaultManager];
        NSDictionary *infoDict = [fm attributesOfItemAtPath:binPath error:nil];
        if(infoDict != nil)
        {
//            DLog(@"%@",infoDict);
            if(response.expectedContentLength == [infoDict[NSFileSize] longLongValue])
            {
//                DLog(@"don't need download");
                return ;
            }
        }
        mRequest.HTTPMethod = @"GET";
        NSData *binData = [NSURLConnection sendSynchronousRequest:mRequest returningResponse:nil error:nil];
        [binData writeToFile:binPath atomically:YES];
    }];

}

- (IBAction)btnPressed:(id)sender {
    [self.delegate binViewBtnPressed:self];
}



@end
