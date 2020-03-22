//
//  BBASMRecordHierarchyModel.h
//  BBAMNP
//
//  Created by chenronghang on 2020/2/13.
//  Copyright © 2020 Baidu. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "BBASMRecordModel.h"

NS_ASSUME_NONNULL_BEGIN

@interface BBASMRecordHierarchyModel : NSObject <NSCopying>

@property (nonatomic, strong)BBASMRecordModel *rootMethod;
@property (nonatomic, copy)NSArray *subMethods;
@property (nonatomic, assign)BOOL isExpand;   //是否展开所有的子函数

- (instancetype)initWithRecordModelArr:(NSArray *)recordModelArr;
- (BBASMRecordModel *)getRecordModel:(NSInteger)index;

@end

NS_ASSUME_NONNULL_END
