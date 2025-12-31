# RISC-V 异常处理汇编代码 - S-mode 版本
# 实现异常向量表和上下文保存/恢复

# 定义常量
# TRAP_FRAME_SIZE: 36 个 64 位寄存器 = 36 * 8 = 288 字节
.equ TRAP_FRAME_SIZE, 288

# 异常原因码定义
.equ CAUSE_SUPERVISOR_ECALL, 9

.section .text

# 外部函数声明
.extern handle_exception
.extern handle_syscall

# ===============================================================================
# 系统调用网关 (Syscall Gateway)
# 放置在固定内存区域，供用户态程序直接跳转
# ===============================================================================
.section .syscall_gateway, "ax"
.align 4
.global syscall_gateway
syscall_gateway:
    # 每个条目 8 字节 (2条指令)
    # 我们这里实现一个通用的入口，或者根据 a7 分发
    # 为了简单且符合用户要求 "根据 syscall id 跳转"，我们做一个跳转表
    
    # 假设支持 256 个系统调用
    .rept 256
    j handle_syscall_direct
    nop                         # 确保每个条目占用 8 字节 (id * 8)
    .endr

# 通用的直接系统调用处理入口
# 注意：这里是在 S-mode 直接跳转过来的，没有经过 trap_vector
# 所以我们需要手动保存必要的寄存器
handle_syscall_direct:
    addi sp, sp, -16
    sd   ra, 0(sp)
    
    # 调用 C 语言处理函数
    # a7 已经是 syscall id, a0-a5 是参数
    call handle_syscall_direct_c
    
    ld   ra, 0(sp)
    addi sp, sp, 16
    ret

# ===============================================================================
# 异常向量表入口
# RISC-V 在异常/中断发生时会跳转到这里
# ===============================================================================
.align 4
.global trap_vector
trap_vector:
    # 简化：直接跳转到异常处理程序
    j trap_handler

# ===============================================================================
# 异常/中断处理程序 - S-mode 版本
# ===============================================================================
.align 4
trap_handler:
    # 简化版本：因为我们没有用户态，直接在当前栈上保存上下文
    # 在栈上分配空间保存寄存器
    addi sp, sp, -TRAP_FRAME_SIZE

save_registers:
    # 保存所有通用寄存器到栈上
    sd   x1,  1*8(sp)              # ra
    sd   x2,  2*8(sp)              # sp (保存调整前的值)
    sd   x3,  3*8(sp)              # gp
    sd   x4,  4*8(sp)              # tp
    sd   x5,  5*8(sp)              # t0
    sd   x6,  6*8(sp)              # t1
    sd   x7,  7*8(sp)              # t2
    sd   x8,  8*8(sp)              # s0/fp
    sd   x9,  9*8(sp)              # s1
    sd   x10, 10*8(sp)             # a0
    sd   x11, 11*8(sp)             # a1
    sd   x12, 12*8(sp)             # a2
    sd   x13, 13*8(sp)             # a3
    sd   x14, 14*8(sp)             # a4
    sd   x15, 15*8(sp)             # a5
    sd   x16, 16*8(sp)             # a6
    sd   x17, 17*8(sp)             # a7
    sd   x18, 18*8(sp)             # s2
    sd   x19, 19*8(sp)             # s3
    sd   x20, 20*8(sp)             # s4
    sd   x21, 21*8(sp)             # s5
    sd   x22, 22*8(sp)             # s6
    sd   x23, 23*8(sp)             # s7
    sd   x24, 24*8(sp)             # s8
    sd   x25, 25*8(sp)             # s9
    sd   x26, 26*8(sp)             # s10
    sd   x27, 27*8(sp)             # s11
    sd   x28, 28*8(sp)             # t3
    sd   x29, 29*8(sp)             # t4
    sd   x30, 30*8(sp)             # t5
    sd   x31, 31*8(sp)             # t6

    # 修正保存的 sp 值（因为我们已经调整了 TRAP_FRAME_SIZE）
    addi t0, sp, TRAP_FRAME_SIZE   # 计算原始 sp
    sd   t0, 2*8(sp)               # 保存原始 sp

    # 保存异常相关的 CSR 寄存器
    csrr t0, sepc
    sd   t0, 32*8(sp)              # 保存 sepc
    
    csrr t0, scause
    sd   t0, 33*8(sp)              # 保存 scause
    
    csrr t0, stval
    sd   t0, 34*8(sp)              # 保存 stval
    
    csrr t0, sstatus
    sd   t0, 35*8(sp)              # 保存 sstatus

handle_trap:
    # 调用 C 语言异常处理函数
    mv   a0, sp                    # 传递 trap_frame 指针
    
    # 检查是否为系统调用
    csrr t0, scause
    li   t1, CAUSE_SUPERVISOR_ECALL
    beq  t0, t1, syscall_entry
    
    # 普通异常/中断处理
    call handle_exception
    j    restore_registers

syscall_entry:
    # 系统调用处理
    call handle_syscall

restore_registers:
    # 恢复异常相关寄存器
    ld   t0, 32*8(sp)              # 加载 sepc
    csrw sepc, t0
    
    ld   t0, 35*8(sp)              # 加载 sstatus
    csrw sstatus, t0

    # 恢复通用寄存器
    ld   x1,  1*8(sp)              # ra
    # x2(sp) 最后恢复
    ld   x3,  3*8(sp)              # gp
    ld   x4,  4*8(sp)              # tp
    ld   x5,  5*8(sp)              # t0
    ld   x6,  6*8(sp)              # t1
    ld   x7,  7*8(sp)              # t2
    ld   x8,  8*8(sp)              # s0/fp
    ld   x9,  9*8(sp)              # s1
    ld   x10, 10*8(sp)             # a0
    ld   x11, 11*8(sp)             # a1
    ld   x12, 12*8(sp)             # a2
    ld   x13, 13*8(sp)             # a3
    ld   x14, 14*8(sp)             # a4
    ld   x15, 15*8(sp)             # a5
    ld   x16, 16*8(sp)             # a6
    ld   x17, 17*8(sp)             # a7
    ld   x18, 18*8(sp)             # s2
    ld   x19, 19*8(sp)             # s3
    ld   x20, 20*8(sp)             # s4
    ld   x21, 21*8(sp)             # s5
    ld   x22, 22*8(sp)             # s6
    ld   x23, 23*8(sp)             # s7
    ld   x24, 24*8(sp)             # s8
    ld   x25, 25*8(sp)             # s9
    ld   x26, 26*8(sp)             # s10
    ld   x27, 27*8(sp)             # s11
    ld   x28, 28*8(sp)             # t3
    ld   x29, 29*8(sp)             # t4
    ld   x30, 30*8(sp)             # t5
    ld   x31, 31*8(sp)             # t6

    # 最后恢复 sp 并返回
    ld   x2,  2*8(sp)              # 恢复 sp
    
    sret
