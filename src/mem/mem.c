/*
 * RISC-V testos 简单内存管理器
 * 只分配不释放的简单堆分配器
 */

#include "types.h"
#include "cfg/cfg.h"
#include "uart.h"

// ===============================================================================
// 内存管理器状态
// ===============================================================================

// 堆内存信息
typedef struct {
    uintptr_t start;        // 堆起始地址
    uintptr_t end;          // 堆结束地址
    uintptr_t current;      // 当前分配位置
    size_t total_size;      // 总大小
    size_t allocated;       // 已分配大小
    size_t align;           // 对齐要求
} heap_info_t;

static heap_info_t heap;

// 外部符号（由链接器提供）
extern char _heap_start[];

// ===============================================================================
// 内存管理器初始化
// ===============================================================================

void mem_init(void)
{
    // 获取堆起始地址
    heap.start = (uintptr_t)_heap_start;
    
    // 设置堆结束地址（使用可用内存的一部分）
    // 假设我们有 128MB 物理内存，内核占用前面的空间
    // 为堆分配 64MB 空间
    heap.end = MEM_START + MEM_SIZE - (32 * 1024 * 1024);  // 预留 32MB
    
    // 确保堆起始地址有效
    if (heap.start < MEM_START + (4 * 1024 * 1024)) {  // 内核至少占用 4MB
        heap.start = MEM_START + (4 * 1024 * 1024);
    }
    
    // 页对齐
    heap.start = ALIGN_UP(heap.start, 4096);
    heap.end = ALIGN_DOWN(heap.end, 4096);
    
    heap.current = heap.start;
    heap.total_size = heap.end - heap.start;
    heap.allocated = 0;
    heap.align = 8;  // 默认 8 字节对齐
    
    // 输出初始化信息
    uart_puts("Memory heap initialized:\r\n");
    uart_puts("  Start: 0x");
    uart_print_hex(heap.start);
    uart_puts("\r\n  End:   0x");
    uart_print_hex(heap.end);
    uart_puts("\r\n  Size:  ");
    uart_print_dec(heap.total_size / 1024);
    uart_puts(" KB\r\n");
}

// ===============================================================================
// 内存分配函数
// ===============================================================================

void *malloc(size_t size)
{
    if (size == 0) {
        return NULL;
    }
    
    // 对齐大小到指定边界
    size = ALIGN_UP(size, heap.align);
    
    // 检查是否有足够的空间
    if (heap.current + size > heap.end) {
        uart_puts("ERROR: Out of memory!\r\n");
        uart_puts("  Requested: ");
        uart_print_dec(size);
        uart_puts(" bytes\r\n");
        uart_puts("  Available: ");
        uart_print_dec(heap.end - heap.current);
        uart_puts(" bytes\r\n");
        return NULL;
    }
    
    // 分配内存
    void *ptr = (void *)heap.current;
    heap.current += size;
    heap.allocated += size;
    
    return ptr;
}

// ===============================================================================
// 内存分配函数（带清零）
// ===============================================================================

void *calloc(size_t count, size_t size)
{
    size_t total_size = count * size;
    if (total_size / count != size) {
        // 整数溢出检查
        return NULL;
    }
    
    void *ptr = malloc(total_size);
    if (ptr) {
        // 清零内存
        uint8_t *bytes = (uint8_t *)ptr;
        for (size_t i = 0; i < total_size; i++) {
            bytes[i] = 0;
        }
    }
    
    return ptr;
}

// ===============================================================================
// 对齐内存分配
// ===============================================================================

void *aligned_alloc(size_t alignment, size_t size)
{
    if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
        // alignment 必须是 2 的幂
        return NULL;
    }
    
    // 分配额外的空间以确保对齐
    size_t aligned_size = size + alignment - 1;
    void *raw_ptr = malloc(aligned_size);
    if (!raw_ptr) {
        return NULL;
    }
    
    // 计算对齐后的地址
    uintptr_t aligned_addr = ALIGN_UP((uintptr_t)raw_ptr, alignment);
    
    // 注意：这种实现会浪费一些内存，因为我们不能释放前面的部分
    return (void *)aligned_addr;
}

// ===============================================================================
// 内存分配信息查询
// ===============================================================================

size_t mem_get_total_size(void)
{
    return heap.total_size;
}

size_t mem_get_allocated_size(void)
{
    return heap.allocated;
}

size_t mem_get_free_size(void)
{
    return heap.end - heap.current;
}

// ===============================================================================
// 内存统计和调试
// ===============================================================================

