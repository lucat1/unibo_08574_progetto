/**
 * \file
 * \brief Schedule awating processes.
 *
 * \author Alessandro Frau
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 20-03-2022
 */

#include <umps/libumps.h>

#include "os/asl.h"
#include "os/list.h"
#include "os/pcb.h"
#include "os/scheduler.h"
#include "os/util.h"

int running_count;
int blocked_count;
list_head ready_queue_lo, ready_queue_hi;
pcb_t *active_process;
pcb_t *last_process;
cpu_t start_tod;
state_t *wait_state;

/* Always points to the pid of the most recently created process */
static int pid_count = 0;

void init_scheduler()
{
    running_count = 0;
    blocked_count = 0;
    mk_empty_proc_q(&ready_queue_hi);
    mk_empty_proc_q(&ready_queue_lo);
    active_process = NULL;
    STCK(start_tod);
}

pcb_t *spawn_process(bool priority)
{
    pcb_t *p = alloc_pcb();
    if (p == NULL) {
        return NULL;
    }
    p->p_pid =
        ++pid_count; /* TODO: Change this with the actual implementation */
    p->p_prio = priority;
    ++running_count;
    enqueue_process(p);
    return p;
}

inline void enqueue_process(pcb_t *p)
{
    insert_proc_q(p->p_prio ? &ready_queue_hi : &ready_queue_lo, p);
}

inline void dequeue_process(pcb_t *p)
{
    out_proc_q(p->p_prio ? &ready_queue_hi : &ready_queue_lo, p);
}

/* TODO: Maybe optimize this solution */
static inline void delete_progeny(pcb_t *p)
{
    if (p == NULL)
        return;

    pcb_t *child;
    while ((child = remove_child(p)) != NULL) {
        kill_process(child);
    }
}

void kill_process(pcb_t *p)
{
    if (p != NULL) {
        --running_count;

        /* recursively removes progeny of process that should be terminated */
        delete_progeny(p);

        /* removes process that should be terminated from parent's children */
        out_child(p);

        /* In case it is blocked by a semaphore*/
        out_blocked(p);

        /* In case it is in the ready queue */
        dequeue_process(p);

        /* Set pcb as free */
        free_pcb(p);
    } else {
        stderr("Can't kill NULL process\n");
    }
}

void schedule(pcb_t *pcb, bool enqueue)
{
    pandos_kprintf("-- SCHEDULE(%p, %s)\n", pcb, enqueue ? "true" : "false");
    if (enqueue && pcb != NULL)
        enqueue_process(pcb);

    /* Process selection */
    if (active_process == NULL && running_count == 0) {
        pandos_kprintf("Nothing left, HALT()!");
        HALT();
    } else if (pcb != NULL && !enqueue) {
        active_process = pcb;
    } else if (!list_empty(&ready_queue_hi)) {
        active_process = remove_proc_q(&ready_queue_hi);
    } else if (!list_empty(&ready_queue_lo)) {
        active_process = remove_proc_q(&ready_queue_lo);
    } else {
        active_process = NULL;
        scheduler_unlock();
        scheduler_wait();
    }

    /* This point should never be reached unless processes have been
     * re-scheduled (i.e. when waiting for events in a soft blocked state )
     */
    if (active_process)
        scheduler_takeover();
}
