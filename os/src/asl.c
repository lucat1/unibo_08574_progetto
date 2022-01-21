/** \file
 * \brief Implements \ref asl.h
 *
 * \author Alessandro Frau
 * \author Gianmaria Rovelli
 * \date 17-01-2022
 */

#include "os/asl.h"
#include "umps/types.h"

semd_t semd_table[MAX_PROC];
list_head semd_free;
list_head semd_h;
#ifdef PANDOS_TESTING
semd_t *get_semd_table() { return semd_table; }
list_head *get_semd_free() { return &semd_free; }
list_head *get_semd_h() { return &semd_h; }
#endif

/* TODO: REMOVE */
#include <stdio.h>

void init_asl()
{
    size_t i;

    INIT_LIST_HEAD(&semd_free);
    INIT_LIST_HEAD(&semd_h);
    for (i = 0; i < MAX_PROC; ++i) {
        list_add(&semd_table[i].s_link, &semd_free);
    }
}

#ifndef PANDOS_TESTING
static inline
#endif
semd_t *find_semd(list_head *list, int *sem_addr) {
    list_head *iter;

    int i = 0;
    for (iter = list->next;
         iter != list;
         iter = iter->next) {
        semd_t *sem;
        if((sem = container_of(iter, semd_t, s_link))->s_key == sem_addr)
            return sem;
    }

    return NULL;
}

bool insert_blocked(int *sem_addr, pcb_t *p)
{
    semd_t *sem;

    sem = find_semd(&semd_h, sem_addr);
    if (sem == NULL && !list_empty(&semd_free)) {
        sem = container_of(semd_free.next, semd_t, s_link);
        list_del(&sem->s_link);
        list_add(&sem->s_link, &semd_h);

        /* Fill the new structure */
        sem->s_key = sem_addr;
        INIT_LIST_HEAD(&sem->s_procq);
    }

    /* Return error when we run out of memory */
    if(sem == NULL)
        return true;
    /* TODO: fact checking */
    list_add_tail(&p->p_list, &sem->s_procq);
    return false;
}
