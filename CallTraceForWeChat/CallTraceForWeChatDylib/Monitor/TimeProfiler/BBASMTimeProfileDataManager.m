//
//  BBASMTimeProfileDataManager.m
//  BBAMNP
//
//  Created by chenronghang on 2020/2/13.
//  Copyright © 2020 Baidu. All rights reserved.
//

#import "BBASMTimeProfileDataManager.h"
static NSString *const BBASMTimeProfileDataManagerFileName = @"BBASMTimeProfileDataManager.plist";
@interface BBASMTimeProfileDataManager()
@property (nonatomic, strong) NSMutableArray *datArray;
@property (nonatomic, copy) NSString *filePath;
@end

static BBASMTimeProfileDataManager *profileDataManager;
@implementation BBASMTimeProfileDataManager
+ (instancetype)shareInstance {
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        profileDataManager = [[BBASMTimeProfileDataManager alloc] init];
    });
    return profileDataManager;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        _datArray = [NSMutableArray arrayWithCapacity:1];
    }
    return self;
}

- (void)save {
    NSArray *dataArray = [[BBASMTimeProfileDataManager shareInstance] datArray];
    float avg = [[dataArray valueForKeyPath:@"@avg.floatValue"] floatValue];//求平均值
    NSDictionary *dic = @{
        @"均值":@(avg),
        @"count":@(dataArray.count),
        @"list":dataArray
    };
    
    [dic writeToFile:self.filePath atomically:YES];
}

- (void)clear {
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        [self->_datArray removeAllObjects];
        NSFileManager *manager = [NSFileManager defaultManager];
        if ([manager fileExistsAtPath:self->_filePath]) {
            [manager removeItemAtPath:self->_filePath error:nil];
        }
    });
}

- (NSString *)filePath {
    if (!_filePath) {
        NSString *rootPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,
                                                                  NSUserDomainMask, YES) objectAtIndex:0];
        NSString *plistPath = [rootPath stringByAppendingPathComponent:BBASMTimeProfileDataManagerFileName];
        _filePath = plistPath;
    }
    return _filePath;
}

#pragma mark - Pulic Methods
+ (void)addData:(uint64_t)time {
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        [[[self shareInstance] datArray] addObject:@(time)];
        [[self shareInstance] save];
    });
}

+ (void)clear {
    [[self shareInstance] clear];
}
@end
