/*
 * RISC-V testos 字符串处理函数头文件
 */

#ifndef __STRING_H__
#define __STRING_H__

#include "types.h"
#include <stdarg.h>

// 字符串函数
size_t strlen(const char *str);
char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, size_t n);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strcat(char *dst, const char *src);
char *strchr(const char *s, int c);
char *strstr(const char *haystack, const char *needle);

// 内存函数
void *memset(void *s, int c, size_t n);
void *memcpy(void *dst, const void *src, size_t n);
void *memmove(void *dst, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
void *memchr(const void *s, int c, size_t n);

// 数字转换函数
long atol(const char *str);
int atoi(const char *str);

// 简单的格式化输出
int simple_sprintf(char *buf, const char *fmt, ...);

#endif /* __STRING_H__ */