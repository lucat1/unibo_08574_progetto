/**
 * \file
 * \brief Schedule awating processes.
 *
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 20-03-2022
 */

#include "os/scheduler.h"
#include "os/list.h"
#include "os/pcb.h"
#include "os/util.h"

int running_count;
int blocked_count;
list_head ready_queue_hi, ready_queue_lo;
pcb_t *active_process;

/* Always points to the pid of the most recently created process */
static int pid_count = 0;

void init_scheduler()
{
    running_count = 0;
    blocked_count = 0;
    mk_empty_proc_q(&ready_queue_hi);
    mk_empty_proc_q(&ready_queue_lo);
    active_process = NULL;
}

pcb_t *spawn_process(bool priority)
{
    pcb_t *p = alloc_pcb();
    p->p_pid = ++pid_count;
    p->p_prio = priority;
    ++running_count;
    list_add(&p->p_list, priority ? &ready_queue_hi : &ready_queue_lo);
    return p;
}

void kill_process(pcb_t *p)
{ /* TODO */
}

void schedule()
{
    pandos_kprintf(":: scheduling...\n");
    if (!list_empty(&ready_queue_hi))
        active_process = remove_proc_q(&ready_queue_hi);
    else if (!list_empty(&ready_queue_lo))
        active_process = remove_proc_q(&ready_queue_lo);
    else
        pandos_kprintf(":: possibly deadlock\n");

    scheduler_takeover();
}
