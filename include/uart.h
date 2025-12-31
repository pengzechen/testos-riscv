/*
 * RISC-V UART 驱动头文件
 * Wrapper for DW UART driver
 */

#ifndef __UART_H__
#define __UART_H__

#include "types.h"
#include "cfg/cfg.h"

#if defined(UART_TYPE_DW)
    #include "dw_uart.h"
    #define _uart_init           dw_uart_init
    #define _uart_putchar        dw_uart_putchar
    #define _uart_puts           dw_uart_puts
    #define _uart_getchar        dw_uart_getchar
    #define _uart_try_getchar    dw_uart_try_getchar
    #define _uart_gets           dw_uart_gets
    #define _uart_data_available dw_uart_data_available
    #define _uart_print_hex      dw_uart_print_hex
    #define _uart_print_dec      dw_uart_print_dec
#elif defined(UART_TYPE_PL011)
    #include "pl011.h"
    // PL011 驱动目前只实现了基础功能，这里需要映射
    #define _uart_init           pl011_init
    #define _uart_putchar        pl011_putchar
    #define _uart_puts           pl011_puts
    #define _uart_getchar        pl011_getchar
    // 其他高级功能如果 PL011 没实现，可以暂时映射到空或基础实现
    #define _uart_try_getchar()  (-1)
    #define _uart_gets           generic_uart_gets
    #define _uart_data_available() (false)
    #define _uart_print_hex      generic_uart_print_hex
    #define _uart_print_dec      generic_uart_print_dec
#endif

// Wrapper functions
static inline void uart_init(void) {
    _uart_init();
}

static inline void uart_putchar(char c) {
    _uart_putchar(c);
}

static inline void uart_puts(const char *s) {
    _uart_puts(s);
}

static inline char uart_getchar(void) {
    return _uart_getchar();
}

// 声明通用辅助函数
int generic_uart_gets(char *buffer, size_t buffer_size);
void generic_uart_print_hex(uint64_t value);
void generic_uart_print_dec(int64_t value);

#if defined(UART_TYPE_DW)
static inline int uart_try_getchar(void) {
    return _uart_try_getchar();
}

static inline int uart_gets(char *buffer, size_t buffer_size) {
    return _uart_gets(buffer, buffer_size);
}

static inline bool uart_data_available(void) {
    return _uart_data_available();
}

static inline void uart_print_hex(uint64_t value) {
    _uart_print_hex(value);
}

static inline void uart_print_dec(int64_t value) {
    _uart_print_dec(value);
}
#else
// 对于 PL011 使用通用实现
static inline int uart_try_getchar(void) {
    return -1; 
}

static inline int uart_gets(char *buffer, size_t buffer_size) {
    return generic_uart_gets(buffer, buffer_size);
}

static inline bool uart_data_available(void) {
    return false;
}

static inline void uart_print_hex(uint64_t value) {
    generic_uart_print_hex(value);
}

static inline void uart_print_dec(int64_t value) {
    generic_uart_print_dec(value);
}
#endif

// Legacy functions for compatibility
void uart_write(const void *data, size_t len);
bool uart_tx_ready(void);
uint8_t uart_get_status(void);
void uart_printf(const char *fmt, ...);

#endif /* __UART_H__ */