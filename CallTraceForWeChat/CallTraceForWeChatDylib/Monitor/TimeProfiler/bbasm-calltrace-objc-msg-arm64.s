//
//  bbasm-calltrace-objc-msg-arm64.s
//  BBAMNP
//
//  Created by chenronghang on 2020/3/24.
//  Copyright © 2020 Baidu. All rights reserved.
//
//  原作者:maniackk
//  github:https://github.com/maniackk/TimeProfiler

#ifdef __arm64__
#include <arm/arch.h>


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

* LDR    X5，[X6，#0x08]        ;ld：load; X6寄存器加0x08的和的地址值内的数据传送到X5
* LDP  x29, x30, [sp, #0x10]    ; ldp :load pair ; 一对寄存器, 从内存读取数据到寄存器

* STR X0, [SP, #0x8]         ;st:store,str:往内存中写数据（偏移值为正）; X0寄存器的数据传送到SP+0x8地址值指向的存储空间
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

* bbasm_calltrace_fake_objc_msgSend_safe 大致逻辑如下
*  复制栈帧
*  保存寄存器x0 - x8，q0 - q7到栈上，
*  记录class，selector，startTime，lr
*  恢复寄存器x0 - x8，q0 - q7，
*  调用原始objc_msg_send，
*  保存寄存器x0 - x8，q0 - q7到栈上，
*  记录endTime，
*  恢复寄存器x0 - x8，q0 - q7，以及lr
*  释放栈帧
*/

.macro BBASM_ENTRY /* name */
    .text
    .align 5
    .private_extern    $0
$0:
.endmacro

.macro BBASM_END_ENTRY /* name */
LExit$0:
.endmacro

//由于显示调用堆栈（复制栈帧）有一定性能消耗，可自行评估。1表示显示调用堆栈；0表示不显示调用堆栈
#define BBASM_SUPPORT_SHOW_CALL_STACK 1

.macro BBASM_BACKUP_REGISTERS
    stp q6, q7, [sp, #-0x20]!
    stp q4, q5, [sp, #-0x20]!
    stp q2, q3, [sp, #-0x20]!
    stp q0, q1, [sp, #-0x20]!
    stp x6, x7, [sp, #-0x10]!
    stp x4, x5, [sp, #-0x10]!
    stp x2, x3, [sp, #-0x10]!
    stp x0, x1, [sp, #-0x10]!
    str x8,  [sp, #-0x10]!
.endmacro

.macro BBASM_RESTORE_REGISTERS
    ldr x8,  [sp], #0x10
    ldp x0, x1, [sp], #0x10
    ldp x2, x3, [sp], #0x10
    ldp x4, x5, [sp], #0x10
    ldp x6, x7, [sp], #0x10
    ldp q0, q1, [sp], #0x20
    ldp q2, q3, [sp], #0x20
    ldp q4, q5, [sp], #0x20
    ldp q6, q7, [sp], #0x20
.endmacro

.macro BBASM_CALL_HOOK_BEFORE
    BBASM_BACKUP_REGISTERS
    mov x2, lr
    bl _bbasm_calltrace_hook_objc_msgSend_before
    BBASM_RESTORE_REGISTERS
.endmacro

.macro BBASM_CALL_HOOK_AFTER
    BBASM_BACKUP_REGISTERS
    bl _bbasm_calltrace_hook_objc_msgSend_after
    mov lr, x0
    BBASM_RESTORE_REGISTERS
.endmacro

.macro BBASM_CALL_ORIGIN_OBJC_MSGSEND
    adrp    x17, _bbasm_calltrace_orgin_objc_msgSend@PAGE
    ldr    x17, [x17, _bbasm_calltrace_orgin_objc_msgSend@PAGEOFF]
    blr x17
.endmacro

.macro BBASM_COPY_STACK_FRAME
#if BBASM_SUPPORT_SHOW_CALL_STACK
    stp x29, x30, [sp, #-0x10]
    mov x17, sp
    sub x17, fp, x17
    sub fp, sp, #0x10
    sub sp, fp, x17
    stp x0, x1, [sp, #-0x10]
    stp x2, x3, [sp, #-0x20]
    mov x0, sp
    add x1, sp, x17
    add x1, x1, #0x10
    mov x3, #0x0
    cmp x3, x17
    b.eq #0x18
    ldr x2, [x1, x3]
    str x2, [x0, x3]
    add x3, x3, #0x8
    cmp x3, x17
    b.lt #-0x10
    ldp x0, x1, [sp, #-0x10]
    ldp x2, x3, [sp, #-0x20]
#endif
.endmacro

.macro BBASM_FREE_STACK_FRAME
#if BBASM_SUPPORT_SHOW_CALL_STACK
    mov sp, fp
    add sp, sp, #0x10
    ldr fp, [fp]
#endif
.endmacro

# todo: 目前是全量复制栈帧，但是其实只需要复制参数传递用到的栈，利用函数签名？等手段，去判断需要复制的栈帧大小
BBASM_ENTRY _bbasm_calltrace_fake_objc_msgSend_safe
    BBASM_COPY_STACK_FRAME
    BBASM_CALL_HOOK_BEFORE
    BBASM_CALL_ORIGIN_OBJC_MSGSEND
    BBASM_CALL_HOOK_AFTER
    BBASM_FREE_STACK_FRAME
    ret
BBASM_END_ENTRY _bbasm_calltrace_fake_objc_msgSend_safe


#endif
