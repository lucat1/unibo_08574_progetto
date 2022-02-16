/** \file
 * \brief Helper routines and such
 *
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \author Alessandro Frau
 * \date 16-02-2022
 */

#ifndef PANDOS_UTIL_IMPL_H
#define PANDOS_UTIL_IMPL_H

#include "os/types.h"

#ifndef __x86_64__
#define va_arg(varg, type) (type) * ((type *)varg++)
#define va_list int *
#define va_start(varg, fmt) varg = (int *)(&fmt + 1)
#define va_end(varg) /* noop */

/* Serial printing constants */
#define ST_READY 1
#define ST_BUSY 3
#define ST_TRANSMITTED 5
#define CMD_ACK 1
#define CMD_TRANSMIT 2
#define CHAR_OFFSET 8
#define TERM_STATUS_MASK 0xFF
#define term_status(tp) ((tp->transm_status) & TERM_STATUS_MASK)
typedef unsigned int devreg;
#else
#include <stdarg.h>
#endif

int __itoa(void *target, size_t (*writer)(void *, const char *), int i,
           int base);

size_t __printf(void *target, size_t (*writer)(void *, const char *),
                const char *fmt, va_list varg);

/**
 * \brief Utility structure for streaming data to a string target and keeping track
 * of the progress
 */
typedef struct str_target {
    char *str;
    size_t size, wrote;
} str_target_t;

size_t str_writer(void *dest, const char *data);

#ifndef __x86_64__
int term_putchar(termreg_t *term, char c);
size_t serial_writer(void *dest, const char *data);
#endif

#endif /* PANDOS_UTIL_IMPL_H */
