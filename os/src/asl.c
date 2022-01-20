/** \file
 * \brief Implements \ref asl.h
 *
 * \author Alessandro Frau
 * \author Gianmaria Rovelli
 * \date 17-01-2022
 */

#include "os/asl.h"
#include "os/list.h"

semd_t semd_table[MAX_PROC];
struct list_head semd_free;
struct list_head semd_h;

static void init_asl() 
{
    size_t i;

    INIT_LIST_HEAD(&semd_free);
    INIT_LIST_HEAD(&semd_h);
    for(i = 0; i < MAX_PROC; ++i) {
        list_add_tail(&semd_table[i].s_link, &semd_free);
    }
}

static int insert_blocked(int *sem_addr, pcb_t *p) 
{
    struct list_head* iter;
    semd_t *sem = NULL;

    list_for_each(iter, &semd_h) {
        semd_t *item = container_of(iter, semd_t, s_link);
        // if(item->s_key == sem_addr)
        //     sem = item;
    }
    // sem.s_key = NULL;
    // INIT_LIST_HEAD(sem.s_procq);
}
