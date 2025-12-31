/*
 * RISC-V UART 驱动头文件
 * Wrapper for DW UART driver
 */

#ifndef __UART_H__
#define __UART_H__

#include "types.h"
#include "dw_uart.h"

// Wrapper functions - redirect to DW UART
static inline void uart_init(void) {
    dw_uart_init();
}

static inline void uart_putchar(char c) {
    dw_uart_putchar(c);
}

static inline void uart_puts(const char *s) {
    dw_uart_puts(s);
}

static inline char uart_getchar(void) {
    return dw_uart_getchar();
}

static inline int uart_try_getchar(void) {
    return dw_uart_try_getchar();
}

static inline int uart_gets(char *buffer, size_t buffer_size) {
    return dw_uart_gets(buffer, buffer_size);
}

static inline bool uart_data_available(void) {
    return dw_uart_data_available();
}

static inline void uart_print_hex(uint64_t value) {
    dw_uart_print_hex(value);
}

static inline void uart_print_dec(int64_t value) {
    dw_uart_print_dec(value);
}

// Legacy functions for compatibility
void uart_write(const void *data, size_t len);
bool uart_tx_ready(void);
uint8_t uart_get_status(void);
void uart_printf(const char *fmt, ...);

#endif /* __UART_H__ */