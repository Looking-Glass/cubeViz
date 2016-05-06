//
//  NetManager.h
//  t1
//
//  Created by niit on 16/4/6.
//  Copyright © 2016年 NIIT. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NetManager : NSObject

+ (void)requestListSuccessBlock:(void (^)(NSArray *list))successBlock
                      failBlock:(void (^)(NSError *error))failBlock;
+ (NSArray *)loadList;
@end
