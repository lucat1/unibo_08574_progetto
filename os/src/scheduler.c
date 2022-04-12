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
inline size_t get_recycle_count() { return recycle_count; }
#endif

/* TODO: test that max_proc_bits is >= log_2(max_proc) */

inline void enqueue_process(pcb_t *p)
{
    if (p == NULL)
        return;

    insert_proc_q(p->p_prio ? &ready_queue_hi : &ready_queue_lo, p);
}

inline pcb_t *dequeue_process(pcb_t *p)
{
    pcb_t *r;

    if (p == NULL ||
        (r = out_proc_q(p->p_prio ? &ready_queue_hi : &ready_queue_lo, p)) ==
            NULL)
        return NULL;

    return r;
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

    if ((p = alloc_pcb()) == NULL)
        return NULL;
    ++process_count;
    p->p_pid = make_pid(p - get_pcb_table(), recycle_count++);
    p->p_prio = priority;
    enqueue_process(p);
    return p;
}

static inline int kill_process(pcb_t *const p)
{
    if (p == NULL)
        return 1;

    if (p->p_parent != NULL && !out_child(p))
        return 2;

    --process_count;
    /* In case it is blocked by a semaphore*/
    if (out_blocked(p) != NULL)
        --softblock_count;
    else
        dequeue_process(p);

    free_pcb(p);
    return 0;
}

inline int kill_progeny(pcb_t *p)
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

static inline void wait_or_die()
{
    if (!process_count)
        halt();
    else if (softblock_count)
        scheduler_wait();
    else
        scheduler_panic("Deadlock detected\n");
}

void reset_yield_process()
{
    if (yield_process != NULL) {
        insert_proc_q(yield_process->p_prio ? &ready_queue_hi : &ready_queue_lo,
                      yield_process);
        yield_process = NULL;
    }
}

void schedule(pcb_t *pcb, bool enqueue)
{
    if (active_process != NULL) {
        int now_tod;
        store_tod(&now_tod);
        active_process->p_time += (now_tod - start_tod);
    }
    pandos_kprintf("-- SCHEDULE(%p, %s)\n", pcb, enqueue ? "true" : "false");
    if (enqueue && pcb != NULL) {
        enqueue_process(pcb);
    }

    /* Process selection */
    if (pcb != NULL && !enqueue)
        active_process = pcb;
    else if (!list_empty(&ready_queue_hi)) {
        active_process = remove_proc_q(&ready_queue_hi);
        reset_yield_process();
    } else if (!list_empty(&ready_queue_lo)) {
        active_process = remove_proc_q(&ready_queue_lo);
        reset_yield_process();
    } else if (yield_process != NULL) {
        active_process = yield_process;
        yield_process = NULL;
    } else
        wait_or_die();

    /* This point should never be reached unless processes have been
     * re-scheduled (i.e. when waiting for events in a soft blocked state )
     */
    if (active_process)
        scheduler_takeover();
}

inline void reset_timer() { load_interval_timer(IT_INTERVAL); }
inline void reset_local_timer() { load_local_timer(PLT_INTERVAL); }

void scheduler_wait()
{
    pandos_kprintf("-- WAIT\n");
    active_process = NULL;
    reset_timer();

    size_t status = get_status();
    status_interrupts_on_nucleus(&status);
    status_toggle_local_timer(&status);
    // status_il_on_all(&status);
    set_status(status);

    wait();
    schedule(NULL, false);
}

void scheduler_takeover()
{
    pandos_kprintf(">> TAKEOVER(%d)\n", active_process->p_pid);
    status_interrupts_on_process(&active_process->p_s.status);
    reset_local_timer();
    /* Disable the processor Local Timer on hi processes */
    if (active_process->p_prio)
        status_toggle_local_timer(&active_process->p_s.status);
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
