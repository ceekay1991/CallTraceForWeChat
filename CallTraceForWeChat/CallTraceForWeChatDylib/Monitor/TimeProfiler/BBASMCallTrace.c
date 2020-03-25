//
//  BBASMCallTrace.c
//  BBAMNP
//
//  Created by chenronghang on 2020/2/13.
//  Copyright © 2020 Baidu. All rights reserved.
//
#include "BBASMCallTrace.h"

#ifdef BBASMCallTraceEnable

#include <string.h>
#include <objc/objc.h>
#include <stdlib.h>
#include <dispatch/dispatch.h>
#include <pthread.h>
#include <objc/runtime.h>
#include <sys/time.h>
#include "fishhook.h"

typedef struct {
    Class cls;
    SEL sel;
    uint64_t time;
    bool isMainThread;
} BBASMMethodRecord;

typedef struct {
    BBASMMethodRecord *stack;
    int allocLength;
    int index;
} BBASMThreadMethodStack;

typedef struct {
    int allocLength;
    int index;
    uintptr_t *lr_stack;
} BBASMLRStack;

id (*bbasm_calltrace_orgin_objc_msgSend)(id, SEL, ...);
static pthread_key_t bbasm_calltrace_mainThreadKeyLR;
static pthread_key_t bbasm_calltrace_otherThreadKeyLR;
static BBASMThreadMethodStack *bbasm_calltrace_main_thread_stack = NULL;
static BBASMThreadCallRecord *bbasm_calltrace_main_thread_call_record = NULL;
static BBASMThreadMethodStack *bbasm_calltrace_other_thread_stack = NULL;
static BBASMThreadCallRecord *bbasm_calltrace_other_thread_call_record = NULL;
static bool bbasm_calltrace_callRecordEnable = true;
static int bbasm_calltrace_maxDepth = 30;
static int bbasm_calltrace_ignore_main_call_num = 0;
static int bbasm_calltrace_ignore_other_call_num = 0;
static uint64_t bbasm_calltrace_costMinTime = 1000;
static uint64_t bbasm_calltrace_startTime = 0;
static uint64_t bbasm_calltrace_endTime = 0;
static const char *bbasm_calltrace_monitorThreadName = NULL;
static Class *bbasm_calltrace_class_ignore = NULL;
static uint64_t bbasm_calltrace_ignore_class_count = 0;


void bbasm_calltrace_setIgnoreClass(Class *cls,uint64_t count) {
    bbasm_calltrace_class_ignore = cls;
    bbasm_calltrace_ignore_class_count = count;
}


void bbasm_calltrace_setMonitorThreadName(const char * name) {
    bbasm_calltrace_monitorThreadName = name;
}

static inline bool bbasm_calltrace_matchMonitorThread() {
    bool isMain = pthread_main_np() != 0;
    if (isMain) {
        return true;
    } else {
        if (!bbasm_calltrace_monitorThreadName) {
            return false;
        }
        char name[256];
        name[0] = '\0';
        pthread_getname_np(pthread_self(), name, sizeof name);
        //strcmp 相等返回0
        return !strcmp(name, bbasm_calltrace_monitorThreadName);
    }
    
}

static inline uint64_t bbasm_calltrace_getVirtualCallTime() {
    struct timeval now;
    gettimeofday(&now, NULL);
    uint64_t time = (now.tv_sec % 1000) * 1000000 + now.tv_usec;
    return time;
}

static inline void bbasm_calltrace_pushCallRecord(Class cls, SEL sel, bool isMainThread) {
     BBASMThreadMethodStack *thread_stack = NULL;
    if (isMainThread) {
        thread_stack = bbasm_calltrace_main_thread_stack;
    } else {
       thread_stack = bbasm_calltrace_other_thread_stack;
    }
    if (thread_stack) {
        if (thread_stack->index >= bbasm_calltrace_maxDepth) {
            if (isMainThread) {
                bbasm_calltrace_ignore_main_call_num++;
            } else {
                bbasm_calltrace_ignore_other_call_num++;
            }
           
            return;
        }
        uint64_t time = bbasm_calltrace_getVirtualCallTime();
        if (++thread_stack->index >= thread_stack->allocLength) {
            thread_stack->allocLength += 128;
            thread_stack->stack = (BBASMMethodRecord *)realloc(thread_stack->stack, thread_stack->allocLength *  sizeof(BBASMMethodRecord));
        }
        BBASMMethodRecord *record = &thread_stack->stack[thread_stack->index];
        record->cls = cls;
        record->sel = sel;
        record->time = time;
        record->isMainThread = isMainThread;
        
    }
}



