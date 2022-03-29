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


state_t *wait_state;

inline void reset_timer() { LDIT(IT_INTERVAL);}
inline void reset_plt() {
    setTIMER(TRANSLATE_TIME(PLT_INTERVAL));
}

int waiting_count = 0;

void scheduler_wait()
{   
    stdout("[-] WAITING (%d)\n", ++waiting_count);
    active_process = NULL;
    reset_timer();
    setSTATUS((getSTATUS() | STATUS_IEc | STATUS_IM_MASK | STATUS_TE) ^ STATUS_TE);
    while(1) WAIT();
}

void scheduler_takeover()
{
    pandos_kprintf(">> TAKEOVER(%d,%p)\n", active_process->p_pid,
                   active_process->p_s.pc_epc);
    /* Enable interrupts */
    active_process->p_s.status |= STATUS_IEp | STATUS_TE | STATUS_IM_MASK;
    reset_plt();
    /* Disable the processor Local Timer on hi processes */
    if (active_process->p_prio) 
        active_process->p_s.status ^= STATUS_TE;

    LDST(&active_process->p_s);
}

void scheduler_panic(const char *msg)
{
    pandos_kfprintf(&kstderr, "!! PANIC: %s", msg);
    PANIC();
}
