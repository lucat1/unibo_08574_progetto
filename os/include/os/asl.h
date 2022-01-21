/** \file
 * \brief Active Semaphore List
 *
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 17-01-2022
 */

#ifndef PANDOS_ASL_H
#define PANDOS_ASL_H

#include "os/list.h"
#include "os/pcb.h"
#include "os/types.h"

#ifdef PANDOS_TESTING
semd_t *get_semd_table();
list_head *get_semd_free();
list_head *get_semd_h();
semd_t *find_semd(list_head *list, int *sem_addr);
#endif

void init_asl();
bool insert_blocked(int *sem_addr, pcb_t *p);

#endif /* PANDOS_ASL_H */
