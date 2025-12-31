#ifndef __LOGGER_H__
#define __LOGGER_H__

#include "types.h"
#include <stdarg.h>

// Logger API - 类似于 printf 的格式化输出
extern int logger(const char *fmt, ...);
extern int logger_debug(const char *fmt, ...);
extern int logger_info(const char *fmt, ...);
extern int logger_warn(const char *fmt, ...);
extern int logger_error(const char *fmt, ...);

// 底层 printf 实现
extern int my_vprintf(const char *fmt, va_list va);
extern int my_snprintf(char *buf, int size, const char *fmt, ...);
extern int my_vsnprintf(char *buf, int size, const char *fmt, va_list va);

// 工具函数
extern void print_hex_logger(uint64_t val);
extern void dumpmem_as_u64(uint64_t *addr, int nums);

#endif // __LOGGER_H__