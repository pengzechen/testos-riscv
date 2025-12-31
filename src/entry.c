/*
 * RISC-V testos 主入口文件
 * 简化版本，专注于基本功能测试
 */

#include "types.h"
#include "sysreg.h"
#include "cfg/cfg.h"
#include "uart.h"
#include "string.h"
#include "mem.h"
#include "exception.h"
#include "timer.h"
#include "lib/logger.h"

// ===============================================================================
// 系统调用处理函数示例
// ===============================================================================

// SYS_putchar 系统调用
static uint64_t sys_putchar(uint64_t c, uint64_t arg1, uint64_t arg2, 
                           uint64_t arg3, uint64_t arg4, uint64_t arg5)
{
    (void)arg1; (void)arg2; (void)arg3; (void)arg4; (void)arg5;
    uart_putchar((char)c);
    return 0;
}

// SYS_puts 系统调用
static uint64_t sys_puts(uint64_t str_ptr, uint64_t arg1, uint64_t arg2,
                        uint64_t arg3, uint64_t arg4, uint64_t arg5)
{
    (void)arg1; (void)arg2; (void)arg3; (void)arg4; (void)arg5;
    const char *str = (const char *)str_ptr;
    uart_puts(str);
    return strlen(str);
}

// ===============================================================================
// 用户态测试函数（通过系统调用与内核交互）
// ===============================================================================

static void test_syscalls(void)
{
    uart_puts("=== System Call Tests ===\r\n");
    
    // 测试系统调用：这里我们在内核态直接调用，
    // 在真实环境中应该是用户态程序通过 ecall 调用
    uart_puts("Testing sys_putchar: ");
    sys_putchar('H', 0, 0, 0, 0, 0);
    sys_putchar('e', 0, 0, 0, 0, 0);
    sys_putchar('l', 0, 0, 0, 0, 0);
    sys_putchar('l', 0, 0, 0, 0, 0);
    sys_putchar('o', 0, 0, 0, 0, 0);
    uart_putchar('\r');
    uart_putchar('\n');
    
    uart_puts("Testing sys_puts: ");
    sys_puts((uint64_t)"World!\r\n", 0, 0, 0, 0, 0);
    
    uart_puts("System call tests completed.\r\n");
}

// ===============================================================================
// 基本功能测试
// ===============================================================================

static void test_basic_functions(void)
{
    uart_puts("=== Basic Function Tests ===\r\n");
    
    // 测试字符串函数
    uart_puts("Testing string functions:\r\n");
    
    char test_str[] = "Hello, RISC-V!";
    uart_puts("  String: ");
    uart_puts(test_str);
    uart_puts("\r\n  Length: ");
    uart_print_dec(strlen(test_str));
    uart_puts("\r\n");
    
    char buffer[64];
    strcpy(buffer, "Copy test: ");
    strcat(buffer, "OK");
    uart_puts("  ");
    uart_puts(buffer);
    uart_puts("\r\n");
    
    // 测试内存函数
    uart_puts("Testing memory functions:\r\n");
    
    uint8_t mem_test[16];
    memset(mem_test, 0xAA, sizeof(mem_test));
    uart_puts("  memset test: ");
    for (int i = 0; i < 4; i++) {
        uart_print_hex(mem_test[i]);
        uart_putchar(' ');
    }
    uart_puts("...\r\n");
    
    uint8_t mem_src[] = {0x01, 0x02, 0x03, 0x04};
    uint8_t mem_dst[4];
    memcpy(mem_dst, mem_src, 4);
    uart_puts("  memcpy test: ");
    for (int i = 0; i < 4; i++) {
        uart_print_hex(mem_dst[i]);
        uart_putchar(' ');
    }
    uart_puts("\r\n");
    
    uart_puts("Basic function tests completed.\r\n");
}

// ===============================================================================
// 异常测试（可选，用于验证异常处理）
// ===============================================================================

static void test_exception_handling(void)
{
    uart_puts("=== Exception Handling Test ===\r\n");
    uart_puts("Note: These tests may cause system halt.\r\n");
    
    // 测试 1: 非法指令异常（注释掉以避免系统崩溃）
    // uart_puts("Testing illegal instruction...\r\n");
    // asm volatile (".word 0x00000000");  // 非法指令
    
    // 测试 2: 断点异常
    uart_puts("Testing breakpoint exception...\r\n");
    asm volatile ("ebreak");  // 断点指令
    
    uart_puts("Exception handling test completed.\r\n");
}

// ===============================================================================
// 交互式命令处理
// ===============================================================================

