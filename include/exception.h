/*
 * RISC-V testos 异常处理头文件
 */

#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#include "types.h"

// 异常上下文结构体
typedef struct {
    uint64_t x[32];        // 通用寄存器 x0-x31
    uint64_t f[32];        // 浮点寄存器 f0-f31
    uint64_t mepc;         // 异常程序计数器
    uint64_t mcause;       // 异常原因
    uint64_t mtval;        // 异常值
    uint64_t mstatus;      // 机器状态寄存器
    uint64_t fcsr;         // 浮点控制和状态寄存器
} trap_frame_t;

// 异常处理函数类型
typedef void (*exception_handler_t)(trap_frame_t *frame);
typedef uint64_t (*syscall_handler_t)(uint64_t arg0, uint64_t arg1, 
                                      uint64_t arg2, uint64_t arg3,
                                      uint64_t arg4, uint64_t arg5);

// 函数声明
void exception_init(void);
void register_exception_handler(uint64_t cause, exception_handler_t handler);
void register_interrupt_handler(uint64_t cause, exception_handler_t handler);
void register_syscall_handler(uint64_t syscall_num, syscall_handler_t handler);

void handle_exception(trap_frame_t *frame);
void handle_syscall(trap_frame_t *frame);
void print_hex(uint64_t val);

#endif /* __EXCEPTION_H__ */