/*
 * RISC-V 系统寄存器操作宏
 */

#ifndef __SYSREG_H__
#define __SYSREG_H__

#include "types.h"

// CSR 读写宏
#define CSR_READ(csr) ({ \
    uint64_t val; \
    asm volatile ("csrr %0, " #csr : "=r"(val)); \
    val; \
})

#define CSR_WRITE(csr, val) \
    asm volatile ("csrw " #csr ", %0" :: "r"(val) : "memory")

#define CSR_SET(csr, val) \
    asm volatile ("csrs " #csr ", %0" :: "r"(val) : "memory")

#define CSR_CLEAR(csr, val) \
    asm volatile ("csrc " #csr ", %0" :: "r"(val) : "memory")

// 常用 CSR 操作
#define READ_MSTATUS()      CSR_READ(mstatus)
#define WRITE_MSTATUS(val)  CSR_WRITE(mstatus, val)
#define READ_MIE()          CSR_READ(mie)
#define WRITE_MIE(val)      CSR_WRITE(mie, val)
#define READ_MTVEC()        CSR_READ(mtvec)
#define WRITE_MTVEC(val)    CSR_WRITE(mtvec, val)
#define READ_MCAUSE()       CSR_READ(mcause)
#define READ_MEPC()         CSR_READ(mepc)
#define WRITE_MEPC(val)     CSR_WRITE(mepc, val)
#define READ_MTVAL()        CSR_READ(mtval)

// Supervisor 模式 CSR
#define READ_SSTATUS()      CSR_READ(sstatus)
#define WRITE_SSTATUS(val)  CSR_WRITE(sstatus, val)
#define READ_SIE()          CSR_READ(sie)
#define WRITE_SIE(val)      CSR_WRITE(sie, val)
#define READ_STVEC()        CSR_READ(stvec)
#define WRITE_STVEC(val)    CSR_WRITE(stvec, val)
#define READ_SCAUSE()       CSR_READ(scause)
#define READ_SEPC()         CSR_READ(sepc)
#define WRITE_SEPC(val)     CSR_WRITE(sepc, val)
#define READ_STVAL()        CSR_READ(stval)

// MSTATUS 寄存器位定义
#define MSTATUS_MIE     (1UL << 3)   // Machine 模式中断使能
#define MSTATUS_MPIE    (1UL << 7)   // 之前的 MIE 值
#define MSTATUS_MPP     (3UL << 11)  // 之前的特权模式
#define MSTATUS_MPP_M   (3UL << 11)  // Machine 模式
#define MSTATUS_MPP_S   (1UL << 11)  // Supervisor 模式
#define MSTATUS_MPP_U   (0UL << 11)  // User 模式

// SSTATUS 寄存器位定义
#define SSTATUS_SIE     (1UL << 1)   // Supervisor 模式中断使能
#define SSTATUS_SPIE    (1UL << 5)   // 之前的 SIE 值
#define SSTATUS_SPP     (1UL << 8)   // 之前的特权模式

// MIE/SIE 中断使能位
#define MIE_MSIE        (1UL << 3)   // Machine 软件中断
#define MIE_MTIE        (1UL << 7)   // Machine 定时器中断
#define MIE_MEIE        (1UL << 11)  // Machine 外部中断
#define SIE_SSIE        (1UL << 1)   // Supervisor 软件中断
#define SIE_STIE        (1UL << 5)   // Supervisor 定时器中断
#define SIE_SEIE        (1UL << 9)   // Supervisor 外部中断

// 异常原因码
#define CAUSE_MISALIGNED_FETCH    0
#define CAUSE_FETCH_ACCESS        1
#define CAUSE_ILLEGAL_INSTRUCTION 2
#define CAUSE_BREAKPOINT          3
#define CAUSE_MISALIGNED_LOAD     4
#define CAUSE_LOAD_ACCESS         5
#define CAUSE_MISALIGNED_STORE    6
#define CAUSE_STORE_ACCESS        7
#define CAUSE_USER_ECALL          8
#define CAUSE_SUPERVISOR_ECALL    9
#define CAUSE_MACHINE_ECALL       11

// 中断原因码 (最高位为1表示中断)
#define INTERRUPT_BIT             (1UL << 63)
#define IRQ_S_SOFT                1
#define IRQ_M_SOFT                3
#define IRQ_S_TIMER               5
#define IRQ_M_TIMER               7
#define IRQ_S_EXT                 9
#define IRQ_M_EXT                 11

// WFI 等待中断指令
#define WFI()   asm volatile ("wfi" ::: "memory")

// 内存屏障
#define FENCE() asm volatile ("fence" ::: "memory")

#endif /* __SYSREG_H__ */