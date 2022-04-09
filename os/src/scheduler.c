/**
 * \file scheduler.c
 * \brief Schedule awating processes.
 *
 * \author Alessandro Frau
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 20-03-2022
 */

#include "os/scheduler.h"
#include "arch/devices.h"
#include "arch/processor.h"
#include "os/asl.h"
#include "os/list.h"
#include "os/pcb.h"
#include "os/scheduler_impl.h"
#include "os/util.h"
#include "os/util_impl.h"

int running_count;
int blocked_count;
list_head ready_queue_lo, ready_queue_hi;
pcb_t *active_process;
pcb_t *last_process;
cpu_t start_tod;
cpu_t last_plt;
state_t *wait_state;

/* Always points to the pid of the most recently created process */
static unsigned int recycle_count;

/* TODO: test that max_proc_bits is >= log_2(max_proc) */

inline pcb_t *spawn_process(bool priority)
{
    pcb_t *p;

    if ((p = alloc_pcb()) == NULL)
        return NULL;
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

inline pcb_t *const find_process(pandos_pid_t pid)
{
    size_t i = mask_pid_id(pid);
    if (i < 0 || i >= MAX_PROC || get_pcb_table()[i].p_pid != pid)
        return NULL;
    return (pcb_t *const)(get_pcb_table() + i);
}

/* TODO return int, change if */
inline void kill_process(pcb_t *const p)
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

inline void init_scheduler()
{
    running_count = 0;
    blocked_count = 0;
    mk_empty_proc_q(&ready_queue_hi);
    mk_empty_proc_q(&ready_queue_lo);
    active_process = NULL;
    store_tod(&start_tod);
    recycle_count = 0;
}

static inline void wait_or_die()
{
    if (active_process == NULL && !running_count) {
        pandos_kprintf("Nothing left, halting");
        halt();
        /* TODO: Can the active process be null and blocked count be 0?
         * I think that active_process == NULL is redundant.
         **/
    } else if (active_process == NULL || blocked_count) {
        active_process = NULL;
        scheduler_wait();
    } else
        scheduler_panic("Deadlock detected.\n");
}

void schedule(pcb_t *pcb, bool enqueue)
{
    if (active_process != NULL) {
        int now_tod;
        store_tod(&now_tod);
        active_process->p_time += (now_tod - start_tod);
    }
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
        wait_or_die();

    /* This point should never be reached unless processes have been
     * re-scheduled (i.e. when waiting for events in a soft blocked state )
     */
    if (active_process)
        scheduler_takeover();
}

inline void reset_timer() { load_interval_timer(IT_INTERVAL); }
inline void reset_local_timer()
{
    store_tod(&last_plt);
    load_local_timer(PLT_INTERVAL);
}

void scheduler_wait()
{
    pandos_kprintf("-- WAIT\n");
    active_process = NULL;
    reset_timer();
    set_status(
        status_toggle_local_timer(status_interrupts_on_nucleus(get_status())));
    wait();
    schedule(NULL, false);
}

void scheduler_takeover()
{
    pandos_kprintf(">> TAKEOVER(%d, %p)\n", active_process->p_pid,
                   active_process->p_s.pc_epc);
    /* Enable interrupts */
    active_process->p_s.status =
        status_interrupts_on_process(active_process->p_s.status);
    reset_local_timer();
    /* Disable the processor Local Timer on hi processes */
    if (active_process->p_prio)
        active_process->p_s.status =
            status_toggle_local_timer(active_process->p_s.status);
    store_tod(&start_tod);
    load_state(&active_process->p_s);
}

void scheduler_panic(const char *fmt, ...)
{
#ifndef __x86_64__
    pandos_kfprintf(&kstderr, "!! PANIC: ");
    va_list varg;
    va_start(varg, fmt);
    __pandos_printf(&kstderr, memory_writer, fmt, varg);
    va_end();
    __pandos_printf(&kstderr, memory_writer, "\n", NULL);
#endif
    panic();
}
