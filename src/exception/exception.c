/*
 * RISC-V 异常处理 C 代码
 */

#include "types.h"
#include "sysreg.h"
#include "cfg/cfg.h"
#include "timer.h"
#include "lib/logger.h"
#include "uart.h"


// 异常上下文结构体，与汇编代码中的布局一致
typedef struct
{
    uint64_t x[32];    // 通用寄存器 x0-x31 (x0 不使用)
    uint64_t f[32];    // 浮点寄存器 f0-f31 (64位，支持单精度和双精度)
    uint64_t sepc;     // 异常程序计数器 (S-mode)
    uint64_t scause;   // 异常原因 (S-mode)
    uint64_t stval;    // 异常值 (S-mode)
    uint64_t sstatus;  // Supervisor 状态寄存器
    uint64_t fcsr;     // 浮点控制和状态寄存器
} trap_frame_t;

// 异常处理函数指针类型
typedef void (*exception_handler_t)(trap_frame_t *frame);

// 异常处理函数数组
static exception_handler_t exception_handlers[16];
static exception_handler_t interrupt_handlers[16];

// 系统调用处理函数指针类型
typedef uint64_t (*syscall_handler_t)(uint64_t arg0,
                                      uint64_t arg1,
                                      uint64_t arg2,
                                      uint64_t arg3,
                                      uint64_t arg4,
                                      uint64_t arg5);

// 系统调用处理函数数组
static syscall_handler_t syscall_handlers[256];

// 前向声明
void
print_hex(uint64_t val);

// 内部函数声明
static void
default_exception_handler(trap_frame_t *frame);
static void
default_interrupt_handler(trap_frame_t *frame);
static void
ebreak_handler(trap_frame_t *frame);
static uint64_t
default_syscall_handler(uint64_t arg0,
                        uint64_t arg1,
                        uint64_t arg2,
                        uint64_t arg3,
                        uint64_t arg4,
                        uint64_t arg5);

// 异常/中断处理函数注册声明
void
register_exception_handler(uint64_t cause, exception_handler_t handler);
void
register_interrupt_handler(uint64_t cause, exception_handler_t handler);
void
register_syscall_handler(uint64_t syscall_num, syscall_handler_t handler);

// ===============================================================================
// 异常处理初始化
// ===============================================================================
void
exception_init(void)
{
    // 初始化异常处理函数为默认处理函数
    for (int i = 0; i < 16; i++) {
        exception_handlers[i] = default_exception_handler;
        interrupt_handlers[i] = default_interrupt_handler;
    }

    // 初始化系统调用处理函数为默认处理函数
    for (int i = 0; i < 256; i++) {
        syscall_handlers[i] = default_syscall_handler;
    }
    
    // 注册ebreak异常处理函数
    register_exception_handler(CAUSE_BREAKPOINT, ebreak_handler);
    
    // 初始化 sscratch 为当前栈指针，用于异常处理时的栈切换
    // 在 S-mode 运行时，sscratch 保存内核栈指针
    uint64_t current_sp;
    asm volatile("mv %0, sp" : "=r"(current_sp));
    CSR_WRITE(sscratch, current_sp);
}

// ===============================================================================
// 注册异常处理函数
// ===============================================================================
void
register_exception_handler(uint64_t cause, exception_handler_t handler)
{
    if (cause < 16) {
        exception_handlers[cause] = handler;
    }
}

// ===============================================================================
// 注册中断处理函数
// ===============================================================================
void
register_interrupt_handler(uint64_t cause, exception_handler_t handler)
{
    if (cause < 16) {
        interrupt_handlers[cause] = handler;
    }
}

// ===============================================================================
// 注册系统调用处理函数
// ===============================================================================
void
register_syscall_handler(uint64_t syscall_num, syscall_handler_t handler)
{
    if (syscall_num < 256) {
        syscall_handlers[syscall_num] = handler;
    }
}

// 调试：输出异常信息
static uint64_t trap_count = 0;

// ===============================================================================
// 主异常处理函数 - 从汇编代码调用
// ===============================================================================
void
handle_exception(trap_frame_t *frame)
{
    uint64_t cause = frame->scause;

    trap_count++;

    logger_debug("trap count: %d\n", trap_count);

    if (cause & INTERRUPT_BIT) {
        // 处理中断
        uint64_t interrupt_cause = cause & ~INTERRUPT_BIT;
        if (interrupt_cause < 16) {
            interrupt_handlers[interrupt_cause](frame);
        } else {
            default_interrupt_handler(frame);
        }
    } else {
        // 处理异常
        if (cause < 16) {
            exception_handlers[cause](frame);
        } else {
            default_exception_handler(frame);
        }
    }
}

// ===============================================================================
// 系统调用处理函数 - 从汇编代码调用
// ===============================================================================
void
handle_syscall(trap_frame_t *frame)
{
    // 系统调用号在 a7 寄存器中
    uint64_t syscall_num = frame->x[17];  // a7 = x17

    // 系统调用参数在 a0-a6 寄存器中
    uint64_t arg0 = frame->x[10];  // a0 = x10
    uint64_t arg1 = frame->x[11];  // a1 = x11
    uint64_t arg2 = frame->x[12];  // a2 = x12
    uint64_t arg3 = frame->x[13];  // a3 = x13
    uint64_t arg4 = frame->x[14];  // a4 = x14
    uint64_t arg5 = frame->x[15];  // a5 = x15

    // 调用相应的系统调用处理函数
    uint64_t ret_val;
    if (syscall_num < 256 && syscall_handlers[syscall_num]) {
        ret_val = syscall_handlers[syscall_num](arg0, arg1, arg2, arg3, arg4, arg5);
    } else {
        ret_val = default_syscall_handler(arg0, arg1, arg2, arg3, arg4, arg5);
    }

    // 将返回值放入 a0 寄存器
    frame->x[10] = ret_val;  // a0 = x10

    // 更新 sepc 以跳过 ecall 指令（4 字节）
    frame->sepc += 4;
}

