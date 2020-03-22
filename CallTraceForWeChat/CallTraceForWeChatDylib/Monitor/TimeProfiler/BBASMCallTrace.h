//
//  BBASMCallTrace.h
//  BBAMNP
//
//  Created by chenronghang on 2020/2/13.
//  Copyright © 2020 Baidu. All rights reserved.
//
#import <stdio.h>
#import <objc/objc.h>

#ifndef __arm64__
//只支持arm64
#else
#define BBASMCallTraceEnable
#endif


typedef struct {
    Class cls;
    SEL sel;
    uint64_t costTime; //单位：纳秒（百万分之一秒）
    int depth;
    bool isMainThread;
} BBASMCallRecord;

typedef struct {
    BBASMCallRecord *record;
    int allocLength;
    int index;
} BBASMThreadCallRecord;

#ifdef BBASMCallTraceEnable
void bbasm_calltrace_startTrace(void);
void bbasm_calltrace_stopTrace(void);
void bbasm_calltrace_clearTrace(void);
BBASMThreadCallRecord *bbasm_calltrace_getMainThreadCallRecord(void);
BBASMThreadCallRecord *bbasm_calltrace_getOtherThreadCallRecord(void);
uint64_t bbasm_calltrace_totalCostTime(void);
void bbasm_calltrace_setMaxDepth(int depth);
void bbasm_calltrace_setCostMinTime(uint64_t time);
//设置需要监听的线程name 不设置默认只监听主线程
void bbasm_calltrace_setMonitorThreadName(const char * name);
//设置忽略的类
void bbasm_calltrace_setIgnoreClass(Class *cls,uint64_t count);
#endif
