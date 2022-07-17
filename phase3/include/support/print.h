/**
 * \file print.h
 * \brief Support level print utilities
 *
 * \author Gianmaria Rovelli
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 26-06-2022
 */

#ifndef PANDOS_PRINT_H
#define PANDOS_PRINT_H

#include "os/types.h"

/**
 * \brief A well-behaved utility for formatted printing on terminal 0,
 * guaranteed not to be interrupted by other prints on the same terminal.
 * \param[in] The format string to be printed.
 * \param[in] ... Additional parameters for the format string.
 * \return The number of characters actually printed.
 */
#define sysprintf(...) fsysprintf(0, __VA_ARGS__)

/**
 * \brief A well-behaved utility for formatted printing, guaranteed not to be
 * interrupted by other prints on the same terminal.
 * \param[in] The identifier of the terminal to be used.
 * \param[in] The format string to be printed.
 * \param[in] ... Additional parameters for the format string.
 * \return The number of characters actually printed.
 */
size_t fsysprintf(int termid, const char *fmt, ...);

#endif /* PANDOS_PRINT_H */
