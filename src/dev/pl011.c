/*
 * PL011 UART Driver Implementation
 */

#include "pl011.h"
#include "cfg/cfg.h"

static inline void write32(uintptr_t addr, uint32_t val) {
    *(volatile uint32_t *)addr = val;
}

static inline uint32_t read32(uintptr_t addr) {
    return *(volatile uint32_t *)addr;
}

void pl011_init(void) {
    // Disable UART before configuration
    write32(UART_BASE + UART_CR, 0);

    // Clear pending interrupts
    write32(UART_BASE + UART_ICR, 0x7FF);

    // Set baud rate (assuming UART_CLOCK and UART_BAUDRATE are defined)
    // For simplicity, we might skip complex baud rate calculation if QEMU doesn't care
    // But here is a basic one:
    // baud_div = UART_CLOCK / (16 * UART_BAUDRATE)
    // IBRD = floor(baud_div)
    // FBRD = floor((baud_div - IBRD) * 64 + 0.5)
    
    uint32_t divider = UART_CLOCK / (16 * UART_BAUDRATE);
    uint32_t remainder = UART_CLOCK % (16 * UART_BAUDRATE);
    uint32_t fraction = (8 * remainder) / UART_BAUDRATE;

    write32(UART_BASE + UART_IBRD, divider);
    write32(UART_BASE + UART_FBRD, fraction);

    // Line control: 8-bit, FIFO enabled
    write32(UART_BASE + UART_LCR_H, UART_LCR_H_WLEN_8 | UART_LCR_H_FEN);

    // Enable UART, TX, and RX
    write32(UART_BASE + UART_CR, UART_CR_UARTEN | UART_CR_TXE | UART_CR_RXE);
}

void pl011_putchar(char c) {
    // Wait until transmit FIFO is not full
    while (read32(UART_BASE + UART_FR) & UART_FR_TXFF);
    write32(UART_BASE + UART_DR, c);
}

char pl011_getchar(void) {
    // Wait until receive FIFO is not empty
    while (read32(UART_BASE + UART_FR) & UART_FR_RXFE);
    return (char)(read32(UART_BASE + UART_DR) & 0xFF);
}

void pl011_puts(const char *s) {
    while (*s) {
        if (*s == '\n') pl011_putchar('\r');
        pl011_putchar(*s++);
    }
}