static inline void bbasm_calltrace_popCallRecord() {
    BBASMThreadMethodStack *call_stack = NULL;
    BBASMThreadCallRecord *call_record = NULL;
    if (pthread_main_np()) {
        if (bbasm_calltrace_ignore_main_call_num > 0) {
            bbasm_calltrace_ignore_main_call_num--;
            return;
        }
        call_record = bbasm_calltrace_main_thread_call_record;
        call_stack = bbasm_calltrace_main_thread_stack;
    } else {
        if (bbasm_calltrace_ignore_other_call_num > 0) {
            bbasm_calltrace_ignore_other_call_num--;
            return;
        }
        call_record = bbasm_calltrace_other_thread_call_record;
        call_stack = bbasm_calltrace_other_thread_stack;
    }
    
    
    if (call_stack && call_stack->index >= 0) {
        //todo: stack空间缩小算法
        uint64_t time = bbasm_calltrace_getVirtualCallTime();
        BBASMMethodRecord *record = &call_stack->stack[call_stack->index];
        uint64_t costTime = time - record->time;
        int depth = call_stack->index--;
        if (costTime >= bbasm_calltrace_costMinTime) {
            if (++call_record->index >= call_record->allocLength) {
                call_record->allocLength += 128;
                call_record->record = realloc(call_record->record, call_record->allocLength * sizeof(BBASMCallRecord));
            }
            BBASMCallRecord* callRecord = &call_record->record[call_record->index];
            callRecord->cls = record->cls;
            callRecord->depth = depth;
            callRecord->costTime = costTime;
            callRecord->sel = record->sel;
            callRecord->isMainThread = record->isMainThread;
        }
    }
    
}

static inline void bbasm_calltrace_setLRRegisterValue(uintptr_t lr) {
     bool isMain = pthread_main_np() != 0;
    BBASMLRStack *lrStack = NULL;
    if (isMain) {
        lrStack = pthread_getspecific(bbasm_calltrace_mainThreadKeyLR);
    } else {
       lrStack = pthread_getspecific(bbasm_calltrace_otherThreadKeyLR);
    }
    
    if (!lrStack) {
        lrStack = (BBASMLRStack *)malloc(sizeof(BBASMLRStack));
        lrStack->allocLength = 128;
        lrStack->lr_stack = (uintptr_t *)malloc(lrStack->allocLength * sizeof(uintptr_t));
        lrStack->index = -1;
        if (isMain) {
        pthread_setspecific(bbasm_calltrace_mainThreadKeyLR, lrStack);
        } else {
            pthread_setspecific(bbasm_calltrace_otherThreadKeyLR, lrStack);
        }
        
    }
    if (++lrStack->index >= lrStack->allocLength) {
        lrStack->allocLength += 128;
        lrStack->lr_stack = (uintptr_t *)realloc(lrStack->lr_stack, lrStack->allocLength *sizeof(uintptr_t));
    }
    lrStack->lr_stack[lrStack->index] = lr;
}

static inline uintptr_t bbasm_calltrace_getLRRegisterValue() {
    bool isMain = pthread_main_np() != 0;
    BBASMLRStack *lrStack = NULL;
    if (isMain) {
       lrStack = pthread_getspecific(bbasm_calltrace_mainThreadKeyLR);
    } else {
       lrStack = pthread_getspecific(bbasm_calltrace_otherThreadKeyLR);
    }
    uintptr_t lr = lrStack->lr_stack[lrStack->index--];
    return lr;
}


void bbasm_calltrace_hook_objc_msgSend_before(id self, SEL sel, uintptr_t lr) {
    if (bbasm_calltrace_callRecordEnable && bbasm_calltrace_matchMonitorThread()) {
        Class cls = object_getClass(self);
        bool isMain = pthread_main_np() != 0;
        bbasm_calltrace_pushCallRecord(cls, sel, isMain);
    }
    bbasm_calltrace_setLRRegisterValue(lr);
}

uintptr_t bbasm_calltrace_hook_objc_msgSend_after() {
    if (bbasm_calltrace_callRecordEnable && bbasm_calltrace_matchMonitorThread()) {
        bbasm_calltrace_popCallRecord();
    }
    
    return bbasm_calltrace_getLRRegisterValue();
}

void bbasm_calltrace_threadCleanLRStack(void *ptr) {
    if (ptr != NULL) {
        BBASMLRStack *lrStack = (BBASMLRStack *)ptr;
        if (lrStack->lr_stack) {
            free(lrStack->lr_stack);
        }
        free(lrStack);
    }
}

BBASMThreadMethodStack * bbasm_calltrace_init_thread_stack() {
    BBASMThreadMethodStack *thread_stack = (BBASMThreadMethodStack *)malloc(sizeof(BBASMThreadMethodStack));
    thread_stack->allocLength = 128;
    thread_stack->stack = (BBASMMethodRecord *)malloc(thread_stack->allocLength * sizeof(BBASMMethodRecord));
    thread_stack->index = -1;
    return thread_stack;
}

BBASMThreadCallRecord * bbasm_calltrace_init_call_record() {
    BBASMThreadCallRecord *call_record = (BBASMThreadCallRecord *)malloc(sizeof(BBASMThreadCallRecord));
    call_record->allocLength = 128;
    call_record->record = (BBASMCallRecord *)malloc(call_record->allocLength * sizeof(BBASMCallRecord));
    call_record->index = -1;
    return call_record;
}

