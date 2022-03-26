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
#include "umps/libumps.h"

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

/**
 * \brief Prints the string representation of a given integer on a given target.
 * \param[out] target The buffer on which the string representation is printed.
 * \param[in] writer The utility procedure used to print the string.
 * \param[in] i The integer whose representation is to be computed.
 * \param[in] base The positional base of the representation.
 * \return The length of the string representation actually computed.
 */
size_t __itoa(void *target, size_t (*writer)(void *, const char *, size_t len),
              int i, int base);

/**
 * \brief Prints formatted text on a given target.
 * \param[out] target The buffer on which the string representation is printed.
 * \param[in] writer The utility procedure used to print the string.
 * \param[in] fmt The format string to be printed.
 * \param[in] varg Additional parameters for the format string.
 * \return The number of characters actually printed.
 */
size_t __printf(void *target,
                size_t (*writer)(void *, const char *, size_t len),
                const char *fmt, va_list varg);

/**
 * \brief Utility structure for streaming data to a string target and keeping
 * track of the progress.
 */
typedef struct str_target {
    char *str;    /** Actual character buffer. */
    size_t size;  /** Size of the character buffer. */
    size_t wrote; /** Number of characters actually written on the buffer. */
} str_target_t;

/**
 * \brief Prints a string to the given `str_target`.
 * Respects the size constraints attached with the given `str_target`.
 * If `len` is zero, it is ignored. If a terminator character is encountered
 * before `len` character have been written, the copy process halts.
 * \param[out] dest The str_target to which the string is to be printed.
 * \param[in] data The string to be printed.
 * \param[in] len The length of the string to be printed.
 * \return Returns the number of written characters.
 */
size_t str_writer(void *dest, const char *data, size_t len);

#ifndef __x86_64__

/**
 * \brief Prints a single character to the given terminal.
 * \param[out] term The terminal to be printed on.
 * \param[in] c The character to be printed.
 * \return Returns 0 if the character was printed correctly, 1 if the
 * operation failed because the terminal was in an incorrect state, 2 if any
 * other kind of failure happened during the operation.
 */
static int term_putchar(termreg_t *term, char c);

/**
 * \brief Prints a string to the given serial terminal.
 * If `len` is zero, it is ignored. If a terminator character is encountered
 * before `len` character have been written, the copy process halts.
 * \param[out] dest The serial terminal to which the string is to be
 * printed. \param[in] data The string to be printed. \param[in] len The
 * length of the string to be printed. \return Returns the number of written
 * characters.
 */
static size_t serial_writer(void *dest, const char *data, size_t len);

static size_t memory_writer(void *dest, const char *data, size_t len);
#endif

#endif /* PANDOS_UTIL_IMPL_H */
