/*
 * RISC-V Logger Implementation
 * Based on testos-reflector logger
 */

#include "lib/logger.h"
#include "string.h"
#include "uart.h"
#include "sysreg.h"

#define BUFSZ 512

// ANSI 颜色代码
#define ANSI_RED    "\x1b[31m"
#define ANSI_YELLOW "\x1b[33m"
#define ANSI_GREEN  "\x1b[32m"
#define ANSI_BLUE   "\x1b[34m"
#define ANSI_RESET  "\x1b[0m"

// 日志级别枚举
typedef enum {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_NORMAL
} log_level_t;

// 日志级别配置结构
typedef struct {
    const char *color;
    bool show_prefix;
} log_config_t;

// 日志级别配置表
static const log_config_t log_configs[] = {
    [LOG_LEVEL_DEBUG]  = {ANSI_BLUE, false},   // 蓝色，无前缀
    [LOG_LEVEL_INFO]   = {ANSI_GREEN, true},   // 绿色，带前缀
    [LOG_LEVEL_WARN]   = {ANSI_YELLOW, true},  // 黄色，带前缀
    [LOG_LEVEL_ERROR]  = {ANSI_RED, true},     // 红色，带前缀
    [LOG_LEVEL_NORMAL] = {NULL, true},         // 无色，带前缀
};

// 简单的printf实现结构
typedef struct pstream {
    char *buffer;
    int remain;
    int added;
} pstream_t;

typedef struct strprops {
    char pad;
    int npad;
    bool alternate;
} strprops_t;

static char digits[16] = "0123456789abcdef";

static void addchar(pstream_t *p, char c)
{
    if (p->remain) {
        *p->buffer++ = c;
        --p->remain;
    }
    ++p->added;
}

static void print_str(pstream_t *p, const char *s, strprops_t props)
{
    const char *s_orig = s;
    int npad = props.npad;

    if (npad > 0) {
        npad -= strlen(s_orig);
        while (npad > 0) {
            addchar(p, props.pad);
            --npad;
        }
    }

    while (*s)
        addchar(p, *s++);

    if (npad < 0) {
        props.pad = ' '; /* ignore '0' flag with '-' flag */
        npad += strlen(s_orig);
        while (npad < 0) {
            addchar(p, props.pad);
            ++npad;
        }
    }
}

static void print_int(pstream_t *ps, long long n, int base, strprops_t props)
{
    char buf[sizeof(long) * 3 + 2], *p = buf;
    int s = 0, i;

    if (n < 0) {
        n = -n;
        s = 1;
    }

    while (n) {
        *p++ = digits[n % base];
        n /= base;
    }

    if (s)
        *p++ = '-';

    if (p == buf)
        *p++ = '0';

    for (i = 0; i < (p - buf) / 2; ++i) {
        char tmp;
        tmp = buf[i];
        buf[i] = p[-1 - i];
        p[-1 - i] = tmp;
    }

    *p = 0;
    print_str(ps, buf, props);
}

static void print_unsigned(pstream_t *ps, unsigned long long n, int base, strprops_t props)
{
    char buf[sizeof(long) * 3 + 3], *p = buf;
    int i;

    while (n) {
        *p++ = digits[n % base];
        n /= base;
    }

    if (p == buf)
        *p++ = '0';
    else if (props.alternate && base == 16) {
        if (props.pad == '0') {
            addchar(ps, '0');
            addchar(ps, 'x');
            if (props.npad > 0)
                props.npad = (props.npad - 2 > 0) ? props.npad - 2 : 0;
        } else {
            *p++ = 'x';
            *p++ = '0';
        }
    }

    for (i = 0; i < (p - buf) / 2; ++i) {
        char tmp;
        tmp = buf[i];
        buf[i] = p[-1 - i];
        p[-1 - i] = tmp;
    }

    *p = 0;
    print_str(ps, buf, props);
}

static int fmtnum(const char **fmt)
{
    const char *f = *fmt;
    int len = 0, num = 0;

    if (*f == '-')
        ++f, ++len;

    while (*f >= '0' && *f <= '9') {
        num = num * 10 + (*f - '0');
        ++f, ++len;
    }

    if (**fmt == '-')
        num = -num;

    *fmt += len;
    return num;
}

