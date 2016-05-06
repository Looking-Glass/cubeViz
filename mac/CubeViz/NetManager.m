//
//  NetManager.m
//  t1
//
//  Created by niit on 16/4/6.
//  Copyright © 2016年 NIIT. All rights reserved.
//

#import "NetManager.h"
#import "BinInfo.h"

@implementation NetManager

+ (void)requestListSuccessBlock:(void (^)(NSArray *list))successBlock
                      failBlock:(void (^)(NSError *error))failBlock
{
    NSURL *url = [NSURL URLWithString:@"http://6x.cubetube.org/apps/"];
    NSURLRequest *request = [NSURLRequest requestWithURL:url];
    
    [NSURLConnection sendAsynchronousRequest:request queue:[NSOperationQueue mainQueue] completionHandler:^(NSURLResponse *response, NSData *data, NSError *connectionError) {
        
        NSArray *arr;
        if(connectionError == nil)
        {
            arr = [NSJSONSerialization JSONObjectWithData:data options:NSJSONReadingMutableLeaves error:nil];
        }
        
        if(arr == nil||arr.count < 1)
        {
            if([[NSUserDefaults standardUserDefaults] objectForKey:@"info"]!=nil)
            {
                arr = [[NSUserDefaults standardUserDefaults] objectForKey:@"info"];
            }
        }
        else
        {
            [[NSUserDefaults standardUserDefaults] setObject:arr forKey:@"info"];
            [[NSUserDefaults standardUserDefaults] synchronize];
        }
        
        if(arr != nil&& arr.count>0)
        {
            NSMutableArray *mArr = [NSMutableArray array];
            for(int i=0;i<1;i++)
            {
                for(NSDictionary *info in arr)
                {
                    BinInfo *binInfo = [BinInfo binInfoWithDict:info];
                    [mArr addObject:binInfo];
                }
            }
            successBlock(mArr);
        }
        else
        {
            failBlock(connectionError);
        }
        
        
    }];
}

+ (NSArray *)loadList
{
    NSArray *arr;
    NSMutableArray *mArr = [NSMutableArray array];
    if([[NSUserDefaults standardUserDefaults] objectForKey:@"info"]!=nil)
    {
        arr = [[NSUserDefaults standardUserDefaults] objectForKey:@"info"];
    }
    if(arr != nil&& arr.count>0)
    {
        for(NSDictionary *info in arr)
        {
            BinInfo *binInfo = [BinInfo binInfoWithDict:info];
            [mArr addObject:binInfo];
        }
    }
    return mArr;
}
@end
