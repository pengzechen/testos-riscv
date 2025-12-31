# RISC-V testos 启动汇编代码
# 从机器模式启动，设置基本环境，跳转到 C 代码

#include "cfg/cfg.h"

.section .text
.global _start

# 外部函数声明
.extern kernel_main
.extern trap_vector
.extern mem_init

# ===============================================================================
# 程序入口点 - S 模式启动
# ===============================================================================
.section .text._start
.global _start
_start:
    # 禁用所有中断，确保启动过程不被打断
    csrw sie, zero
    csrw sip, zero

    # 设置机器模式的栈指针
    # RISC-V 栈是向下增长的，所以栈顶是最高地址
    la   sp, _stack_top

    # 打印启动信息到 UART (早期调试)
    call early_uart_init
    
    # 设置机器模式异常向量表
    # RISC-V 使用 stvec 寄存器指向异常处理程序
    # 这里使用直接模式（最低两位为 00）
    la   t0, trap_vector           # 加载异常向量表地址
    csrw stvec, t0                 # 写入 stvec 寄存器

    # 清空 BSS 段
    # BSS 段包含未初始化的全局变量，需要清零
    call clear_bss

    # 初始化堆内存系统
    # 为后续的动态内存分配做准备
    call mem_init

    # 跳转到 C 语言的内核主函数
    # a0 寄存器通常用作函数的第一个参数，这里传递 hart id, 前面应该没有改过它，
    # sbi 传过来的值还在 a0 中
    call kernel_main               # 调用 C 语言主函数

    # 如果 kernel_main 返回（不应该发生），进入死循环
halt:
    wfi                            # 等待中断（降低功耗）
    j    halt                      # 无限循环


# ===============================================================================
# 早期 UART 初始化 - 用于启动阶段的调试输出
# ===============================================================================
early_uart_init:
    # QEMU virt 机器的 UART 通常已经初始化完成
    # 这里只需要输出一个简单的启动标识
    
    # UART 基地址
    li   t0, UART_BASE
    
    # 输出 "Boot\r\n" 字符串
    li   t1, 'B'
    sb   t1, 0(t0)                 # 发送字符 'B'
    li   t1, 'o' 
    sb   t1, 0(t0)                 # 发送字符 'o'
    li   t1, 'o'
    sb   t1, 0(t0)                 # 发送字符 'o'
    li   t1, 't'
    sb   t1, 0(t0)                 # 发送字符 't'
    li   t1, '\r'
    sb   t1, 0(t0)                 # 发送回车符
    li   t1, '\n'
    sb   t1, 0(t0)                 # 发送换行符
    
    ret                            # 返回调用者


# ===============================================================================
# 清空 BSS 段函数
# BSS 段包含未初始化的全局和静态变量，需要在程序启动时清零
# ===============================================================================
clear_bss:
    # 获取 BSS 段的起始和结束地址（由链接器提供）
    la   t0, __bss_start           # BSS 段起始地址
    la   t1, __bss_end             # BSS 段结束地址

bss_clear_loop:
    # 检查是否已经清理完毕
    bgeu t0, t1, bss_clear_done    # 如果 t0 >= t1，清理完成

    # 清零当前 8 字节（RISC-V 64位）
    sd   zero, 0(t0)               # 存储 0 到当前地址
    addi t0, t0, 8                 # 移动到下一个 8 字节
    j    bss_clear_loop            # 继续循环

bss_clear_done:
    ret                            # 返回调用者


# ===============================================================================
# 数据段 - 栈空间分配
# ===============================================================================
.section .bss
.align 12                          # 按 4KB 对齐（页对齐，为将来的 MMU 做准备）

# 内核栈空间
# RISC-V 栈向下增长，_stack_bottom 是低地址，_stack_top 是高地址
_stack_bottom:
    .skip STACK_SIZE               # 分配栈空间（8KB）
_stack_top:
    # 栈顶位置，sp 寄存器将指向这里

# 堆空间起始标记（为将来的堆内存管理做准备）
.align 12                          # 页对齐
.global _heap_start
_heap_start: