/**
 * \file semaphore.c
 * \brief Device semaphore global values.
 *
 * \author Gianmaria Rovelli
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 17-03-2022
 */

#ifndef PANDOS_SEMAPHORE_H
#define PANDOS_SEMAPHORE_H


#include "os/types.h"
#include "os/list.h"
#include "os/util.h"
#include "os/pcb.h"
#include "os/asl.h"
#include "os/scheduler.h"

extern void init_semaphores();

static inline pcb_t *P(int *sem_addr, pcb_t *p)
{
    if (*sem_addr > 0) {
        *sem_addr = *sem_addr - 1;
        return p;
    } else {
        /* TODO: dequeing here is useless if the p is the current_process */
        dequeue_process(p);
        int r = insert_blocked(sem_addr, p);

        if (r > 0) {
            scheduler_panic("PASSEREN failed\n");
        }

        return NULL;
    }
}

static inline pcb_t *V(int *sem_addr)
{
    pcb_t *p = remove_blocked(sem_addr);
    if (p == NULL) { /* means that sem_proc is empty */
        *sem_addr = *sem_addr + 1;
    } else {
        enqueue_process(p);
    }

    return p;
}

static inline scheduler_control_t mask_V(pcb_t *p)
{
    if (p == NULL)
        return CONTROL_PRESERVE(active_process);
    return CONTROL_RESCHEDULE;
}

static inline scheduler_control_t mask_P(pcb_t *p)
{
    if (p == NULL)
        return CONTROL_BLOCK;
    return CONTROL_RESCHEDULE;
}

#endif /* PANDOS_SEMAPHORE_H */