static int process_command(const char *cmd)
{
    if (strcmp(cmd, "help") == 0 || strcmp(cmd, "h") == 0) {
        uart_puts("Available commands:\r\n");
        uart_puts("  help, h        - Show this help\r\n");
        uart_puts("  info, i        - Show system information\r\n");
        uart_puts("  mem, m         - Show memory statistics\r\n");
        uart_puts("  test, t        - Run basic tests\r\n");
        uart_puts("  syscall, s     - Test system calls\r\n");
        uart_puts("  exception, e   - Test exception handling\r\n");
        uart_puts("  reboot, r      - Restart system\r\n");
        uart_puts("  quit, q        - Enter idle loop\r\n");
    }
    else if (strcmp(cmd, "info") == 0 || strcmp(cmd, "i") == 0) {
        uart_puts("=== System Information ===\r\n");
        uart_puts("System: RISC-V testos\r\n");
        uart_puts("Version: 1.0\r\n");
        // uart_puts("Hart ID: ");
        // uart_print_hex(CSR_READ(mhartid));
        // uart_puts("\r\n");
        uart_puts("SSTATUS: ");
        uart_print_hex(READ_SSTATUS());
        uart_puts("\r\n");
        uart_puts("STVEC: ");
        uart_print_hex(READ_STVEC());
        uart_puts("\r\n");
    }
    else if (strcmp(cmd, "mem") == 0 || strcmp(cmd, "m") == 0) {
        mem_print_stats();
    }
    else if (strcmp(cmd, "test") == 0 || strcmp(cmd, "t") == 0) {
        test_basic_functions();
    }
    else if (strcmp(cmd, "syscall") == 0 || strcmp(cmd, "s") == 0) {
        test_syscalls();
    }
    else if (strcmp(cmd, "exception") == 0 || strcmp(cmd, "e") == 0) {
        test_exception_handling();
    }
    else if (strcmp(cmd, "reboot") == 0 || strcmp(cmd, "r") == 0) {
        uart_puts("Rebooting system...\r\n");
        // 简单的重启：跳转到启动地址
        void (*reset_func)(void) = (void (*)(void))__LOAD_ADDR__;
        reset_func();
    }
    else if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "q") == 0) {
        uart_puts("Entering idle loop. System will wait for interrupts.\r\n");
        return 0;
    }
    else if (strlen(cmd) > 0) {
        uart_puts("Unknown command: ");
        uart_puts(cmd);
        uart_puts("\r\nType 'help' for available commands.\r\n");
    }

    return 1;
}

// ===============================================================================
// 交互式命令行
// ===============================================================================

static void interactive_shell(void)
{
    char cmd_buffer[128];
    
    uart_puts("\r\n");
    uart_puts("========================================\r\n");
    uart_puts("     RISC-V testos Interactive Shell   \r\n");
    uart_puts("========================================\r\n");
    uart_puts("Type 'help' for available commands.\r\n\r\n");
    
    while (1) {
        uart_puts("testos> ");
        
        // 读取用户输入
        int len = uart_gets(cmd_buffer, sizeof(cmd_buffer));
        
        if (len > 0) {
            // 去除尾部空格
            while (len > 0 && (cmd_buffer[len-1] == ' ' || cmd_buffer[len-1] == '\t')) {
                cmd_buffer[--len] = '\0';
            }
            
            // 处理命令
            int ret = process_command(cmd_buffer);
            if (ret == 0) {
                break;
            }
        }
        
        uart_puts("\r\n");
    }
}

// ===============================================================================
// 内核主函数
// ===============================================================================

void kernel_main(uint64_t hart_id)
{
    // 1. 初始化 UART（早期调试输出）
    // uart_init();
    
    // 2. 输出启动信息
    logger("\n");
    logger_info("==========================================\n");
    logger_info("    RISC-V testos - Simple OS Kernel     \n");
    logger_info("==========================================\n");
    logger_info("Compiled: %s %s\n", __DATE__, __TIME__);
    logger_info("Hart ID: 0x%llx\n", hart_id);
    logger("\n");
    
    // 3. 初始化异常处理系统
    logger_info("Initializing exception handling...\n");
    exception_init();
    
    // 4. 初始化定时器模块
    logger_info("Initializing timer...\n");
    timer_init();
    timer_enable();
    
    // 注册系统调用处理函数
    register_syscall_handler(0, sys_putchar);  // SYS_putchar
    register_syscall_handler(1, sys_puts);     // SYS_puts
    
    // 5. 初始化内存管理
    logger_info("Initializing memory management...\n");
    // mem_init(); // 在启动汇编中已调用
    
    // 6. 运行内存测试
    logger_info("Running memory allocator test...\n");
    mem_test();
    
    // 6. 显示系统准备就绪信息
    logger_info("\nSystem initialization completed!\n");
    logger_info("Supervisor status: 0x%llx\n", READ_SSTATUS());


    interactive_shell();
    
    // 7. 启用中断
    logger_info("Before enabling interrupts - SIE: 0x%llx\n", READ_SIE());
    CSR_SET(sstatus, SSTATUS_SIE);
    logger_info("Global interrupts enabled.\n");
    logger_info("After enabling interrupts - SSTATUS: 0x%llx, SIE: 0x%llx\n", READ_SSTATUS(), READ_SIE());
    
    logger_info("Entering WFI loop...\n");
    
    // 9. 如果从 shell 返回（不应该发生），进入空闲循环
    logger_warn("Kernel main function returned. Entering idle loop.\n");
    while (1) {
        WFI();
    }
}