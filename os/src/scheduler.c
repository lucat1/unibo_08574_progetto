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
#include "os/scheduler_impl.h"
#include "os/util.h"

int running_count;
int blocked_count;
list_head ready_queue_lo, ready_queue_hi;
pcb_t *active_process;
pcb_t *last_process;
cpu_t start_tod;
state_t *wait_state;

/* Always points to the pid of the most recently created process */
static unsigned int recycle_count;

/* TODO: test that max_proc_bits is >= log_2(max_proc) */

inline pcb_t *spawn_process(bool priority)
{
    pcb_t *p = alloc_pcb();
    if (p == NULL) {
        return NULL;
    }
    p->p_pid = (p - get_pcb_table()) | (recycle_count++ << MAX_PROC_BITS);
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
    pcb_t *child;

    if (p == NULL)
        return;
    while ((child = remove_child(p)) != NULL)
        kill_process(child);
}

inline const pcb_t *find_process(pid_t pid)
{
    size_t i = mask_pid_id(pid);
    if (i < 0 || i >= MAX_PROC || get_pcb_table()[i].p_pid != pid)
        return NULL;
    return get_pcb_table() + i;
}

/* TODO return int, change if */
inline void kill_process(pcb_t *p)
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
        p->p_pid = -1;
    } else {
        pandos_kfprintf(&kstderr, "Can't kill NULL process\n");
    }
}

extern void init_scheduler()
{
    running_count = 0;
    blocked_count = 0;
    mk_empty_proc_q(&ready_queue_hi);
    mk_empty_proc_q(&ready_queue_lo);
    active_process = NULL;
    STCK(start_tod);
    recycle_count = 0;
}

inline void scheduler_on_empty_queues()
{
    if (active_process == NULL && !running_count) {
        pandos_kprintf("Nothing left, HALT()!");
        HALT();
    } else if (active_process == NULL || blocked_count) {
        active_process = NULL;
        scheduler_unlock();
        scheduler_wait();
    } else
        scheduler_panic("Deadlock detected.\n");
}

void schedule(pcb_t *pcb, bool enqueue)
{
    pandos_kprintf("-- SCHEDULE(%p, %s)\n", pcb, enqueue ? "true" : "false");
    if (enqueue && pcb != NULL)
        enqueue_process(pcb);

    /* Process selection */
    if (pcb != NULL && !enqueue)
        active_process = pcb;
    else if (!list_empty(&ready_queue_hi))
        active_process = remove_proc_q(&ready_queue_hi);
    else if (!list_empty(&ready_queue_lo))
        active_process = remove_proc_q(&ready_queue_lo);
    else
        scheduler_on_empty_queues();

    /* This point should never be reached unless processes have been
     * re-scheduled (i.e. when waiting for events in a soft blocked state )
     */
    if (active_process)
        scheduler_takeover();
}
