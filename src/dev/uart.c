/*
 * RISC-V UART 驱动 (Compatibility layer for DW UART)
 */

#include "uart.h"
#include "dw_uart.h"
#include "types.h"

// ===============================================================================
// Legacy compatibility functions
// ===============================================================================

void uart_write(const void *data, size_t len)
{
    const uint8_t *bytes = (const uint8_t *)data;
    for (size_t i = 0; i < len; i++) {
        dw_uart_putchar(bytes[i]);
    }
}

bool uart_tx_ready(void)
{
    // Check LSR THRE bit
    uint32_t lsr = *(volatile uint32_t *)DW_UART_LSR;
    return (lsr & DW_UART_LSR_THRE) != 0;
}

uint8_t uart_get_status(void)
{
    return (uint8_t)(*(volatile uint32_t *)DW_UART_LSR);
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
                    dw_uart_puts("(string)");
                    break;
                case 'd':
                    dw_uart_puts("(decimal)");
                    break;
                case 'x':
                    dw_uart_puts("(hex)");
                    break;
                default:
                    dw_uart_putchar('%');
                    dw_uart_putchar(*p);
                    break;
            }
        } else {
            dw_uart_putchar(*p);
        }
        p++;
    }
}
