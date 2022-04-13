/**
 * \file semaphore.c
 * \brief Device semaphore management.
 *
 * \author Gianmaria Rovelli
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 17-03-2022
 */

#include "os/semaphores.h"
#include "os/asl.h"
#include "os/scheduler.h"
#include "os/util.h"
#include <umps/arch.h>

/* Semaphores for each device */
int semaphores[SEMAPHORES_NUM];

inline scheduler_control_t P(int *const sem_addr, pcb_t *const p)
{
    int r;
    pcb_t *t;

    if (*sem_addr == 0) {
        if (!list_empty(&p->p_list))
            dequeue_process(p);
        if ((r = insert_blocked(sem_addr, p)) > 0)
            scheduler_panic("PASSEREN failed %d\n", r);
        /* TODO : mhhhhh weirdo */
        pandos_kfprintf(&kstdout, "P block (%p) %d - %d\n", sem_addr, p->p_pid,
                        *sem_addr);
        ++softblock_count;
        return CONTROL_BLOCK;
    } else if ((t = remove_blocked(sem_addr)) != NULL) {
        enqueue_process(t);
        --softblock_count;
        return CONTROL_RESCHEDULE;
    } else {
        --*sem_addr;
        return CONTROL_RESCHEDULE;
    }
}

inline pcb_t *V(int *const sem_addr)
{
    pcb_t *p;

    if (*sem_addr == 1) {
        if (active_process->p_sem_add == NULL) {
            if (!list_empty(&active_process->p_list))
                dequeue_process(active_process);
            ++softblock_count;
            if ((insert_blocked(sem_addr, active_process)) > 0)
                scheduler_panic("VERHOGEN failed\n");
        }
        return NULL;
    } else if ((p = remove_blocked(sem_addr)) != NULL) {
        /* TODO : mhhhhh weirdo */
        --softblock_count;
        enqueue_process(p);
        return p;
    } else {
        ++*sem_addr;
        return active_process;
    }
}

int *get_semaphore(int int_l, int dev_n, bool is_w)
{
    int sem;

    if (int_l > IL_TERMINAL || int_l < IL_DISK)
        scheduler_panic("Semaphore not found\n");

    sem = (int_l - IL_DISK) * DEVPERINT;
    if (int_l == IL_TERMINAL) {
        sem += 2 * dev_n + (int)is_w;
    } else {
        sem += dev_n;
    }

    return &semaphores[sem];
}

int *get_timer_semaphore() { return &semaphores[SEMAPHORES_NUM - 1]; }

inline void init_semaphores()
{
    /* Semaphores */
    for (int i = 0; i < SEMAPHORES_NUM; i++)
        semaphores[i] = 0;
}
