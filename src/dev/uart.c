/*
 * RISC-V UART 驱动 (Compatibility layer for DW UART)
 */

#include "uart.h"
#include "types.h"

// ===============================================================================
// Generic UART helper functions
// ===============================================================================

void generic_uart_print_hex(uint64_t value)
{
    char hex_chars[] = "0123456789ABCDEF";
    for (int i = 60; i >= 0; i -= 4) {
        uart_putchar(hex_chars[(value >> i) & 0xF]);
    }
}

void generic_uart_print_dec(int64_t value)
{
    if (value == 0) {
        uart_putchar('0');
        return;
    }
    if (value < 0) {
        uart_putchar('-');
        value = -value;
    }
    char buffer[20];
    int i = 0;
    while (value > 0) {
        buffer[i++] = (value % 10) + '0';
        value /= 10;
    }
    while (i > 0) {
        uart_putchar(buffer[--i]);
    }
}

int generic_uart_gets(char *buffer, size_t buffer_size)
{
    size_t i = 0;
    while (i < buffer_size - 1) {
        char c = uart_getchar();
        if (c == '\r' || c == '\n') {
            uart_putchar('\r');
            uart_putchar('\n');
            break;
        }
        if (c == '\b' || c == 127) {
            if (i > 0) {
                i--;
                uart_putchar('\b');
                uart_putchar(' ');
                uart_putchar('\b');
            }
            continue;
        }
        uart_putchar(c);
        buffer[i++] = c;
    }
    buffer[i] = '\0';
    return i;
}

// ===============================================================================
// Legacy compatibility functions
// ===============================================================================

void uart_write(const void *data, size_t len)
{
    const uint8_t *bytes = (const uint8_t *)data;
    for (size_t i = 0; i < len; i++) {
        uart_putchar(bytes[i]);
    }
}

bool uart_tx_ready(void)
{
#if defined(UART_TYPE_DW)
    // Check LSR THRE bit
    uint32_t lsr = *(volatile uint32_t *)DW_UART_LSR;
    return (lsr & DW_UART_LSR_THRE) != 0;
#elif defined(UART_TYPE_PL011)
    return !( *(volatile uint32_t *)(UART_BASE + UART_FR) & UART_FR_TXFF );
#endif
}

uint8_t uart_get_status(void)
{
#if defined(UART_TYPE_DW)
    return (uint8_t)(*(volatile uint32_t *)DW_UART_LSR);
#elif defined(UART_TYPE_PL011)
    return (uint8_t)(*(volatile uint32_t *)(UART_BASE + UART_FR));
#endif
}

void uart_printf(const char *fmt, ...)
{
    // Simplified printf - only supports basic formats
    const char *p = fmt;
    
    while (*p) {
        if (*p == '%' && *(p + 1)) {
            p++;
            switch (*p) {
                case 's':
                    uart_puts("(string)");
                    break;
                case 'd':
                    uart_puts("(decimal)");
                    break;
                case 'x':
                    uart_puts("(hex)");
                    break;
                default:
                    uart_putchar('%');
                    uart_putchar(*p);
            }
        } else {
            uart_putchar(*p);
        }
        p++;
    }
}
