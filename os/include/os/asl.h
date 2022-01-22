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
void set_semd_free(const list_head new);
list_head *get_semd_h();

semd_t *alloc_semd(int *sem_addr);
bool free_semd(semd_t *semd);
semd_t *find_semd(list_head *list, int *sem_addr);
#endif

void init_asl();
int insert_blocked(int *sem_addr, pcb_t *p);
pcb_t *out_blocked(pcb_t *pcb);
pcb_t *head_blocked(int *sem_addr);
pcb_t *remove_blocked(int *sem_addr);

#endif /* PANDOS_ASL_H */
