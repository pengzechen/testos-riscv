/*
 * RISC-V testos 配置文件
 */

#ifndef __CFG_H__
#define __CFG_H__

// 内核加载地址 (QEMU virt 机器)
#ifndef __LOAD_ADDR__
#define __LOAD_ADDR__   0x80200000
#endif

// 系统调用入口地址
#ifndef __SYS_ENTER_ADDR__
#define __SYS_ENTER_ADDR__ 0x80204000  
#endif

// 栈大小配置
#define STACK_SIZE      0x2000          // 8KB 栈空间

// UART 配置 (QEMU virt 机器的 16550A UART)
// #define UART_BASE       0x10000000      // QEMU virt UART0 基地址
#define UART_BASE       0x4140000      // sg2002 UART0 基地址
#define UART_CLOCK      3686400         // UART 时钟频率
#define UART_BAUDRATE   115200          // 波特率

// 内存配置
#define MEM_START       0x80000000      // 物理内存起始地址
#define MEM_SIZE        0x8000000       // 128MB 内存大小

// 系统配置
#define MAX_IRQ_NUM     127             // 最大中断号

// CLINT (Core Local Interruptor) 配置
#define CLINT_BASE      0x02000000      // QEMU virt CLINT 基地址
#define CLINT_MTIMECMP  (CLINT_BASE + 0x4000)  // M-mode timer compare register offset
#define CLINT_MTIME     (CLINT_BASE + 0xBFF8)  // M-mode time register offset

// 定时器配置
#define TIMER_FREQ_HZ       10000000UL  // QEMU RISC-V virt 定时器频率 (10MHz)
#define TIMER_TICK_MS       10          // 定时器tick间隔 (10ms)
#define TIMER_FREQUENCY_HZ  (1000 / TIMER_TICK_MS)  // 定时器中断频率 (100Hz)

// 调试开关
#define DEBUG_UART      1               // 启用 UART 调试输出
#define DEBUG_EXCEPTION 1               // 启用异常调试信息
#define DEBUG_TIMER     1               // 启用定时器调试信息

#endif /* __CFG_H__ */