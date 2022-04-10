/**
 * \file init.h
 * \brief Implementation of internal init routines.
 *
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 20-03-2022
 *
 */

#ifndef PANDOS_INIT_H
#define PANDOS_INIT_H

#include "os/ctypes.h"

extern void init(memaddr tbl_refill_handler, memaddr exception_handler,
                 memaddr init_pc);

#endif /* PANDOS_INIT_H */
