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
#include "umps/const.h"
#include "umps/cp0.h"
#include "umps/libumps.h"

inline void reset_timer() { LDIT(IT_INTERVAL);}
inline void reset_plt() {
    setTIMER(TRANSLATE_TIME(PLT_INTERVAL));
}

void scheduler_wait()
{
    //setSTATUS(getSTATUS() | STATUS_IEc | STATUS_IM_MASK | STATUS_TE);
    //setSTATUS(getSTATUS() ^ STATUS_TE);
    // reset_timer();
    /* enables all interreupts and disables local timer */
    active_process->p_s.status |= STATUS_TE | STATUS_IM_MASK;
    active_process->p_s.status ^= STATUS_TE;
    stdout("WAITING\n");
    WAIT();
}

void scheduler_takeover()
{
    pandos_kprintf(">> TAKEOVER(%d,%p)\n", active_process->p_pid,
                   active_process->p_s.pc_epc);
    /* Enable interrupts */
    active_process->p_s.status |= STATUS_IEp;
    reset_plt();
    /* Enable the processor Local Timer */
    if (!active_process->p_prio) {
        active_process->p_s.status |= STATUS_TE | STATUS_IM_MASK;
    }else {
        /* deactives local timer interrupt on hp process */
        active_process->p_s.status |= STATUS_TE;
        active_process->p_s.status ^= STATUS_TE;
    }
    LDST(&active_process->p_s);
}

void scheduler_panic(const char *msg)
{
    pandos_kfprintf(&kstderr, "!! PANIC: %s", msg);
    PANIC();
}
