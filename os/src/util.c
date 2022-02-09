#include "os/util.h"
#include <umps/arch.h>
#include <umps/libumps.h>

#ifdef __x86_64__
#include <stdarg.h>
#define varg_arg va_arg
#define varg_type va_list
#define varg_init(varg, fmt) va_start(varg, fmt)
#define varg_end(varg) va_end(varg)
#else
#define varg_arg(varg, type) (type) * ((type *)varg++)
#define varg_type int *
#define varg_init(varg, fmt) varg = (int *)(&fmt + 1)
#define varg_end(varg) /* noop */
#endif

static char *end = "";

// Computes the power of a given intger base to the given *unsigned* intereger
// exponent in log(n) time
int pow(int base, unsigned int exp)
{
    int tmp;
    if (exp == 0)
        return 1;

    tmp = pow(base, exp / 2);
    return (exp % 2 == 0 ? 1 : base) * tmp * tmp;
}

int __itoa(void *target, size_t (*writer)(void *, const char *), int i,
           int base)
{
    int cpy, digits, wrote, exp;

    for (cpy = i, digits = 0; cpy != 0; ++digits, cpy /= base)
        ;

    wrote = 0;
    if (i < 0) {
        char *neg = "-";
        // Ignore an error here as it'll crop up later either way
        wrote += writer(target, neg);
        i = -i;
    }

    int wr = 1;
    for (exp = pow(base, digits - 1); digits && wr;
         --digits, i %= exp, exp /= base, wrote += wr) {
        int r = (i / exp); // remainder
        char digit[2] = {r > 9 ? 'a' + r - 10 : '0' + r, '\0'};
        wr = writer(target, digit);
    }

    // always write the string termination char (but don't count it as str
    // length)
    writer(target, end);
    return wrote;
}

size_t __printf(void *target, size_t (*writer)(void *, const char *),
                const char *fmt, varg_type varg)
{
    size_t wr, last_wrote;

    for (wr = 0; *fmt != '\0'; ++fmt, wr += last_wrote) {
        if (*fmt == '%') {
            ++fmt;
            switch (*fmt) {
                case 's':
                    last_wrote = writer(target, varg_arg(varg, char *));
                    break;
                case 'c': {
                    char str[2] = {varg_arg(varg, int), '\0'};
                    last_wrote = writer(target, str);
                    break;
                }
                case 'd':
                    last_wrote =
                        __itoa(target, writer, varg_arg(varg, int), 10);
                    break;
                case 'p': {
                    memaddr ptr = varg_arg(varg, memaddr);
                    if (ptr == (memaddr)NULL) {
                        char *str = "(null)";
                        last_wrote = writer(target, str);
                    } else {
                        char *str = "0x";
                        last_wrote = writer(target, str);
                        last_wrote += __itoa(target, writer, (int)ptr, 16);
                    }
                    break;
                }
                case '%': {
                    char *str = "%";
                    last_wrote = writer(target, str);
                    break;
                }
                default: {
                    char str[3] = {*(fmt - 1), *fmt, '\0'};
                    last_wrote = writer(target, str);
                    break;
                }
            }
        } else {
            char str[2] = {*fmt, '\0'};
            last_wrote = writer(target, str);
        }
        if (!last_wrote)
            break;
    }
    // if we stpped printing because of a writer error, make sure to print the
    // ending null character
    if (*fmt != '\0')
        wr += writer(target, end);
    return wr;
}

#ifndef PANDOS_TESTING
typedef struct str_writer {
    char *str;
    size_t size, wrote;
} str_writer_t;
#endif

size_t str_writer(void *dest, const char *data)
{
    str_writer_t *d;
    int i;

    d = (str_writer_t *)dest;
    i = 0;
    // Make sure we always write the NULL char (in the approriate location)
    if (*data == '\0')
        *(d->str + (d->wrote >= d->size - 1 ? d->wrote : d->wrote + 1)) = '\0';
    else
        while (d->wrote + i < d->size - 1 &&
               (*(d->str + d->wrote + i) = data[i]) != '\0')
            ++i;

    d->wrote += i;
    return i;
}

int nitoa(int i, int base, char *dest, size_t len)
{
    str_writer_t w = {dest, len, 0};
    return __itoa((void *)&w, str_writer, i, base);
}

int pandos_snprintf(char *dest, size_t len, const char *fmt, ...)
{
    str_writer_t w = {dest, len, 0};
    varg_type varg;
    varg_init(varg, fmt);
    size_t res = __printf((void *)&w, str_writer, fmt, varg);
    varg_end(varg);
    return res;
}

#ifndef __x86_64__
#define ST_READY 1
#define ST_BUSY 3
#define ST_TRANSMITTED 5
#define CMD_ACK 1
#define CMD_TRANSMIT 2
#define CHAR_OFFSET 8
#define TERM_STATUS_MASK 0xFF
#define term_status(tp) ((tp->transm_status) & TERM_STATUS_MASK)
typedef unsigned int devreg;

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

size_t serial_writer(void *dest, const char *data)
{
    char *it;
    termreg_t *term;

    for (term = (termreg_t *)dest, it = (char *)data; *it != '\0'; ++it)
        if (term_putchar(term, *it))
            break;

    return it - data;
}

int pandos_fprintf(int fd, const char *fmt, ...)
{
    termreg_t *term = (termreg_t *)DEV_REG_ADDR(IL_TERMINAL, fd);
    varg_type varg;
    varg_init(varg, fmt);
    size_t res = __printf(term, serial_writer, fmt, varg);
    varg_end(varg);
    return res;
}
#endif
