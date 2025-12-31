/*
 * RISC-V 定时器模块实现
 */

#include "timer.h"
#include "lib/logger.h"

// 全局变量
volatile uint64_t g_system_ticks    = 0;
volatile uint64_t g_timer_frequency = TIMER_FREQ_HZ;
volatile uint64_t g_uptime_seconds  = 0;
volatile uint32_t g_tick_counter    = 0;
timer_stats_t     g_timer_stats     = {0};

// 前向声明

// ===============================================================================
// 定时器初始化
// ===============================================================================
void timer_init(void)
{
    logger_info("Timer initialization:\n");
    logger_info("  Timer frequency: %llu Hz\n", g_timer_frequency);
    logger_info("  Target frequency: %d Hz\n", TIMER_FREQUENCY_HZ);
    logger_info("  Tick interval: %d ms\n", TIMER_TICK_MS);

    // 禁用定时器中断
    timer_disable();

    // 清零统计信息
    timer_reset_stats();

    logger_info("Timer initialized successfully\n");
}

// ===============================================================================
// 启用定时器中断
// ===============================================================================
void timer_enable(void)
{
    logger_debug("Starting timer_enable...\n");
    
    // 计算下一次中断的时间
    uint64_t ticks_per_interrupt = g_timer_frequency / TIMER_FREQUENCY_HZ;
    
    logger_info("Timer enabled with %llu ticks per interrupt\n", ticks_per_interrupt);
    
    logger_debug("Setting next interrupt...\n");
    // 首先设置下一次中断时间
    timer_set_next_interrupt(ticks_per_interrupt);
    
    logger_debug("Interrupt time set, enabling MTIE...\n");
    // 然后启用Machine模式定时器中断
    uint64_t sie = READ_SIE();
    sie |= SIE_STIE;  // 启用机器定时器中断
    CSR_WRITE(sie, sie);

    logger_info("Machine timer interrupt enabled\n");
}

// ===============================================================================
// 禁用定时器中断
// ===============================================================================
void timer_disable(void)
{
    // 禁用Machine模式定时器中断
    uint64_t sie = READ_SIE();
    sie &= ~SIE_STIE;  // 禁用机器定时器中断
    CSR_WRITE(sie, sie);

    logger_info("Timer disabled\n");
}

// ===============================================================================
// 设置下一次中断时间
// S-mode 下使用 SBI 调用设置定时器
// ===============================================================================
void timer_set_next_interrupt(uint64_t ticks_from_now)
{
    uint64_t current_time = READ_TIME();
    uint64_t next_time = current_time + ticks_from_now;
    
    // 使用 SBI 调用设置定时器
    // SBI call: sbi_set_timer(uint64_t stime_value)
    // a7 = 0 (EID: Timer Extension)
    // a6 = 0 (FID: sbi_set_timer)
    // a0 = stime_value
    register uint64_t a0 asm("a0") = next_time;
    register uint64_t a6 asm("a6") = 0;  // FID: sbi_set_timer
    register uint64_t a7 asm("a7") = 0;  // EID: Timer Extension
    
    asm volatile(
        "ecall"
        : "+r"(a0)
        : "r"(a6), "r"(a7)
        : "memory"
    );
}

// ===============================================================================
// 调度下一个tick
// ===============================================================================
void timer_schedule_next_tick(void)
{
    uint64_t ticks_per_interrupt = g_timer_frequency / TIMER_FREQUENCY_HZ;
    timer_set_next_interrupt(ticks_per_interrupt);
}

// ===============================================================================
// 定时器中断处理函数
// ===============================================================================
void timer_handler(void *frame)
{
    (void)frame;  // 抑制未使用参数警告

    // 更新系统tick计数
    g_system_ticks++;
    g_tick_counter++;

    // 更新统计信息
    g_timer_stats.total_interrupts++;
    g_timer_stats.last_interrupt_time = timer_get_uptime_ms();

    // 每100个tick（1秒）更新秒数计数器
    if (g_tick_counter >= TIMER_FREQUENCY_HZ) {
        g_uptime_seconds++;
        g_tick_counter = 0;
        g_timer_stats.total_seconds = g_uptime_seconds;

        // 每秒输出一句话
        logger_info("System running - Uptime: %llus\n", g_uptime_seconds);
    }

    // 调度下一个tick
    timer_schedule_next_tick();
}

// ===============================================================================
// 获取系统tick数
// ===============================================================================
uint64_t timer_get_system_ticks(void)
{
    return g_system_ticks;
}

// ===============================================================================
// 获取系统运行时间（毫秒）
// ===============================================================================
uint64_t timer_get_uptime_ms(void)
{
    return g_system_ticks * TIMER_TICK_MS;
}

// ===============================================================================
// 获取系统运行时间（秒）
// ===============================================================================
uint64_t timer_get_uptime_seconds(void)
{
    return g_uptime_seconds;
}

// ===============================================================================
// 获取定时器频率
// ===============================================================================
uint64_t timer_get_frequency(void)
{
    return g_timer_frequency;
}

// ===============================================================================
// 毫秒级延时（忙等待）
// ===============================================================================
void timer_delay_ms(uint32_t ms)
{
    uint64_t start_time = READ_TIME();
    uint64_t delay_ticks = (g_timer_frequency * ms) / 1000;
    uint64_t target_time = start_time + delay_ticks;

    while (READ_TIME() < target_time) {
        asm volatile("nop");
    }
}

// ===============================================================================
// 微秒级延时（忙等待）
// ===============================================================================
void timer_delay_us(uint32_t us)
{
    uint64_t start_time = READ_TIME();
    uint64_t delay_ticks = (g_timer_frequency * us) / 1000000;
    uint64_t target_time = start_time + delay_ticks;

    while (READ_TIME() < target_time) {
        asm volatile("nop");
    }
}

// ===============================================================================
// 获取统计信息
// ===============================================================================
void timer_get_stats(timer_stats_t *stats)
{
    if (stats) {
        *stats = g_timer_stats;
    }
}

// ===============================================================================
// 重置统计信息
// ===============================================================================
void timer_reset_stats(void)
{
    g_timer_stats.total_interrupts = 0;
    g_timer_stats.total_seconds = 0;
    g_timer_stats.last_interrupt_time = 0;
}

// ===============================================================================
// 打印定时器状态信息
// ===============================================================================
void timer_dump_info(void)
{
    logger_info("\n=== Timer Information ===\n");
    logger_info("System Ticks: %llu\n", g_system_ticks);
    logger_info("Uptime: %llu seconds (%llu ms)\n", g_uptime_seconds, timer_get_uptime_ms());
    logger_info("Timer Frequency: %llu Hz\n", g_timer_frequency);
    logger_info("Total Interrupts: %llu\n", g_timer_stats.total_interrupts);
    logger_info("========================\n");
}

