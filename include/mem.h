/*
 * RISC-V testos 内存管理头文件
 */

#ifndef __MEM_H__
#define __MEM_H__

#include "types.h"

// 内存管理器初始化
void mem_init(void);

// 内存分配函数
void *malloc(size_t size);
void *calloc(size_t count, size_t size);
void *aligned_alloc(size_t alignment, size_t size);
void free(void *ptr);  // 注意：在简单分配器中不实际释放

// 内存信息查询
size_t mem_get_total_size(void);
size_t mem_get_allocated_size(void);
size_t mem_get_free_size(void);

// 调试和测试
void mem_print_stats(void);
void mem_test(void);

// 辅助函数
bool mem_is_heap_addr(void *ptr);
void mem_get_heap_range(uintptr_t *start, uintptr_t *end);

#endif /* __MEM_H__ */