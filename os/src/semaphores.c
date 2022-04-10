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

inline scheduler_control_t _P(int *const sem_addr, pcb_t *const p)
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

inline pcb_t *_V(int *const sem_addr)
{
    pcb_t *p = remove_blocked(sem_addr);
    if (p == NULL) /* means that sem_proc is empty */
        *sem_addr = *sem_addr + 1;
    else
        enqueue_process(p);

    return p;
}

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
        ++blocked_count;
        return CONTROL_BLOCK;
    } else if ((t = remove_blocked(sem_addr)) != NULL) {
        pandos_kfprintf(&kstdout, "P reschedule (%p) %d - %d\n", sem_addr,
                        p->p_pid, *sem_addr);
        enqueue_process(t);
        --blocked_count;
        return CONTROL_RESCHEDULE;
    } else {
        --*sem_addr;
        pandos_kfprintf(&kstdout, "P decr (%p) %d - %d\n", sem_addr, p->p_pid);
        return CONTROL_RESCHEDULE;
    }
}

inline pcb_t *V(int *const sem_addr)
{
    pcb_t *p;

    if (*sem_addr == 1) {
        /* process already blocked */
        /* nothing to do */
        //
        if (active_process->p_sem_add == NULL) {
            if (!list_empty(&active_process->p_list))
                dequeue_process(active_process);
            ++blocked_count;
            if ((insert_blocked(sem_addr, active_process)) > 0) {
                scheduler_panic("VERHOGEN failed\n");
            }
            
        }
        return NULL;
    } else if ((p = remove_blocked(sem_addr)) != NULL) {
        /* TODO : mhhhhh weirdo */
        --blocked_count;
        enqueue_process(p);
        return p;
    } else {
        ++*sem_addr;
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
