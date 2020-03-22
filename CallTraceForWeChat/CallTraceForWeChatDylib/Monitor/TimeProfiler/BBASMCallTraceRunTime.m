//
//  BBASMCallTraceRunTime.m
//  BBASMCallTraceDylib
//
//  Created by chenronghang on 2020/3/20.
//  Copyright © 2020 com.ceekay1991. All rights reserved.
//

#import "BBASMCallTraceRunTime.h"
#import <objc/runtime.h>
#import <libkern/OSAtomic.h>
static volatile OSSpinLock lock = OS_SPINLOCK_INIT;
@implementation BBASMCallTraceRunTime
+ (void)replaceClass:(Class)origCls
                 sel:(SEL)origSEL
           withClass:(Class)cls
             withSEL:(SEL)sel
       isClassMethod:(BOOL)isClassMethod {
    if ((origCls == NULL) || (origSEL == NULL) || (cls == NULL) || (sel == NULL)) {
        return;
    }
    
    OSSpinLockLock(&lock);
    
    do {
        Method origMethod;
        Method method;
        if (isClassMethod) {
            // 类方法需要使用 object_getClass(obj) 返回类对象中的isa指针，即指向元类对象的指针
            origCls = object_getClass(origCls);
            cls = object_getClass(cls);
            origMethod = class_getClassMethod(origCls, origSEL);
            method = class_getClassMethod(cls, sel);
        } else {
            origMethod = class_getInstanceMethod(origCls, origSEL);
            method = class_getInstanceMethod(cls, sel);
        }
        if ((origMethod == NULL) || (method == NULL)) break;
        BOOL didAddMethod = class_addMethod(origCls, sel, method_getImplementation(method), method_getTypeEncoding(origMethod));
        if (!didAddMethod) {
            // 说明 origCls 存在与 sel 同名的方法，那么选择替换掉方法
            class_replaceMethod(origCls, sel, method_getImplementation(method), method_getTypeEncoding(origMethod));
            // 若方法名相同，替换掉方法后 break
            if (sel == origSEL) break;
        }
        Method exchangeMethod;
        if (isClassMethod) {
            exchangeMethod = class_getClassMethod(origCls, sel);
        } else {
            exchangeMethod = class_getInstanceMethod(origCls, sel);
        }
        method_exchangeImplementations(origMethod, exchangeMethod);
    } while(0);
    
    OSSpinLockUnlock(&lock);
}

+ (void)replaceClass:(Class)origCls
                 sel:(SEL)origSEL
           withClass:(Class)cls
             withSEL:(SEL)sel {
    [self replaceClass:origCls
                   sel:origSEL
             withClass:cls
               withSEL:sel
         isClassMethod:NO];
}
@end
