/*
 * RISC-V testos 基础类型定义
 */

#ifndef __TYPES_H__
#define __TYPES_H__

#define NULL ((void *)0)

// 基础整数类型定义
typedef unsigned char       uint8_t;
typedef signed char         int8_t;
typedef unsigned short      uint16_t;
typedef signed short        int16_t;
typedef unsigned int        uint32_t;
typedef signed int          int32_t;
typedef unsigned long long  uint64_t;
typedef signed long long    int64_t;

// 指针和大小类型
typedef uint64_t            uintptr_t;
typedef int64_t             intptr_t;
typedef uint64_t            size_t;
typedef int64_t             ssize_t;

// 布尔类型
typedef int                 bool;
#define true                1
#define false               0

// 最大值定义
#define SIZE_MAX            ((size_t)(-1))
#define UINT64_MAX          ((uint64_t)(-1))

// 对齐宏
#define ALIGN_UP(addr, align)   (((addr) + (align) - 1) & ~((align) - 1))
#define ALIGN_DOWN(addr, align) ((addr) & ~((align) - 1))

// 工具宏
#define ARRAY_SIZE(arr)     (sizeof(arr) / sizeof((arr)[0]))
#define offsetof(type, member) ((size_t)&((type *)0)->member)

#endif /* __TYPES_H__ */