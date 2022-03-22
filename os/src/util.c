#include "os/util.h"
#include "os/util_impl.h"
#include <umps/arch.h>
#include <umps/libumps.h>

static char *end = "", *neg = "-";

int pow(int base, unsigned int exp)
{
    int tmp;
    if (exp == 0)
        return 1;

    tmp = pow(base, exp / 2);
    return (exp % 2 == 0 ? 1 : base) * tmp * tmp;
}

size_t __itoa(void *target, size_t (*writer)(void *, const char *, size_t len),
              int i, int base)
{
    int cpy, digits, wrote, exp;

    for (cpy = i, digits = 0; cpy != 0; ++digits, cpy /= base)
        ;

    wrote = 0;
    if (i < 0) {
        /* Ignore an error here as it'll crop up later either way */
        wrote += writer(target, neg, 1);
        i = -i;
    }

    int wr = 1;
    for (exp = pow(base, digits - 1); digits && wr;
         --digits, i %= exp, exp /= base, wrote += wr) {
        int r = (i / exp); /* remainder */
        char digit[2] = {r > 9 ? 'a' + r - 10 : '0' + r, '\0'};
        wr = writer(target, digit, 1);
    }

    /* always write the string termination char (but don't count it as string
     * length) */
    writer(target, end, 1);
    return wrote;
}

size_t __printf(void *target,
                size_t (*writer)(void *, const char *, size_t len),
                const char *fmt, va_list varg)
{
    size_t wr, last_wrote;

    for (wr = 0; *fmt != '\0'; ++fmt, wr += last_wrote) {
        if (*fmt == '%') {
            ++fmt;
            switch (*fmt) {
                case 's':
                    last_wrote = writer(target, va_arg(varg, char *), 0);
                    break;
                case 'c': {
                    char str[2] = {va_arg(varg, int), '\0'};
                    last_wrote = writer(target, str, 1);
                    break;
                }
                case 'd':
                    last_wrote = __itoa(target, writer, va_arg(varg, int), 10);
                    break;
                case 'p': {
                    memaddr ptr = va_arg(varg, memaddr);
                    if (ptr != (memaddr)NULL) {
                        last_wrote = writer(target, "0x", 2);
                        last_wrote += __itoa(target, writer, (int)ptr, 16);
                    } else
                        last_wrote = writer(target, "(nil)", 5);
                    break;
                }
                case '%': {
                    char *str = "%";
                    last_wrote = writer(target, str, 1);
                    break;
                }
                default: {
                    char str[3] = {*(fmt - 1), *fmt, '\0'};
                    last_wrote = writer(target, str, 2);
                    break;
                }
            }
        } else {
            char str[2] = {*fmt, '\0'};
            last_wrote = writer(target, str, 1);
        }
        if (!last_wrote)
            break;
    }
    /* if we stopped printing because of a writer error, make sure to print the
     * ending null character */
    if (*fmt != '\0')
        wr += writer(target, end, 1);
    return wr;
}

size_t str_writer(void *dest, const char *data, size_t len)
{
    str_target_t *d;
    int i;
    bool check_len;

    d = (str_target_t *)dest;
    i = 0;
    check_len = len != 0;
    /* Make sure we always write the NULL char (in the appropriate location) */
    if (*data == '\0')
        *(d->str + (d->wrote >= d->size - 1 ? d->wrote : d->wrote + 1)) = '\0';
    else
        while (d->wrote + i < d->size - 1 &&
               (*(d->str + d->wrote + i) = data[i]) != '\0' &&
               (!check_len || len--))
            ++i;

    d->wrote += i;
    return i;
}

size_t nitoa(int i, int base, char *dest, size_t len)
{
    str_target_t w = {dest, len, 0};
    return __itoa((void *)&w, str_writer, i, base);
}

size_t pandos_snprintf(char *dest, size_t len, const char *fmt, ...)
{
    str_target_t w = {dest, len, 0};
    va_list varg;
    va_start(varg, fmt);
    size_t res = __printf((void *)&w, str_writer, fmt, varg);
    va_end(varg);
    return res;
}

#ifndef __x86_64__
int term_putchar(termreg_t *term, char c)
{
    devreg stat;

    stat = term_status(term);
    if (stat != ST_READY && stat != ST_TRANSMITTED)
        return 1;

    term->transm_command = ((c << CHAR_OFFSET) | CMD_TRANSMIT);
    while ((stat = term_status(term)) == ST_BUSY)
        ;

    term->transm_command = CMD_ACK;
    return (stat == ST_TRANSMITTED) ? 0 : 2;
}

size_t serial_writer(void *dest, const char *data, size_t len)
{
    char *it;
    termreg_t *term;
    bool check_len;

    for (it = (char *)data, term = (termreg_t *)dest, check_len = len != 0;
         *it != '\0' && (!check_len || len--); ++it)
        if (term_putchar(term, *it))
            break;

    return it - data;
}

/*
 * The following functions have been provided by the tutor (but might have been
 * slightly modified along the way): kputchar, next_line
 */

#define clean_line(l)                                                          \
    for (size_t i = 0; i < MEM_WRITER_LINE_LENGTH; i++)                        \
        mem->buffer[l][i] = ' ';

// Skip to next line
static inline void next_line(memory_target_t *mem)
{
    mem->line = (mem->line + 1) % MEM_WRITER_LINES;
    mem->ch = 0;
    /* Clean out the rest of the line for aesthetic purposes */
    clean_line(mem->line);
}

static void kputchar(memory_target_t *mem, char str)
{
    if (str == '\n')
        next_line(mem);
    else {
        mem->buffer[mem->line][mem->ch] = str;
        if (++mem->ch >= MEM_WRITER_LINE_LENGTH)
            next_line(mem);
    }
}

size_t memory_writer(void *dest, const char *data, size_t len)
{
    char *it;
    memory_target_t *mem;
    bool check_len;

    /* Clean the first line during the first print */
    mem = (memory_target_t *)dest;
    if (mem->line == 0 && mem->ch == 0)
        clean_line(mem->line);

    for (it = (char *)data, check_len = len != 0;
         *it != '\0' && (!check_len || len--); ++it)
        kputchar(mem, *it);

    return it - data;
}

size_t pandos_fprintf(int fd, const char *fmt, ...)
{
    termreg_t *term = (termreg_t *)DEV_REG_ADDR(IL_TERMINAL, fd);
    va_list varg;
    va_start(varg, fmt);
    size_t res = __printf(term, serial_writer, fmt, varg);
    va_end(varg);
    return res;
}

/* The variable to look for when debugging to check out the log */
memory_target_t klog = {.line = 0, .ch = 0};

size_t pandos_kprintf(const char *fmt, ...)
{
    va_list varg;
    va_start(varg, fmt);
    size_t res = __printf(&klog, memory_writer, fmt, varg);
    va_end(varg);
    return res;
}
#endif