// ===============================================================================
// 默认异常处理函数
// ===============================================================================
static void
default_exception_handler(trap_frame_t *frame)
{
    logger_error("*** EXCEPTION ***\n");
    logger_error("Cause: 0x%llx\n", frame->scause);
    logger_error("PC: 0x%llx\n", frame->sepc);
    logger_error("Value: 0x%llx\n", frame->stval);
    logger_error("Status: 0x%llx\n", frame->sstatus);

    // 简单的异常类型识别
    switch (frame->scause) {
        case CAUSE_ILLEGAL_INSTRUCTION:
            logger_error("Illegal instruction\n");
            break;
        case CAUSE_BREAKPOINT:
            logger_error("Breakpoint\n");
            break;
        case CAUSE_MISALIGNED_LOAD:
            logger_error("Misaligned load\n");
            break;
        case CAUSE_MISALIGNED_STORE:
            logger_error("Misaligned store\n");
            break;
        case CAUSE_LOAD_ACCESS:
            logger_error("Load access fault\n");
            break;
        case CAUSE_STORE_ACCESS:
            logger_error("Store access fault\n");
            break;
        default:
            logger_error("Unknown exception\n");
            break;
    }

    // 进入死循环或重启
    logger_error("System halted.\n");
    while (1) {
        WFI();
    }
}

// ===============================================================================
// 默认中断处理函数
// ===============================================================================
static void
default_interrupt_handler(trap_frame_t *frame)
{
    uint64_t interrupt_cause = frame->scause & ~INTERRUPT_BIT;

    // 对于定时器中断，直接调用处理函数，不输出额外信息
    if (interrupt_cause == IRQ_S_TIMER) {
        timer_handler(frame);
        return;
    }

    // 其他中断打印调试信息
    logger_warn("*** INTERRUPT: 0x%llx ***\n", interrupt_cause);

    switch (interrupt_cause) {
        case IRQ_S_EXT:
            logger_warn("Supervisor external interrupt\n");
            break;
        case IRQ_S_SOFT:
            logger_warn("Supervisor software interrupt\n");
            break;
        default:
            logger_warn("Unknown interrupt\n");
            break;
    }
}

// ===============================================================================
// 直接系统调用处理函数 (从 Gateway 跳转而来)
// ===============================================================================
uint64_t
handle_syscall_direct_c(uint64_t syscall_id,
                        uint64_t arg1,
                        uint64_t arg2,
                        uint64_t arg3,
                        uint64_t arg4,
                        uint64_t arg5)
{
    // 打印调试信息
    logger_info("[GATEWAY] Syscall ID: %lld, args: 0x%llx, 0x%llx, 0x%llx\n", 
                syscall_id, arg1, arg2, arg3);

    // 处理 Linux 标准系统调用号 (musl 使用)
    switch (syscall_id) {
        case 64: // sys_write(fd, buf, count)
        {
            const char *buf = (const char *)arg1;
            size_t count = (size_t)arg2;
            for (size_t i = 0; i < count; i++) {
                uart_putchar(buf[i]);
            }
            return count;
        }
        case 93: // sys_exit(code)
            logger_info("User program exited with code %lld\n", arg1);
            while(1); // 简单挂起
            return 0;
        default:
            // 尝试调用原有的处理逻辑
            return default_syscall_handler(syscall_id, arg1, arg2, arg3, arg4, arg5);
    }
}

// ===============================================================================
// 默认系统调用处理函数
// ===============================================================================
static uint64_t
default_syscall_handler(uint64_t arg0,
                        uint64_t arg1,
                        uint64_t arg2,
                        uint64_t arg3,
                        uint64_t arg4,
                        uint64_t arg5)
{
    // 简单的系统调用示例
    (void) arg1;
    (void) arg2;
    (void) arg3;
    (void) arg4;
    (void) arg5;

    // arg0 作为系统调用号
    switch (arg0) {
        case 0:  // SYS_putchar - 保持兼容性，继续使用uart
            logger("%c", (char)arg1);
            return 0;
        case 1:  // SYS_puts - 保持兼容性，继续使用uart
            logger("%s", (const char*)arg1);
            return 0;
        default:
            logger_warn("Unknown syscall: 0x%llx\n", arg0);
            return -1;
    }
}

// ===============================================================================
// 辅助函数：打印十六进制数
// ===============================================================================
// 兼容性函数：打印十六进制数，现在使用logger
void
print_hex(uint64_t val)
{
    logger("0x%llx", val);
}

// ===============================================================================
// ebreak 异常处理函数
// ===============================================================================
static void
ebreak_handler(trap_frame_t *frame)
{
    logger_info("=== BREAKPOINT HIT ===\n");
    logger_info("Breakpoint reached at PC: 0x%llx\n", frame->sepc);
    logger_info("Continuing execution...\n");
    
    // 跳过 ebreak 指令（4 字节）
    frame->sepc += 4;
}