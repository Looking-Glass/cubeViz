//
//  BinInfo.h
//  t1
//
//  Created by niit on 16/4/6.
//  Copyright © 2016年 NIIT. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface BinInfo : NSObject

//[{"id":1, "title": "demo application", "binary":"http://6x.cubetube.org/binaries/demo.bin", "icon": "http://6x.cubetube.org/images/demo.png", "lastModified": "2016-01-21 14:21:13"}, {"id":2, "title": "music pack", "binary":"http://6x.cubetube.org/binaries/music.bin", "icon": "http://6x.cubetube.org/images/music.png", "lastModified": "2016-01-19 10:15:09"}, ]

@property (nonatomic,assign) int binID;
@property (nonatomic,copy) NSString *title;
@property (nonatomic,copy) NSString *binary;
@property (nonatomic,copy) NSString *icon;
@property (nonatomic,copy) NSString *lastModified;

- (instancetype)initWithDict:(NSDictionary *)dict;
+ (instancetype)binInfoWithDict:(NSDictionary *)dict;

@end
