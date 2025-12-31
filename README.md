# RISC-V testos - 简易操作系统

这是一个基于 RISC-V 架构的简易操作系统，实现了基本的内核功能。

## 功能特性

### ✅ 已实现的功能

1. **启动系统**
   - RISC-V 机器模式启动
   - 栈初始化和 BSS 段清理
   - 异常向量表设置

2. **异常处理框架**
   - 完整的异常/中断处理
   - 系统调用支持 (ecall)
   - 上下文保存和恢复
   - 可扩展的处理函数注册机制

3. **基础输入输出**
   - 16550A UART 驱动
   - 字符/字符串输出
   - 键盘输入支持
   - 交互式命令行

4. **内存管理**
   - 简单的堆分配器 (malloc/calloc)
   - 只分配不释放的设计
   - 内存统计和调试功能

5. **系统调用**
   - SYS_putchar (系统调用号 0)
   - SYS_puts (系统调用号 1)
   - 可扩展的系统调用框架

6. **基础库函数**
   - 字符串处理函数 (strlen, strcpy, strcmp 等)
   - 内存操作函数 (memset, memcpy 等)
   - 简单的格式化输出

### 🚧 待实现的功能

1. **MMU 支持**
   - 页表管理
   - 虚拟内存映射
   - 用户/内核空间隔离

2. **中断控制器**
   - PLIC (Platform-Level Interrupt Controller)
   - 定时器中断
   - 外部中断处理

3. **进程管理**
   - 任务调度
   - 上下文切换
   - 多任务支持

## 构建和运行

### 环境要求

- RISC-V 64位交叉编译工具链
- QEMU RISC-V 模拟器
- Make 构建工具

#### 安装工具链 (Ubuntu/Debian)

```bash
# 安装 RISC-V 工具链
sudo apt install gcc-riscv64-unknown-elf

# 或者使用 Linux 工具链
sudo apt install gcc-riscv64-linux-gnu

# 安装 QEMU
sudo apt install qemu-system-misc
```

### 构建系统

```bash
# 构建内核
make all

# 生成反汇编文件
make disasm

# 查看构建帮助
make help
```

### 运行系统

```bash
# 在 QEMU 中运行
make qemu

# 调试模式运行
make qemu-debug

# 在另一个终端中连接 GDB
make gdb
```

### QEMU 运行参数

系统在以下 QEMU 配置中运行：
- 机器类型：`virt`
- CPU：`rv64`
- 内存：`128M`
- UART：`16550A` (地址 0x10000000)

## 使用方法

系统启动后会显示交互式命令行，支持以下命令：

```
testos> help
Available commands:
  help, h        - Show this help
  info, i        - Show system information
  mem, m         - Show memory statistics
  test, t        - Run basic tests
  syscall, s     - Test system calls
  exception, e   - Test exception handling
  reboot, r      - Restart system
  quit, q        - Enter idle loop
```

### 示例会话

```
testos> info
=== System Information ===
System: RISC-V testos
Version: 1.0
Hart ID: 0x0000000000000000
MSTATUS: 0x0000000000000000
MTVEC: 0x0000000080200XXX

testos> test
=== Basic Function Tests ===
Testing string functions:
  String: Hello, RISC-V!
  Length: 14
  Copy test: OK
Testing memory functions:
  memset test: 0xAA 0xAA 0xAA 0xAA ...
  memcpy test: 0x01 0x02 0x03 0x04

testos> mem
=== Memory Statistics ===
Total heap size: XXXXX bytes (XXX KB)
Allocated: XXX bytes (X KB)
Free: XXXXX bytes (XXX KB)
Usage: X%
```

## 系统架构

### 内存布局

```
0x80000000    +------------------+
              |   OpenSBI        |
0x80200000    +------------------+  <- 内核加载地址
              |   .text          |
              |   .rodata        |
              |   .data          |
              |   .bss           |
              |   栈空间         |
              |   堆空间         |
              +------------------+
              |   可用内存       |
0x88000000    +------------------+
```

### 目录结构

```
testos-riscv/
├── Makefile              # 构建脚本
├── README.md            # 本文件
├── include/             # 头文件
│   ├── cfg/
│   │   └── cfg.h        # 系统配置
│   ├── types.h          # 基础类型定义
│   ├── sysreg.h         # 系统寄存器操作
│   ├── string.h         # 字符串函数
│   ├── uart.h           # UART 驱动
│   └── mem.h            # 内存管理
└── src/                 # 源文件
    ├── boot/
    │   ├── boot.S       # 启动汇编
    │   └── link.lds     # 链接脚本
    ├── exception/
    │   ├── exception.S  # 异常处理汇编
    │   └── exception.c  # 异常处理 C 代码
    ├── lib/
    │   └── string.c     # 字符串库函数
    ├── dev/
    │   └── uart.c       # UART 驱动实现
    ├── mem/
    │   └── mem.c        # 内存管理实现
    └── entry.c          # 内核主函数
```

## 开发指南

### 添加新的系统调用

1. 在 `exception.c` 中定义处理函数
2. 在 `entry.c` 的 `kernel_main` 中注册处理函数

```c
// 定义系统调用处理函数
static uint64_t sys_new_call(uint64_t arg0, ...) {
    // 实现逻辑
    return result;
}

// 注册系统调用
register_syscall_handler(SYSCALL_NUM, (void*)sys_new_call);
```

### 添加新的异常处理

```c
// 定义异常处理函数
void my_exception_handler(trap_frame_t *frame) {
    // 处理异常
}

// 注册异常处理函数
register_exception_handler(EXCEPTION_CAUSE, my_exception_handler);
```

### 调试技巧

1. 使用 `uart_puts()` 和 `uart_print_hex()` 进行调试输出
2. 使用 `make disasm` 查看生成的汇编代码
3. 使用 `make qemu-debug` 和 `make gdb` 进行源码级调试

## 系统限制

1. **单核系统**：目前只支持单个 hart
2. **无 MMU**：运行在物理地址模式
3. **简单内存管理**：只支持分配，不支持释放
4. **无文件系统**：没有存储设备支持
5. **无网络**：没有网络协议栈

## 许可证

本项目基于学习和教育目的开发，代码遵循相应的开源许可证。

## 参考资料

- [RISC-V Instruction Set Manual](https://riscv.org/specifications/)
- [RISC-V Privileged Architecture](https://riscv.org/specifications/privileged-isa/)
- [QEMU RISC-V Documentation](https://www.qemu.org/docs/master/system/target-riscv.html)