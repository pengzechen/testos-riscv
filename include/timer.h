/*
 * RISC-V 定时器模块头文件
 */

#ifndef __TIMER_H__
#define __TIMER_H__

#include "types.h"
#include "sysreg.h"
#include "cfg/cfg.h"

// RISC-V 定时器相关 CSR
#define READ_TIME()         CSR_READ(time)
#define READ_MCYCLE()       CSR_READ(mcycle)
#define READ_MINSTRET()     CSR_READ(minstret)

// 定时器频率配置 (QEMU RISC-V virt 机器通常是 10MHz)
#define TIMER_FREQ_HZ       10000000UL      // 10MHz
#define TIMER_TICK_MS       10              // 10ms 时钟周期
#define TIMER_FREQUENCY_HZ  (1000 / TIMER_TICK_MS)  // 100Hz

// 全局变量
extern volatile uint64_t g_system_ticks;     // 系统tick计数
extern volatile uint64_t g_timer_frequency;  // 定时器频率
extern volatile uint64_t g_uptime_seconds;   // 系统运行时间（秒）
extern volatile uint32_t g_tick_counter;     // tick计数器，用于计算秒

// 统计信息结构体
typedef struct {
    uint64_t total_interrupts;     // 总中断次数
    uint64_t total_seconds;        // 总运行秒数
    uint64_t last_interrupt_time;  // 上次中断时间
} timer_stats_t;

extern timer_stats_t g_timer_stats;

// 函数声明

/**
 * 初始化定时器模块
 */
void timer_init(void);

/**
 * 启用定时器中断
 */
void timer_enable(void);

/**
 * 禁用定时器中断
 */
void timer_disable(void);

/**
 * 定时器中断处理函数
 * @param frame 异常上下文帧
 */
void timer_handler(void *frame);

/**
 * 设置下一次中断时间
 * @param ticks_from_now 从现在开始多少个tick后中断
 */
void timer_set_next_interrupt(uint64_t ticks_from_now);

/**
 * 调度下一个tick中断
 */
void timer_schedule_next_tick(void);

// 时间相关函数

/**
 * 获取系统tick数
 * @return 当前tick数
 */
uint64_t timer_get_system_ticks(void);

/**
 * 获取系统运行时间（毫秒）
 * @return 运行时间（毫秒）
 */
uint64_t timer_get_uptime_ms(void);

/**
 * 获取系统运行时间（秒）
 * @return 运行时间（秒）
 */
uint64_t timer_get_uptime_seconds(void);

/**
 * 获取定时器频率
 * @return 定时器频率（Hz）
 */
uint64_t timer_get_frequency(void);

/**
 * 毫秒级延时（忙等待）
 * @param ms 延时毫秒数
 */
void timer_delay_ms(uint32_t ms);

/**
 * 微秒级延时（忙等待）
 * @param us 延时微秒数
 */
void timer_delay_us(uint32_t us);

// 统计相关函数

/**
 * 获取定时器统计信息
 * @param stats 输出统计信息的结构体指针
 */
void timer_get_stats(timer_stats_t *stats);

/**
 * 重置统计信息
 */
void timer_reset_stats(void);

/**
 * 打印定时器状态信息
 */
void timer_dump_info(void);

#endif /* __TIMER_H__ */