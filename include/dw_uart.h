/*
 * DesignWare UART Driver Header (16550 Compatible)
 * Adapted for RISC-V testos
 */

#ifndef __DW_UART_H__
#define __DW_UART_H__

#include "types.h"
#include "cfg/cfg.h"

// UART register spacing
#if defined(PLATFORM_QEMU)
    #define UART_REG_SHIFT 0
#else
    #define UART_REG_SHIFT 2
#endif

// DWC UART register offsets (8250/16550 compatible)
#define DW_UART_BASE UART_BASE
#define DW_UART_RBR  (DW_UART_BASE + (0x00 << UART_REG_SHIFT))  // Receiver Buffer Register (read)
#define DW_UART_THR  (DW_UART_BASE + (0x00 << UART_REG_SHIFT))  // Transmit Holding Register (write)
#define DW_UART_IER  (DW_UART_BASE + (0x01 << UART_REG_SHIFT))  // Interrupt Enable Register
#define DW_UART_IIR  (DW_UART_BASE + (0x02 << UART_REG_SHIFT))  // Interrupt Identification Register (read)
#define DW_UART_FCR  (DW_UART_BASE + (0x02 << UART_REG_SHIFT))  // FIFO Control Register (write)
#define DW_UART_LCR  (DW_UART_BASE + (0x03 << UART_REG_SHIFT))  // Line Control Register
#define DW_UART_MCR  (DW_UART_BASE + (0x04 << UART_REG_SHIFT))  // Modem Control Register
#define DW_UART_LSR  (DW_UART_BASE + (0x05 << UART_REG_SHIFT))  // Line Status Register
#define DW_UART_MSR  (DW_UART_BASE + (0x06 << UART_REG_SHIFT))  // Modem Status Register
#define DW_UART_SCR  (DW_UART_BASE + (0x07 << UART_REG_SHIFT))  // Scratch Register
#define DW_UART_USR  (DW_UART_BASE + 0x7C)  // UART Status Register (DesignWare specific, usually 4-byte aligned)
#define DW_UART_DLL  (DW_UART_BASE + (0x00 << UART_REG_SHIFT))  // Divisor Latch Low (when DLAB=1)
#define DW_UART_DLM  (DW_UART_BASE + (0x01 << UART_REG_SHIFT))  // Divisor Latch High (when DLAB=1)

// LSR bits
#define DW_UART_LSR_DR   (1 << 0)  // Data Ready
#define DW_UART_LSR_THRE (1 << 5)  // Transmit Holding Register Empty
#define DW_UART_LSR_TEMT (1 << 6)  // Transmitter Empty

// IER bits
#define DW_UART_IER_RDI  (1 << 0)  // Enable Received Data Available Interrupt
#define DW_UART_IER_THRI (1 << 1)  // Enable Transmitter Holding Register Empty Interrupt

// FCR bits
#define DW_UART_FCR_ENABLE_FIFO (1 << 0)
#define DW_UART_FCR_CLEAR_RCVR  (1 << 1)
#define DW_UART_FCR_CLEAR_XMIT  (1 << 2)

// LCR bits
#define DW_UART_LCR_DLAB (1 << 7)

// USR bits (DesignWare specific)
#define DW_UART_USR_BUSY (1 << 0)  // UART Busy

// Early init (no interrupts, for early boot)
void dw_uart_early_init(void);

// Full init (with interrupts) - simplified for now
void dw_uart_init(void);

// Output functions
void dw_uart_putchar(char c);
void dw_uart_puts(const char *str);

// Input functions
char dw_uart_getchar(void);  // Blocking read
int dw_uart_try_getchar(void);  // Non-blocking read (-1 if no data)
int dw_uart_gets(char *buffer, size_t buffer_size);

// Status functions
bool dw_uart_data_available(void);

// Helper functions
void dw_uart_print_hex(uint64_t value);
void dw_uart_print_dec(int64_t value);

#endif  // __DW_UART_H__
