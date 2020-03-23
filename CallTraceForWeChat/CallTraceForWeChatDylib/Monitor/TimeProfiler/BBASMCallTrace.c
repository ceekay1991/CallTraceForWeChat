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

static id (*bbasm_calltrace_orgin_objc_msgSend)(id, SEL, ...);
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


/*
 * arm64程序调用规则
 * https://juejin.im/post/5d14623ef265da1bb47d7635
 * arm64 objc_msgSend 源码
 * https://opensource.apple.com/source/objc4/objc4-723/runtime/Messengers.subproj/objc-msg-arm64.s.auto.html
 * ARM64汇编学习笔记二(寄存器和函数本质)
 * https://www.jianshu.com/p/cf29fb303bdc
 * objc_msgSend 方法执行的逻辑是：先获取对象对应类的信息，再获取方法的缓存，根据方法的 selector 查找函数指针，经过异常错误处理后，最后跳到对应函数的实现。
 * ARM64经常用到的汇编指令
 *##################################################################################
 * MOV    X1，X0         ;将寄存器X0的值传送到寄存器X1
 * ADD    X0，X1，X2     ;寄存器X1和X2的值相加后传送到X0
 * SUB    X0，X1，X2     ;寄存器X1和X2的值相减后传送到X0

 * AND    X0，X0，#0xF    ; X0的值与0xF相位与后的值传送到X0
 * ORR    X0，X0，#9      ; X0的值与9相位或后的值传送到X0
 * EOR    X0，X0，#0xF    ; X0的值与0xF相异或后的值传送到X0

 * LDR    X5，[X6，#0x08]        ；ld：load; X6寄存器加0x08的和的地址值内的数据传送到X5
 * LDP  x29, x30, [sp, #0x10]    ; ldp :load pair ; 一对寄存器, 从内存读取数据到寄存器

 * STR X0, [SP, #0x8]         ；st:store,str:往内存中写数据（偏移值为正）; X0寄存器的数据传送到SP+0x8地址值指向的存储空间
 * STUR   w0, [x29, #-0x8]   ;往内存中写数据（偏移值为负）
 * STP  x29, x30, [sp, #0x10]    ;store pair，存放一对数据, 入栈指令

 * CBZ  ;比较（Compare），如果结果为零（Zero）就转移（只能跳到后面的指令）
 * CBNZ ;比较，如果结果非零（Non Zero）就转移（只能跳到后面的指令）
 * CMP  ;比较指令，相当于SUBS，影响程序状态寄存器CPSR

 * B   ;跳转指令，可带条件跳转与cmp配合使用
 * BL  ;带返回的跳转指令， 返回地址保存到LR（X30）
 * BLR  ; 带返回的跳转指令，跳转到指令后边跟随寄存器中保存的地址(例：blr    x8 ;跳转到x8保存的地址中去执行)
 * RET   ;子程序返回指令，返回地址默认保存在LR（X30）
 *##################################################################################
 */
