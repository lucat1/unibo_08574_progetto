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

/* Semaphores for each device */
int disk_semaphores[DEVPERINT];
int tape_semaphores[DEVPERINT];
int ethernet_semaphores[DEVPERINT];
int printer_semaphores[DEVPERINT];
int termr_semaphores[DEVPERINT];
int termw_semaphores[DEVPERINT];
int timer_semaphore;

inline scheduler_control_t P(int *const sem_addr, pcb_t *const p)
{
    int r;

    if (*sem_addr > 0) {
        *sem_addr = *sem_addr - 1;
        return CONTROL_RESCHEDULE;
    } else {
        /* NOTE: dequeing would be required here but in our use case this
         * procedure is always called with the formal argument p equal to
         * active_process which is assumed to be outside of any queue.
         * If that wasn't the case the following call would be needed:
         * > dequeue_process(p);
         */
        if ((r = insert_blocked(sem_addr, p)) > 0)
            scheduler_panic("PASSEREN failed\n");
        return CONTROL_BLOCK;
    }
}

inline pcb_t *V(int *const sem_addr)
{
    pcb_t *p = remove_blocked(sem_addr);
    if (p == NULL) /* means that sem_proc is empty */
        *sem_addr = *sem_addr + 1;
    else
        enqueue_process(p);

    return p;
}

inline scheduler_control_t _P(int *const sem_addr, pcb_t *const p)
{
    int r;
    // pcb_t *t;

    if (*sem_addr == 0) {
        if ((r = insert_blocked(sem_addr, p)) > 0)
            scheduler_panic("PASSEREN failed\n");
        /* TODO : mhhhhh weirdo */
        blocked_count++;
        return CONTROL_BLOCK;
    }
    // /* TODO : big problem here !!! */
    // else if (!is_ready_queue_empty()) {
    //     pandos_kfprintf(&kstdout, "P reschedule (%p) %d - %d\n", sem_addr,
    //     p->p_pid, *sem_addr);
    //     //enqueue_process(p);
    //     return CONTROL_BLOCK;
    // }
    else {
        *sem_addr = *sem_addr - 1;
        pandos_kfprintf(&kstdout, "P decr (%p) %d - %d\n", sem_addr, p->p_pid);
        return CONTROL_RESCHEDULE;
    }
}

inline pcb_t *_V(int *const sem_addr)
{
    pcb_t *p;

    if (*sem_addr == 1) {
        /* nothing to do */
        return NULL;
    } else if ((p = remove_blocked(sem_addr)) != NULL) {
        enqueue_process(p);
        return p;
    } else {
        *sem_addr = *sem_addr + 1;
        /* TODO : mhhhhh weirdo */
        if (blocked_count > 0)
            blocked_count--;
        pandos_kfprintf(&kstdout, "V sblock (%p)\n", sem_addr);
        return active_process;
    }
}

inline void init_semaphores()
{
    /* Semaphores */
    for (int i = 0; i < DEVPERINT; ++i)
        disk_semaphores[i] = tape_semaphores[i] = ethernet_semaphores[i] =
            printer_semaphores[i] = termr_semaphores[i] = termw_semaphores[i] =
                0;
    timer_semaphore = 0;
}