void bbasm_calltrace_initData() {
    if (!bbasm_calltrace_main_thread_call_record) {
        bbasm_calltrace_main_thread_call_record = bbasm_calltrace_init_call_record();
    }
    if (!bbasm_calltrace_main_thread_stack) {
         bbasm_calltrace_main_thread_stack = bbasm_calltrace_init_thread_stack();
    }
    
    if (!bbasm_calltrace_other_thread_call_record) {
        bbasm_calltrace_other_thread_call_record = bbasm_calltrace_init_call_record();
    }
    if (!bbasm_calltrace_other_thread_stack) {
         bbasm_calltrace_other_thread_stack = bbasm_calltrace_init_thread_stack();
    }
}


extern void bbasm_calltrace_fake_objc_msgSend_safe(void);
void bbasm_calltrace_startTrace() {
    bbasm_calltrace_clearTrace();
    bbasm_calltrace_startTime = bbasm_calltrace_getVirtualCallTime();
    bbasm_calltrace_initData();
    bbasm_calltrace_callRecordEnable = YES;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        //保存2份LR寄存器是因为线程退出时会调用bbasm_calltrace_threadCleanLRStack清空内存，防止子线程退出影响主线程
        //这里子线程和主线程使用不同的lr
        pthread_key_create(&bbasm_calltrace_mainThreadKeyLR, bbasm_calltrace_threadCleanLRStack);
        pthread_key_create(&bbasm_calltrace_otherThreadKeyLR, bbasm_calltrace_threadCleanLRStack);
        struct rebinding bbasm_calltrace_rebindObjc_msgSend;
        bbasm_calltrace_rebindObjc_msgSend.name = "objc_msgSend";
        bbasm_calltrace_rebindObjc_msgSend.replacement = bbasm_calltrace_fake_objc_msgSend_safe;
        bbasm_calltrace_rebindObjc_msgSend.replaced = (void *)&bbasm_calltrace_orgin_objc_msgSend;
        struct rebinding rebs[1] = {bbasm_calltrace_rebindObjc_msgSend};
        rebind_symbols(rebs, 1);
    });
};

void bbasm_calltrace_stopTrace() {
    if (bbasm_calltrace_callRecordEnable) {
        bbasm_calltrace_callRecordEnable = NO;
        bbasm_calltrace_endTime = bbasm_calltrace_getVirtualCallTime();
    }
    
}

void bbasm_calltrace_clear_stack(BBASMThreadMethodStack *call_stack) {
    if (call_stack) {
        if (call_stack->stack) {
            free(call_stack->stack);
        }
        free(call_stack);
        call_stack = NULL;
    }
}

void bbasm_calltrace_clear_record(BBASMThreadCallRecord *call_record) {
    if (call_record) {
        if (call_record->record) {
            free(call_record->record);
        }
        free(call_record);
        call_record = NULL;
    }
}

void bbasm_calltrace_clearTrace() {
    if (bbasm_calltrace_main_thread_call_record) {
        if (bbasm_calltrace_main_thread_call_record->record) {
            free(bbasm_calltrace_main_thread_call_record->record);
        }
        free(bbasm_calltrace_main_thread_call_record);
        bbasm_calltrace_main_thread_call_record = NULL;
    }
    if (bbasm_calltrace_main_thread_stack) {
        if (bbasm_calltrace_main_thread_stack->stack) {
            free(bbasm_calltrace_main_thread_stack->stack);
        }
        free(bbasm_calltrace_main_thread_stack);
        bbasm_calltrace_main_thread_stack = NULL;
    }
    
    if (bbasm_calltrace_other_thread_call_record) {
        if (bbasm_calltrace_other_thread_call_record->record) {
            free(bbasm_calltrace_other_thread_call_record->record);
        }
        free(bbasm_calltrace_other_thread_call_record);
        bbasm_calltrace_other_thread_call_record = NULL;
    }
    if (bbasm_calltrace_other_thread_stack) {
        if (bbasm_calltrace_other_thread_stack->stack) {
            free(bbasm_calltrace_other_thread_stack->stack);
        }
        free(bbasm_calltrace_other_thread_stack);
        bbasm_calltrace_other_thread_stack = NULL;
    }

    bbasm_calltrace_startTime = 0;
    bbasm_calltrace_endTime = 0;
    bbasm_calltrace_ignore_main_call_num = 0;
    bbasm_calltrace_ignore_other_call_num = 0;
}



BBASMThreadCallRecord *bbasm_calltrace_getMainThreadCallRecord(void) {
    return bbasm_calltrace_main_thread_call_record;
}

BBASMThreadCallRecord *bbasm_calltrace_getOtherThreadCallRecord(void) {
    return bbasm_calltrace_other_thread_call_record;
}

 uint64_t bbasm_calltrace_totalCostTime(void) {
    return bbasm_calltrace_endTime - bbasm_calltrace_startTime;
}

void bbasm_calltrace_setMaxDepth(int depth) {
    bbasm_calltrace_maxDepth = depth;
}

void bbasm_calltrace_setCostMinTime(uint64_t time) {
    bbasm_calltrace_costMinTime = time;
}
#endif


