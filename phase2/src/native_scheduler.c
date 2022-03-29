/**
 * \file native_scheduler.c
 * \brief Schedule awating processes.
 *
 * \author Luca Tagliavini
 * \date 22-03-2022
 */

#include "native_scheduler.h"
#include "interrupt.h"
#include "os/scheduler.h"
#include "os/util.h"
#include "umps/arch.h"
#include "umps/const.h"
#include "umps/cp0.h"
#include "umps/libumps.h"

inline void reset_timer() { LDIT(IT_INTERVAL); }
inline void reset_plt() { setTIMER(TRANSLATE_TIME(PLT_INTERVAL)); }
#define ENABLE_PLT(status) (status | STATUS_TE)

void scheduler_wait()
{
    pandos_kprintf("-- WAIT\n");
    // setSTATUS(getSTATUS() ^ STATUS_TE);
    //  reset_timer();
    /* enables all interreupts and disables local timer */
    // STATUS_IM(IL_TIMER); setTIMER(TRANSLATE_TIME(100000000));
    // LDIT(10000000000);

    reset_plt();
    reset_timer();

    while (1)
        WAIT();
}

void scheduler_takeover()
{
    pandos_kprintf("entering takeover\n");
    pandos_kprintf(">> TAKEOVER(%d, %p)\n", active_process->p_pid,
                   active_process->p_s.pc_epc);
    /* Enable interrupts */
    // setSTATUS(getSTATUS() | STATUS_IEp);
    active_process->p_s.status |= STATUS_IEc;
    /* Enable the processor Local Timer */
    if (active_process->p_prio) {
        active_process->p_s.status |= STATUS_TE;
        active_process->p_s.status ^= STATUS_TE;
    } else {
        reset_plt();
        /* Enable the processor Local Timer */
        active_process->p_s.status |= STATUS_TE | STATUS_IM_MASK;
    }
    LDST(&active_process->p_s);
}

void scheduler_panic(const char *msg)
{
    pandos_kfprintf(&kstderr, "!! PANIC: %s", msg);
    PANIC();
}

void scheduler_unlock()
{
    setSTATUS(getSTATUS() | STATUS_IEc | STATUS_TE | STATUS_IM_MASK);
}
