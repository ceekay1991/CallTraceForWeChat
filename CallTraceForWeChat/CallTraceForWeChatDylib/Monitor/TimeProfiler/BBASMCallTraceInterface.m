//
//  BBASMCallTraceInterface.m
//  BBAMNPDebugs
//
//  Created by chenronghang on 2020/3/10.
//  Copyright © 2020 Baidu. All rights reserved.
//

#import "BBASMCallTraceInterface.h"
@implementation BBASMCallTrace
#ifdef BBASMCallTraceEnable
+ (void)startTrace {
    bbasm_calltrace_startTrace();
}

+ (void)stopTrace {
    bbasm_calltrace_stopTrace();
}

+ (void)clearTrace {
    bbasm_calltrace_clearTrace();
}

+ (BBASMThreadCallRecord *)getMainThreadCallRecord {
    return bbasm_calltrace_getMainThreadCallRecord();
}

+ (BBASMThreadCallRecord *)getOtherThreadCallRecord {
    return bbasm_calltrace_getOtherThreadCallRecord();
}

+ (uint64_t)totalCostTime {
    return bbasm_calltrace_totalCostTime();
}

+ (void)setMaxDepth:(int)depth {
    bbasm_calltrace_setMaxDepth(depth);
}

+ (void)setCostMinTime:(uint64_t)time {
    bbasm_calltrace_setCostMinTime(time);
}

//设置需要监听的线程name 不设置默认只监听主线程
+ (void)setMonitorThreadName:(const char *)name {
    bbasm_calltrace_setMonitorThreadName(name);
}

+ (void)setIgnoreClasss:(NSArray<Class> *)classs {
    Class arr[[classs count]];
    int i = 0;
    for (Class cls in classs) {
        arr[i++] = cls;
    }
    bbasm_calltrace_setIgnoreClass(arr, [classs count]);
}

#else
+ (void)startTrace {
    
}

+ (void)stopTrace {
    
}

+ (void)clearTrace {
    
}

+ (BBASMThreadCallRecord *)getThreadCallRecord {
    return NULL;
}

+ (uint64_t)totalCostTime {
    return 0;
}

+ (void)setMaxDepth:(int)depth {
    
}

+ (void)setCostMinTime:(uint64_t)time {
    
}

//设置需要监听的线程name 不设置默认只监听主线程
+ (void)setMonitorThreadName:(const char *)name {
    
}

+ (void)setIgnoreClasss:(NSArray<Class> *)classs {
    
}

#endif
@end
