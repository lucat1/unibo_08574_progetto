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

inline void reset_timer() { LDIT(IT_INTERVAL); }
inline void reset_plt() { setTIMER(TRANSLATE_TIME(PLT_INTERVAL)); }

#define ENABLE_PLT(status) status = (status | STATUS_TE) ^ STATUS_TE

void scheduler_wait()
{
    stdout("-- WAIT\n");
    // setSTATUS((getSTATUS() | STATUS_TE | STATUS_IM_MASK) ^ STATUS_TE);
    setSTATUS(STATUS_IM_MASK);
    WAIT();
}

void scheduler_takeover()
{
    pandos_kprintf(">> TAKEOVER(%d, %p)\n", active_process->p_pid,
                   active_process->p_s.pc_epc);
    /* Enable interrupts */
    active_process->p_s.status |= STATUS_IEp;
    if (active_process->p_prio) {
        /* Deactives local timer interrupt on hp process */
        // active_process->p_s.status = STATUS_TE;
        // active_process->p_s.status ^= STATUS_TE;
        ENABLE_PLT(active_process->p_s.status);
        // active_process->p_s.status =
        //     (active_process->p_s.status | STATUS_TE) ^ STATUS_TE;
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