int my_vsnprintf(char *buf, int size, const char *fmt, va_list va)
{
    pstream_t s;

    s.buffer = buf;
    s.remain = size - 1;
    s.added = 0;

    while (*fmt) {
        char f = *fmt++;
        int nlong = 0;
        strprops_t props;
        memset(&props, 0, sizeof(props));
        props.pad = ' ';

        if (f != '%') {
            addchar(&s, f);
            continue;
        }

    morefmt:
        f = *fmt++;
        switch (f) {
            case '%':
                addchar(&s, '%');
                break;
            case 'c':
                addchar(&s, va_arg(va, int));
                break;
            case '\0':
                --fmt;
                break;
            case '#':
                props.alternate = true;
                goto morefmt;
            case '0':
                props.pad = '0';
                ++fmt;
                /* fall through */
            case '1' ... '9':
            case '-':
                --fmt;
                props.npad = fmtnum(&fmt);
                goto morefmt;
            case 'l':
                ++nlong;
                goto morefmt;
            case 'd':
                switch (nlong) {
                    case 0:
                        print_int(&s, va_arg(va, int), 10, props);
                        break;
                    case 1:
                        print_int(&s, va_arg(va, long), 10, props);
                        break;
                    default:
                        print_int(&s, va_arg(va, long long), 10, props);
                        break;
                }
                break;
            case 'u':
                switch (nlong) {
                    case 0:
                        print_unsigned(&s, va_arg(va, unsigned), 10, props);
                        break;
                    case 1:
                        print_unsigned(&s, va_arg(va, unsigned long), 10, props);
                        break;
                    default:
                        print_unsigned(&s, va_arg(va, unsigned long long), 10, props);
                        break;
                }
                break;
            case 'x':
                switch (nlong) {
                    case 0:
                        print_unsigned(&s, va_arg(va, unsigned), 16, props);
                        break;
                    case 1:
                        print_unsigned(&s, va_arg(va, unsigned long), 16, props);
                        break;
                    default:
                        print_unsigned(&s, va_arg(va, unsigned long long), 16, props);
                        break;
                }
                break;
            case 'p':
                props.alternate = true;
                print_unsigned(&s, (unsigned long) va_arg(va, void *), 16, props);
                break;
            case 's':
                print_str(&s, va_arg(va, const char *), props);
                break;
            default:
                addchar(&s, f);
                break;
        }
    }
    *s.buffer = 0;
    return s.added;
}

int my_snprintf(char *buf, int size, const char *fmt, ...)
{
    va_list va;
    int r;

    va_start(va, fmt);
    r = my_vsnprintf(buf, size, fmt, va);
    va_end(va);
    return r;
}

int my_vprintf(const char *fmt, va_list va)
{
    char buf[BUFSZ];
    int r;

    r = my_vsnprintf(buf, sizeof(buf), fmt, va);
    uart_puts(buf);
    return r;
}

// 通用日志输出函数
static int logger_output(log_level_t level, const char *fmt, va_list args)
{
    char buf[BUFSZ];
    int r = my_vsnprintf(buf, sizeof(buf), fmt, args);

    const log_config_t *config = &log_configs[level];

    // 输出颜色代码
    if (config->color) {
        uart_puts(config->color);
    }

    // 输出前缀
    if (config->show_prefix) {
        uart_puts("[TESTOS] ");
    }

    // 输出消息内容
    uart_puts(buf);

    // 重置颜色
    if (config->color) {
        uart_puts(ANSI_RESET);
    }

    return r;
}

// Logger API 实现
int logger(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    int r = logger_output(LOG_LEVEL_NORMAL, fmt, va);
    va_end(va);
    return r;
}

int logger_debug(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    int r = logger_output(LOG_LEVEL_DEBUG, fmt, va);
    va_end(va);
    return r;
}

int logger_info(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    int r = logger_output(LOG_LEVEL_INFO, fmt, va);
    va_end(va);
    return r;
}

int logger_warn(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    int r = logger_output(LOG_LEVEL_WARN, fmt, va);
    va_end(va);
    return r;
}

int logger_error(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    int r = logger_output(LOG_LEVEL_ERROR, fmt, va);
    va_end(va);
    return r;
}

// 工具函数
void print_hex_logger(uint64_t val)
{
    logger("0x%llx", val);
}

void dumpmem_as_u64(uint64_t *addr, int nums)
{
    for (int i = 0; i < nums; i += 4) {
        logger("0x%llx, 0x%llx, 0x%llx, 0x%llx\n", 
               addr[i], 
               (i + 1 < nums) ? addr[i + 1] : 0,
               (i + 2 < nums) ? addr[i + 2] : 0,
               (i + 3 < nums) ? addr[i + 3] : 0);
    }
}