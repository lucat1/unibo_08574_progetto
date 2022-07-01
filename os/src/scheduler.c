/**
 * \file scheduler.c
 * \brief Schedule awating processes.
 *
 * \author Alessandro Frau
 * \author Gianmaria Rovelli
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

#define process_queue(p) ((p)->p_prio ? &ready_queue_hi : &ready_queue_lo)

size_t process_count;
size_t softblock_count;
list_head ready_queue_lo, ready_queue_hi;
pcb_t *active_process;
pcb_t *yield_process;
cpu_t start_tod;
state_t *wait_state;

/* Always points to the pid of the most recently created process */
static size_t recycle_count;

#ifdef PANDOS_TESTING
size_t get_recycle_count() { return recycle_count; }
#endif

inline void reset_timer() { load_interval_timer(IT_INTERVAL); }
inline void reset_local_timer() { load_local_timer(PLT_INTERVAL); }

inline void enqueue_process(pcb_t *p)
{
    if (p == NULL)
        return;

    insert_proc_q(process_queue(p), p);
}

inline pcb_t *dequeue_process(pcb_t *p)
{
    if (p == NULL)
        return NULL;

    return out_proc_q(process_queue(p), p);
}

inline pcb_t *const find_process(pandos_pid_t pid)
{
    size_t i = mask_pid_id(pid);
    if (i < 0 || i >= MAX_PROC || get_pcb_table()[i].p_pid != pid)
        return NULL;
    return (pcb_t *const)(get_pcb_table() + i);
}

inline pcb_t *spawn_process(bool priority)
{
    pcb_t *p;

    if ((priority != true && priority != false) || (p = alloc_pcb()) == NULL)
        return NULL;
    ++process_count;
    p->p_pid = make_pid(p - get_pcb_table(), recycle_count++);
    p->p_prio = priority;
    enqueue_process(p);
    return p;
}

#ifndef PANDOS_TESTING
static inline
#endif
    int
    kill_process(pcb_t *const p)
{
    if (p == NULL)
        return 1;

    if (p->p_parent != NULL && out_child(p) != p)
        return 2;

    --process_count;

    if (!list_null(&p->p_list)) {
        /* Decrease the amount of processes blocked if one of them is killed */
        if (p->p_sem_add != NULL && out_blocked(p) == p)
            --softblock_count;
        else
            dequeue_process(p);
    }

    /* The removal of the process from any queue is handled by the free_pcb */
    free_pcb(p);
    return 0;
}

inline int kill_progeny(pcb_t *const p)
{
    pcb_t *child;

    while ((child = remove_child(p)) != NULL)
        if (kill_progeny(child))
            return 3;

    return kill_process(p);
}

inline void init_scheduler()
{
    process_count = 0;
    softblock_count = 0;
    mk_empty_proc_q(&ready_queue_hi);
    mk_empty_proc_q(&ready_queue_lo);
    active_process = NULL;
    yield_process = NULL;
    store_tod(&start_tod);
    recycle_count = 0;
}

static inline void scheduler_wait()
{
    active_process = NULL;
    reset_timer();

    size_t status = get_status();
    status_interrupts_on_nucleus(&status);
    status_local_timer_toggle(&status);
    // status_il_on_all(&status);
    set_status(status);

    wait();
    schedule(NULL, false);
}

static inline void scheduler_takeover()
{
    pandos_kprintf("%d\n", active_process->p_pid);
    status_interrupts_on_process(&active_process->p_s.status);
    reset_local_timer();
    /* Disable the processor Local Timer on hi processes */
    if (active_process->p_prio)
        status_local_timer_toggle(&active_process->p_s.status);
    store_tod(&start_tod);
    load_state(&active_process->p_s);
}

static inline void wait_or_die()
{
    if (!process_count)
        halt();
    else if (softblock_count)
        scheduler_wait();
    else
        scheduler_panic("Deadlock detected\n");
}

void schedule(pcb_t *pcb, bool enqueue)
{
    if (enqueue && pcb != NULL) {
        enqueue_process(pcb);
    }

    /* Process selection */
    if (pcb != NULL && !enqueue)
        active_process = pcb;
    else if (!list_empty(&ready_queue_hi))
        active_process = remove_proc_q(&ready_queue_hi);
    else if (!list_empty(&ready_queue_lo))
        active_process = remove_proc_q(&ready_queue_lo);
    else if (yield_process != NULL) {
        active_process = yield_process;
        yield_process = NULL;
    } else
        wait_or_die();

    if (yield_process != NULL) {
        insert_proc_q(process_queue(yield_process), yield_process);
        yield_process = NULL;
    }
    /* This point should never be reached unless processes have been
     * re-scheduled (i.e. when waiting for events in a soft blocked state )
     */
    if (active_process)
        scheduler_takeover();
}

#ifndef __x86_64__
#include <umps/arch.h>
#define __p(...)                                                               \
    __pandos_printf((termreg_t *)DEV_REG_ADDR(IL_TERMINAL, 0), serial_writer,  \
                    __VA_ARGS__)
#endif
void scheduler_panic(const char *fmt, ...)
{
#ifndef __x86_64__
    __p("!! PANIC: ", NULL);
    va_list varg;
    va_start(varg, fmt);
    __p(fmt, varg);
    va_end();
    __p("\n", NULL);
#endif
    panic();
}
#ifndef __x86_64__
#undef __p
#endif