void mem_print_stats(void)
{
    uart_puts("=== Memory Statistics ===\r\n");
    uart_puts("Total heap size: ");
    uart_print_dec(heap.total_size);
    uart_puts(" bytes (");
    uart_print_dec(heap.total_size / 1024);
    uart_puts(" KB)\r\n");
    
    uart_puts("Allocated:       ");
    uart_print_dec(heap.allocated);
    uart_puts(" bytes (");
    uart_print_dec(heap.allocated / 1024);
    uart_puts(" KB)\r\n");
    
    uart_puts("Free:            ");
    uart_print_dec(mem_get_free_size());
    uart_puts(" bytes (");
    uart_print_dec(mem_get_free_size() / 1024);
    uart_puts(" KB)\r\n");
    
    uart_puts("Usage:           ");
    uart_print_dec((heap.allocated * 100) / heap.total_size);
    uart_puts("%\r\n");
    
    uart_puts("Current pointer: 0x");
    uart_print_hex(heap.current);
    uart_puts("\r\n");
}

// ===============================================================================
// 虚拟的 free 函数（不实际释放内存）
// ===============================================================================

void free(void *ptr)
{
    // 在这个简单的分配器中，我们不实际释放内存
    // 只是忽略 free 调用，内存会在系统重启时自动回收
    (void)ptr;  // 避免编译器警告
    
    // 在调试模式下可以输出警告
    #ifdef DEBUG_MEM
    uart_puts("WARNING: free() called but memory not actually freed\r\n");
    #endif
}

// ===============================================================================
// 内存操作辅助函数
// ===============================================================================

// 检查地址是否在堆范围内
bool mem_is_heap_addr(void *ptr)
{
    uintptr_t addr = (uintptr_t)ptr;
    return (addr >= heap.start && addr < heap.current);
}

// 获取堆的地址范围
void mem_get_heap_range(uintptr_t *start, uintptr_t *end)
{
    if (start) *start = heap.start;
    if (end) *end = heap.end;
}

// ===============================================================================
// 内存测试函数
// ===============================================================================

void mem_test(void)
{
    uart_puts("=== Memory Allocator Test ===\r\n");
    
    // 测试 1: 基本分配
    uart_puts("Test 1: Basic allocation\r\n");
    void *ptr1 = malloc(1024);
    if (ptr1) {
        uart_puts("  malloc(1024): OK at 0x");
        uart_print_hex((uintptr_t)ptr1);
        uart_puts("\r\n");
    } else {
        uart_puts("  malloc(1024): FAILED\r\n");
    }
    
    // 测试 2: 零大小分配
    uart_puts("Test 2: Zero size allocation\r\n");
    void *ptr2 = malloc(0);
    if (ptr2 == NULL) {
        uart_puts("  malloc(0): OK (returned NULL)\r\n");
    } else {
        uart_puts("  malloc(0): UNEXPECTED (should return NULL)\r\n");
    }
    
    // 测试 3: calloc
    uart_puts("Test 3: calloc\r\n");
    uint32_t *ptr3 = (uint32_t *)calloc(10, sizeof(uint32_t));
    if (ptr3) {
        uart_puts("  calloc(10, 4): OK at 0x");
        uart_print_hex((uintptr_t)ptr3);
        uart_puts("\r\n");
        
        // 检查是否清零
        bool all_zero = true;
        for (int i = 0; i < 10; i++) {
            if (ptr3[i] != 0) {
                all_zero = false;
                break;
            }
        }
        uart_puts("  Memory cleared: ");
        uart_puts(all_zero ? "OK" : "FAILED");
        uart_puts("\r\n");
    } else {
        uart_puts("  calloc(10, 4): FAILED\r\n");
    }
    
    // 测试 4: 对齐分配
    uart_puts("Test 4: Aligned allocation\r\n");
    void *ptr4 = aligned_alloc(64, 100);
    if (ptr4) {
        uart_puts("  aligned_alloc(64, 100): OK at 0x");
        uart_print_hex((uintptr_t)ptr4);
        uart_puts("\r\n");
        
        if (((uintptr_t)ptr4 % 64) == 0) {
            uart_puts("  Alignment check: OK\r\n");
        } else {
            uart_puts("  Alignment check: FAILED\r\n");
        }
    } else {
        uart_puts("  aligned_alloc(64, 100): FAILED\r\n");
    }
    
    // 输出最终统计
    mem_print_stats();
    uart_puts("=========================\r\n");
}