//
//  BBASMRecordModel.m
//  BBAMNP
//
//  Created by chenronghang on 2020/2/13.
//  Copyright © 2020 Baidu. All rights reserved.
//

#import "BBASMRecordModel.h"

@implementation BBASMRecordModel

- (instancetype)initWithCls:(Class)cls
                        sel:(SEL)sel
                       time:(uint64_t)costTime
                      depth:(int)depth
                      total:(int)total {
    self = [super init];
    if (self) {
        _callCount = 0;
        _cls = cls;
        _sel = sel;
        _costTime = costTime;
        _depth = depth;
        _total = total;
        _isUsed = NO;
    }
    return self;
}

- (id)copyWithZone:(NSZone *)zone {
    BBASMRecordModel *model = [[[self class]  allocWithZone:zone] init];
    model.cls = self.cls;
    model.sel = self.sel;
    model.costTime = self.costTime;
    model.depth = self.depth;
    model.total = self.total;
    model.isUsed = self.isUsed;
    model.callCount = self.callCount;
    model.isMainThread = self.isMainThread;
    return model;
}

- (NSString *)description {
    NSMutableString *str = [NSMutableString new];
    [str appendFormat:@" %2d| ",(int)_depth];
    [str appendFormat:@" %6.2f |",_costTime/1000.0];
    [str appendFormat:@" %2d| ",_callCount];
    [str appendFormat:@" %@| ",_isMainThread ? @"主" : @"JS"];
    for (NSUInteger i = 0; i < _depth; i++) {
        [str appendString:@"  "];
    }

    [str appendFormat:@"%s[%@ %@]", (class_isMetaClass(_cls) ? "+" : "-"), _cls, NSStringFromSelector(_sel)];
    return str;
}

- (BOOL)isEqualRecordModel:(BBASMRecordModel *)model {
    if ([self.cls isEqual:model.cls] && self.sel==model.sel) {
        return YES;
    }
    return NO;
}

@end
