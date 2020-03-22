//
//  BBASMTimeProfileDataManager.h
//  BBAMNP
//
//  Created by chenronghang on 2020/2/13.
//  Copyright © 2020 Baidu. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface BBASMTimeProfileDataManager : NSObject
+ (void)addData:(uint64_t) time;
+ (void)clear;
@end

NS_ASSUME_NONNULL_END
