//
//  BBASMTimeProfilerVC.h
//  BBAMNP
//
//  Created by chenronghang on 2020/2/13.
//  Copyright © 2020 Baidu. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "BBASMCallTrace.h"
NS_ASSUME_NONNULL_BEGIN
extern bool BBASMTimeProfilerVCHasShow;
@interface BBASMTimeProfilerVC : UIViewController
/**
 * @brief 创建显示方法耗时记录的VC
 *
 * @param callRecord 方案耗时记录
 * @return BBASMTimeProfilerVC
 */
- (instancetype)initWithCallRecord:(BBASMThreadCallRecord *)callRecord;
@end

NS_ASSUME_NONNULL_END
