/*
 * DesignWare UART Driver Implementation (16550 Compatible)
 * Adapted for RISC-V testos - Simplified version without interrupts
 */

#include "dw_uart.h"
#include "types.h"

// MMIO helper functions
static inline uint32_t read_reg(volatile void *addr)
{
#if defined(PLATFORM_QEMU)
    return *(volatile uint8_t *)addr;
#else
    return *(volatile uint32_t *)addr;
#endif
}

static inline void write_reg(uint32_t value, volatile void *addr)
{
#if defined(PLATFORM_QEMU)
    *(volatile uint8_t *)addr = (uint8_t)value;
#else
    *(volatile uint32_t *)addr = value;
#endif
}

// Wait for UART to be idle
static void dw_uart_wait_idle(void)
{
    for (int timeout = 100000; timeout > 0; timeout--) {
        uint32_t lsr = read_reg((void *)DW_UART_LSR);
        
#if defined(PLATFORM_SG2002)
        uint32_t usr = read_reg((void *)DW_UART_USR);
        // Check UART is not busy and transmitter is empty
        if (!(usr & DW_UART_USR_BUSY) && (lsr & DW_UART_LSR_TEMT)) {
            return;
        }
#else
        // Standard 16550A: Just check transmitter empty
        if (lsr & DW_UART_LSR_TEMT) {
            return;
        }
#endif
        
        asm volatile("nop");
    }
}

// Check if TX is ready
static bool dw_uart_tx_ready(void)
{
    return (read_reg((void *)DW_UART_LSR) & DW_UART_LSR_THRE) != 0;
}

// Check if RX has data
static bool dw_uart_rx_ready(void)
{
    return (read_reg((void *)DW_UART_LSR) & DW_UART_LSR_DR) != 0;
}

void dw_uart_early_init(void)
{
    // Wait for UART to be idle before configuration
    dw_uart_wait_idle();

    // Disable all interrupts
    write_reg(0, (void *)DW_UART_IER);

    // Configure baud rate
    // For sg2002: UART_CLOCK = 3686400, BAUDRATE = 115200
    // Divisor = UART_CLOCK / (16 * BAUDRATE) = 3686400 / (16 * 115200) = 2
    uint32_t divisor = UART_CLOCK / (16 * UART_BAUDRATE);
    
    uint32_t lcr = read_reg((void *)DW_UART_LCR);
    write_reg(lcr | DW_UART_LCR_DLAB, (void *)DW_UART_LCR);
    write_reg(divisor & 0xFF, (void *)DW_UART_DLL);
    write_reg((divisor >> 8) & 0xFF, (void *)DW_UART_DLM);
    write_reg(lcr & ~DW_UART_LCR_DLAB, (void *)DW_UART_LCR);

    // 8N1 (8 data bits, no parity, 1 stop bit)
    write_reg(0x3, (void *)DW_UART_LCR);

    // Enable and clear FIFO
    write_reg(DW_UART_FCR_ENABLE_FIFO | DW_UART_FCR_CLEAR_RCVR | DW_UART_FCR_CLEAR_XMIT,
            (void *)DW_UART_FCR);
}

void dw_uart_init(void)
{
    // For now, just call early init
    // Future: add interrupt support
    dw_uart_early_init();
}

void dw_uart_putchar(char c)
{
    // If '\n', send '\r' first
    if (c == '\n') {
        while (!dw_uart_tx_ready())
            asm volatile("nop");
        write_reg('\r', (void *)DW_UART_THR);
    }

    // Wait and send character
    while (!dw_uart_tx_ready())
        asm volatile("nop");
    write_reg(c, (void *)DW_UART_THR);
}

void dw_uart_puts(const char *str)
{
    while (*str) {
        dw_uart_putchar(*str++);
    }
}

char dw_uart_getchar(void)
{
    // Wait for data
    while (!dw_uart_rx_ready()) {
        asm volatile("nop");
    }
    return (char)(read_reg((void *)DW_UART_RBR) & 0xFF);
}

int dw_uart_try_getchar(void)
{
    if (dw_uart_rx_ready()) {
        return (int)(read_reg((void *)DW_UART_RBR) & 0xFF);
    }
    return -1;
}

int dw_uart_gets(char *buffer, size_t buffer_size)
{
    size_t i = 0;
    char c;
    
    while (i < buffer_size - 1) {
        c = dw_uart_getchar();
        
        // Handle backspace
        if (c == '\b' || c == 0x7F) {
            if (i > 0) {
                i--;
                dw_uart_puts("\b \b");
            }
            continue;
        }
        
        // Echo character
        dw_uart_putchar(c);
        
        // Check for newline
        if (c == '\r' || c == '\n') {
            buffer[i] = '\0';
            dw_uart_putchar('\n');
            return i;
        }
        
        buffer[i++] = c;
    }
    
    buffer[i] = '\0';
    return i;
}

bool dw_uart_data_available(void)
{
    return dw_uart_rx_ready();
}

void dw_uart_print_hex(uint64_t value)
{
    const char hex[] = "0123456789ABCDEF";
    char buffer[20];
    int i;
    
    buffer[0] = '0';
    buffer[1] = 'x';
    
    for (i = 15; i >= 0; i--) {
        buffer[17 - i] = hex[(value >> (i * 4)) & 0xF];
    }
    buffer[18] = '\0';
    
    dw_uart_puts(buffer);
}

void dw_uart_print_dec(int64_t value)
{
    char buffer[32];
    char *p = buffer;
    bool negative = false;
    
    if (value < 0) {
        negative = true;
        value = -value;
    }
    
    // Convert digits
    do {
        *p++ = '0' + (value % 10);
        value /= 10;
    } while (value > 0);
    
    if (negative) {
        *p++ = '-';
    }
    
    // Reverse string
    char *start = buffer;
    char *end = p - 1;
    while (start < end) {
        char temp = *start;
        *start = *end;
        *end = temp;
        start++;
        end--;
    }
    *p = '\0';
    
    dw_uart_puts(buffer);
}
