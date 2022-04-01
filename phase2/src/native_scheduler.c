/**
 * \file native_scheduler.c
 * \brief Schedule awating processes.
 *
 * \author Luca Tagliavini
 * \date 22-03-2022
 */

#include "native_scheduler.h"
#include "os/scheduler.h"
#include "os/util.h"
#include "os/util_impl.h"
#include <umps/cp0.h>
#include <umps/libumps.h>

inline void reset_timer() { LDIT(IT_INTERVAL); }
inline void reset_plt() { setTIMER(TRANSLATE_TIME(PLT_INTERVAL)); }

void scheduler_wait()
{
    pandos_kprintf("-- WAIT(%d)\n");
    active_process = NULL;
    reset_timer();
    setSTATUS((getSTATUS() | STATUS_IEc | STATUS_IM_MASK | STATUS_TE) ^
              STATUS_TE);
    // while (1)
    WAIT();
    schedule(NULL, false);
}

void scheduler_takeover()
{
    pandos_kprintf("entering takeover\n");
    pandos_kprintf(">> TAKEOVER(%d, %p)\n", active_process->p_pid,
                   active_process->p_s.pc_epc);
    /* Enable interrupts */
    active_process->p_s.status |= STATUS_IEp | STATUS_TE | STATUS_IM_MASK;
    reset_plt();
    /* Disable the processor Local Timer on hi processes */
    if (active_process->p_prio)
        active_process->p_s.status ^= STATUS_TE;
    STCK(start_tod);
    LDST(&active_process->p_s);
}

void scheduler_panic(const char *fmt, ...)
{
    pandos_kfprintf(&kstderr, "!! PANIC: ");
    va_list varg;
    va_start(varg, fmt);
    __pandos_printf(&kstderr, memory_writer, fmt, varg);
    va_end();
    __pandos_printf(&kstderr, memory_writer, "\n", NULL);
    PANIC();
}

void scheduler_unlock()
{
    setSTATUS((getSTATUS() | STATUS_IEc | STATUS_IM_MASK | STATUS_TE) ^
              STATUS_TE);
}
