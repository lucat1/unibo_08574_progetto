/** \file
 * \brief Active Semaphore List
 *
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 17-01-2022
 */

#ifndef PANDOS_ASL_H
#define PANDOS_ASL_H

#include "os/types.h"
#include "os/list.h"

extern semd_t semd_table[MAX_PROC];
extern struct list_head semd_free;
extern struct list_head semd_h;

static void init_asl();

#endif /* PANDOS_ASL_H */
