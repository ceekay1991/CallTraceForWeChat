//
//  BBASMRecordHierarchyModel.m
//  BBAMNP
//
//  Created by chenronghang on 2020/2/13.
//  Copyright © 2020 Baidu. All rights reserved.
//

#import "BBASMRecordHierarchyModel.h"

@implementation BBASMRecordHierarchyModel

- (instancetype)initWithRecordModelArr:(NSArray *)recordModelArr {
    self = [super init];
    if (self) {
        if ([recordModelArr isKindOfClass:NSArray.class] && recordModelArr.count > 0) {
            self.rootMethod = recordModelArr[0];
            self.isExpand = YES;
            if (recordModelArr.count > 1) {
                self.subMethods = [recordModelArr subarrayWithRange:NSMakeRange(1, recordModelArr.count-1)];
            }
        }
    }
    return self;
}

- (BBASMRecordModel *)getRecordModel:(NSInteger)index {
    if (index==0) {
        return self.rootMethod;
    }
    return self.subMethods[index-1];
}

- (id)copyWithZone:(NSZone *)zone {
    BBASMRecordHierarchyModel *model = [[[self class] allocWithZone:zone] init];
    model.rootMethod = self.rootMethod;
    model.subMethods = self.subMethods;
    model.isExpand = self.isExpand;
    return model;
}

- (NSString *)description {
    NSMutableString *str = [NSMutableString new];
    [str appendFormat:@"%@\r", [_rootMethod description]];
    for (BBASMRecordModel *sub in _subMethods) {
        [str appendFormat:@"%@\r", [sub description]];
    }
    return str;
    
}
@end
