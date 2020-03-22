//
//  BBASMCallTraceInterface.h
//  BBAMNPDebugs
//
//  Created by chenronghang on 2020/3/10.
//  Copyright © 2020 Baidu. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "BBASMCallTrace.h"
NS_ASSUME_NONNULL_BEGIN

/**
 * 增加oc接口层，便于切换其它耗时统计工具
 */
@interface BBASMCallTrace : NSObject

/**
 * @brief 开启方法耗时监控
 *
 */
+ (void)startTrace;
/**
* @brief 关闭方法耗时监控
*
*/
+ (void)stopTrace;
/**
* @brief 清理数据
*
*/
+ (void)clearTrace;
/**
 * @brief 获取主线程方法调用耗时记录信息
 *
 * @return BBASMThreadCallRecord
 */
+ (BBASMThreadCallRecord *)getMainThreadCallRecord;
/**
 * @brief 获取非主线程方法调用耗时记录信息
 *
 * @return BBASMThreadCallRecord
 */
+ (BBASMThreadCallRecord *)getOtherThreadCallRecord;
/**
 * @brief startTrace--stopTrace 之间的方法耗时
 *
 * @return uint64_t
 */
+ (uint64_t)totalCostTime;
/**
 * @brief 设置要监听的深度
 *
 * @param depth 默认30层
 */
+ (void)setMaxDepth:(int)depth;
/**
 * @brief 设置要监听的方法最小耗时
 *
 * @param time 默认1s
 */
+ (void)setCostMinTime:(uint64_t)time;

/**
 * @brief 设置需要监听的线程name 不设置默认只监听主线程
 *
 * @param name 线程name
 */
+ (void)setMonitorThreadName:(const char *)name;

/**
 * @brief 设置不监听的类名
 *
 * @param classs class 列表
 */
+ (void)setIgnoreClasss:(NSArray<Class> *)classs;
@end

NS_ASSUME_NONNULL_END
