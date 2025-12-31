/*
 * RISC-V testos 字符串处理函数
 * 移植自 AArch64 版本，适配 RISC-V
 */

#include "types.h"

// ===============================================================================
// 字符串基础函数
// ===============================================================================

size_t strlen(const char *str)
{
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

char *strcpy(char *dst, const char *src)
{
    char *orig_dst = dst;
    while ((*dst++ = *src++))
        ;
    return orig_dst;
}

char *strncpy(char *dst, const char *src, size_t n)
{
    size_t i;
    for (i = 0; i < n && src[i]; i++)
        dst[i] = src[i];
    for (; i < n; i++)
        dst[i] = '\0';
    return dst;
}

int strcmp(const char *s1, const char *s2)
{
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        if (s1[i] != s2[i] || s1[i] == '\0')
            return (unsigned char)s1[i] - (unsigned char)s2[i];
    }
    return 0;
}

char *strcat(char *dst, const char *src)
{
    strcpy(dst + strlen(dst), src);
    return dst;
}

char *strchr(const char *s, int c)
{
    while (*s) {
        if (*s == c)
            return (char *)s;
        s++;
    }
    return (c == '\0') ? (char *)s : NULL;
}

char *strstr(const char *haystack, const char *needle)
{
    size_t needle_len = strlen(needle);
    if (needle_len == 0)
        return (char *)haystack;
    
    while (*haystack) {
        if (strncmp(haystack, needle, needle_len) == 0)
            return (char *)haystack;
        haystack++;
    }
    return NULL;
}

// ===============================================================================
// 内存操作函数
// ===============================================================================

void *memset(void *s, int c, size_t n)
{
    unsigned char *p = s;
    while (n--)
        *p++ = (unsigned char)c;
    return s;
}

void *memcpy(void *dst, const void *src, size_t n)
{
    unsigned char *d = dst;
    const unsigned char *s = src;
    
    // 简单的字节拷贝，可以后续优化为按字对齐拷贝
    while (n--)
        *d++ = *s++;
        
    return dst;
}

void *memmove(void *dst, const void *src, size_t n)
{
    unsigned char *d = dst;
    const unsigned char *s = src;

    if (d < s) {
        // 向前拷贝
        while (n--)
            *d++ = *s++;
    } else {
        // 向后拷贝
        d += n;
        s += n;
        while (n--)
            *--d = *--s;
    }
    return dst;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
    const unsigned char *p1 = s1;
    const unsigned char *p2 = s2;
    
    while (n--) {
        if (*p1 != *p2)
            return *p1 - *p2;
        p1++;
        p2++;
    }
    return 0;
}

void *memchr(const void *s, int c, size_t n)
{
    const unsigned char *p = s;
    while (n--) {
        if (*p == (unsigned char)c)
            return (void *)p;
        p++;
    }
    return NULL;
}

// ===============================================================================
// 数字转换函数
// ===============================================================================

long atol(const char *str)
{
    long result = 0;
    int sign = 1;
    
    // 跳过空白字符
    while (*str == ' ' || *str == '\t' || *str == '\n')
        str++;
    
    // 处理符号
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }
    
    // 转换数字
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    
    return result * sign;
}

int atoi(const char *str)
{
    return (int)atol(str);
}

// ===============================================================================
// 简单的格式化输出支持函数
// ===============================================================================

// 将整数转换为字符串（十进制）
static char *itoa_decimal(long long value, char *str, int *len)
{
    char *p = str;
    char *start = str;
    int digits = 0;
    
    // 处理负数
    if (value < 0) {
        *p++ = '-';
        value = -value;
        start = p;
    }
    
    // 转换数字
    do {
        *p++ = '0' + (value % 10);
        value /= 10;
        digits++;
    } while (value > 0);
    
    *p = '\0';
    if (len) *len = p - str;
    
    // 反转数字部分
    char *end = p - 1;
    while (start < end) {
        char temp = *start;
        *start = *end;
        *end = temp;
        start++;
        end--;
    }
    
    return str;
}

// 将整数转换为字符串（十六进制）
static char *itoa_hex(unsigned long long value, char *str, int uppercase, int *len)
{
    char *p = str;
    char *start = str;
    const char *digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
    
    *p++ = '0';
    *p++ = uppercase ? 'X' : 'x';
    start = p;
    
    if (value == 0) {
        *p++ = '0';
    } else {
        while (value > 0) {
            *p++ = digits[value & 0xF];
            value >>= 4;
        }
    }
    
    *p = '\0';
    if (len) *len = p - str;
    
    // 反转十六进制数字部分
    char *end = p - 1;
    while (start < end) {
        char temp = *start;
        *start = *end;
        *end = temp;
        start++;
        end--;
    }
    
    return str;
}

// ===============================================================================
// 简单的 sprintf 实现（基础版本）
// ===============================================================================

#include <stdarg.h>

int simple_sprintf(char *buf, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    
    char *p = buf;
    const char *f = fmt;
    
    while (*f) {
        if (*f == '%' && *(f + 1)) {
            f++; // 跳过 '%'
            
            switch (*f) {
                case 'd': {
                    int val = va_arg(args, int);
                    char temp[32];
                    int len;
                    itoa_decimal(val, temp, &len);
                    strcpy(p, temp);
                    p += len;
                    break;
                }
                case 'x': {
                    unsigned int val = va_arg(args, unsigned int);
                    char temp[32];
                    int len;
                    itoa_hex(val, temp, 0, &len);
                    strcpy(p, temp);
                    p += len;
                    break;
                }
                case 'X': {
                    unsigned int val = va_arg(args, unsigned int);
                    char temp[32];
                    int len;
                    itoa_hex(val, temp, 1, &len);
                    strcpy(p, temp);
                    p += len;
                    break;
                }
                case 's': {
                    char *val = va_arg(args, char *);
                    strcpy(p, val);
                    p += strlen(val);
                    break;
                }
                case 'c': {
                    int val = va_arg(args, int);
                    *p++ = (char)val;
                    break;
                }
                case '%': {
                    *p++ = '%';
                    break;
                }
                default: {
                    *p++ = '%';
                    *p++ = *f;
                    break;
                }
            }
        } else {
            *p++ = *f;
        }
        f++;
    }
    
    *p = '\0';
    va_end(args);
    return p - buf;
}