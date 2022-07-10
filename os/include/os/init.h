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

/**
 * \brief Initializes the data structures, the devices, the Pass Up Vector, and
 * the first process.
 * \param[in] tlb_refill_handler Nucleus TLB-Refill event handler address.
 * \param[in] exception_handler Nucleus exception handler address.
 * \param[in] init_pc Nucleus function that is to be the entry point of the
 * exception (and interrupt) handling.
 */
extern void init(memaddr tlb_refill_handler, memaddr exception_handler,
                 memaddr init_pc);

#endif /* PANDOS_INIT_H */
