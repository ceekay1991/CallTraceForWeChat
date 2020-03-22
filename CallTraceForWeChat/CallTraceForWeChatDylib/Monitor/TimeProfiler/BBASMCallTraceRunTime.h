//
//  BBASMCallTraceRunTime.h
//  BBASMCallTraceDylib
//
//  Created by chenronghang on 2020/3/20.
//  Copyright © 2020 com.ceekay1991. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface BBASMCallTraceRunTime : NSObject
/**
 * @brief MethodSwizzling 实例方法
 *
 * @param origCls hook的类
 * @param origSEL hook的方法
 * @param cls 替换的类
 * @param sel 替换的实例方法
 */
+ (void)replaceClass:(Class)origCls
                 sel:(SEL)origSEL
           withClass:(Class)cls
             withSEL:(SEL)sel;
/**
 * @brief MethodSwizzling 方法
 *
 * @param origCls origCls hook的类
 * @param origSEL hook的方法
 * @param cls 替换的类
 * @param sel 替换的方法
 * @param isClassMethod YES 类方法 NO 实例方法
 */
+ (void)replaceClass:(Class)origCls
                 sel:(SEL)origSEL
           withClass:(Class)cls
             withSEL:(SEL)sel
       isClassMethod:(BOOL)isClassMethod;
@end



NS_ASSUME_NONNULL_END