#define bbasm_calltrace_call(b, value) \
__asm volatile ("stp x8, x9, [sp, #-16]!\n"); \
__asm volatile ("mov x10, %0\n" :: "r"(value)); \
__asm volatile ("ldp x8, x9, [sp], #16\n"); \
__asm volatile (#b " x10\n");

/*
 * bbasm_calltrace_fake_objc_msgSend_safe
 *  保存寄存器x0 - x8，q0 - q7到栈上，
 *  记录class，selector，startTime，lr
 *  恢复寄存器x0 - x8，q0 - q7，
 *  调用原始objc_msg_send，
 *  保存寄存器x0 - x8，q0 - q7到栈上，
 *  记录endTime，
 *  恢复寄存器x0 - x8，q0 - q7，以及lr
 */
__attribute__((__naked__))
static void bbasm_calltrace_fake_objc_msgSend_safe() {
    //维护CFI(call frame information)，这样就可以看到调用堆栈
//    __asm__ volatile(
//                     ".cfi_def_cfa w29, 16\n"
//                     ".cfi_offset w30, -8\n"
//                     ".cfi_offset w29, -16\n"
//                     "stp    x29, x30, [sp, #-16]!\n"
//                     "mov    x29, sp\n"
//    );
    // backup registers
    __asm__ volatile(
                     // sp 是堆栈寄存器，存放栈的偏移地址，每次都指向栈顶。
                     // 保存 {q0-q7} 偏移地址到 sp 寄存器
                     "stp q6, q7, [sp, #-0x20]!\n"
                     "stp q4, q5, [sp, #-0x20]!\n"
                     "stp q2, q3, [sp, #-0x20]!\n"
                     "stp q0, q1, [sp, #-0x20]!\n"
                     "str x8,  [sp, #-16]!\n"  //arm64标准：sp % 16 必须等于0
                     // 保存 {x0-x8}
                     "stp x6, x7, [sp, #-16]!\n"
                     "stp x4, x5, [sp, #-16]!\n"
                     "stp x2, x3, [sp, #-16]!\n"
                     "stp x0, x1, [sp, #-16]!\n"
                     );
    // prepare args and call func
    __asm volatile (
                    /*
                     hook_objc_msgSend_before(id self, SEL sel, uintptr_t lr)
                     x0=self  x1=sel x2=lr
                     */
                    // 交换参数,调用bbasm_calltrace_hook_objc_msgSend_before
                    "mov x2, lr\n"
                    "bl _bbasm_calltrace_hook_objc_msgSend_before"
                    );
    
    // restore registers
    __asm volatile (
                    // 读取 {x0-x8} 从保存到 sp 栈顶的偏移地址读起
                    "ldp x0, x1, [sp], #16\n"
                    "ldp x2, x3, [sp], #16\n"
                    "ldp x4, x5, [sp], #16\n"
                    "ldp x6, x7, [sp], #16\n"
                    "ldr x8,  [sp], #16\n"
                    // 读取 {q0-q7}
                    "ldp q0, q1, [sp], #0x20\n"
                    "ldp q2, q3, [sp], #0x20\n"
                    "ldp q4, q5, [sp], #0x20\n"
                    "ldp q6, q7, [sp], #0x20\n"
                    );
    // 调用原始 objc_msgSend
    bbasm_calltrace_call(blr, bbasm_calltrace_orgin_objc_msgSend)

    // backup registers
    __asm__ volatile(
                     "stp q6, q7, [sp, #-0x20]!\n"
                     "stp q4, q5, [sp, #-0x20]!\n"
                     "stp q2, q3, [sp, #-0x20]!\n"
                     "stp q0, q1, [sp, #-0x20]!\n"
                     "str x8,  [sp, #-16]!\n"  //arm64标准：sp % 16 必须等于0
                     "stp x6, x7, [sp, #-16]!\n"
                     "stp x4, x5, [sp, #-16]!\n"
                     "stp x2, x3, [sp, #-16]!\n"
                     "stp x0, x1, [sp, #-16]!\n"
                     );
    
    __asm volatile (
                    "bl _bbasm_calltrace_hook_objc_msgSend_after"
                    );
    
    __asm volatile (
                    "mov lr, x0\n"
                    );
    
    // restore registers
    __asm volatile (
                    "ldp x0, x1, [sp], #16\n"
                    "ldp x2, x3, [sp], #16\n"
                    "ldp x4, x5, [sp], #16\n"
                    "ldp x6, x7, [sp], #16\n"
                    "ldr x8,  [sp], #16\n"
                    "ldp q0, q1, [sp], #0x20\n"
                    "ldp q2, q3, [sp], #0x20\n"
                    "ldp q4, q5, [sp], #0x20\n"
                    "ldp q6, q7, [sp], #0x20\n"
                    );
    
    __asm volatile (
//                    "ldp x29, x30, [sp], #16\n"
                    "ret");
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



void bbasm_calltrace_startTrace() {
    bbasm_calltrace_clearTrace();
    bbasm_calltrace_startTime = bbasm_calltrace_getVirtualCallTime();
    bbasm_calltrace_initData();
    bbasm_calltrace_callRecordEnable = YES;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
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


