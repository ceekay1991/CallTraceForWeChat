//
//  BBASMRecordModel.h
//  BBAMNP
//
//  Created by chenronghang on 2020/2/13.
//  Copyright © 2020 Baidu. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <objc/runtime.h>

NS_ASSUME_NONNULL_BEGIN

@interface BBASMRecordModel : NSObject <NSCopying>

@property (nonatomic, strong)Class cls;
@property (nonatomic)SEL sel;
@property (nonatomic, assign)uint64_t costTime; //单位：纳秒（百万分之一秒）
@property (nonatomic, assign)int depth;
@property (nonatomic, assign)BOOL isMainThread;

// 辅助堆栈排序
@property (nonatomic, assign)int total;
@property (nonatomic)BOOL isUsed;

//call 次数
@property (nonatomic, assign)int callCount;

- (instancetype)initWithCls:(Class)cls sel:(SEL)sel time:(uint64_t)costTime depth:(int)depth total:(int)total;

- (BOOL)isEqualRecordModel:(BBASMRecordModel *)model;

@end

NS_ASSUME_NONNULL_END
