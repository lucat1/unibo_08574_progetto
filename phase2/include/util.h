/**
 * \file util.h
 * \brief Implementation of internal init routines.
 *
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 20-03-2022
 */

#ifndef PANDOS_UTIL_H
#define PANDOS_UTIL_H

#include "os/types.h"

extern void reset_timer();

/**
 * \brief Spawns a process and returns the allocated structure.
 * \return The allocated process descriptor.
 */
pcb_t *spawn_process();

#endif /* PANDOS_UTIL_H */
