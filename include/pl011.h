/*
 * PL011 UART Driver Header
 */

#ifndef __PL011_H__
#define __PL011_H__

#include "types.h"
#include "cfg/cfg.h"

// PL011 Register Offsets
#define UART_DR     0x00    // Data Register
#define UART_FR     0x18    // Flag Register
#define UART_IBRD   0x24    // Integer Baud Rate Register
#define UART_FBRD   0x28    // Fractional Baud Rate Register
#define UART_LCR_H  0x2c    // Line Control Register
#define UART_CR     0x30    // Control Register
#define UART_IMSC   0x38    // Interrupt Mask Set Clear Register
#define UART_ICR    0x44    // Interrupt Clear Register

// Flag Register Bits
#define UART_FR_TXFF (1 << 5)  // Transmit FIFO full
#define UART_FR_RXFE (1 << 4)  // Receive FIFO empty
#define UART_FR_BUSY (1 << 3)  // UART busy

// Control Register Bits
#define UART_CR_UARTEN (1 << 0) // UART enable
#define UART_CR_TXE    (1 << 8) // Transmit enable
#define UART_CR_RXE    (1 << 9) // Receive enable

// Line Control Register Bits
#define UART_LCR_H_FEN    (1 << 4) // Enable FIFOs
#define UART_LCR_H_WLEN_8 (3 << 5) // Word length 8 bits

void pl011_init(void);
void pl011_putchar(char c);
char pl011_getchar(void);
void pl011_puts(const char *s);

#endif /* __PL011_H__ */
