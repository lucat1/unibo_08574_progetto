/**
 * \file exception.c
 * \brief Implementation \ref exception.h and \ref exception_impl.h
 *
 * \author Alessandro Frau
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 30-03-2022
 */

#include <umps/cp0.h>
#include <umps/arch.h>

#include "os/const.h"
#include "os/types.h"

#include "exception_impl.h"
#include "exception.h"
#include "interrupt.h"
#include "syscall.h"

scheduler_control_t tbl_handler()
{
    return pass_up_or_die(PGFAULTEXCEPT);
    //return CONTROL_RESCHEDULE;
}

/* TODO: fill me */
scheduler_control_t trap_handler()
{
    int pid = -1;
    if (active_process != NULL)
        pid = active_process->p_pid;
    pandos_kprintf("<< TRAP (%d)\n", pid);

    return pass_up_or_die(GENERALEXCEPT);
}


void exception_handler()
{
    scheduler_control_t ctrl = CONTROL_RESCHEDULE;

    if (active_process != NULL)
        memcpy(&active_process->p_s, (state_t *)BIOSDATAPAGE, sizeof(state_t));

    /* p_s.cause could have been used instead of getCAUSE() */
    switch (CAUSE_GET_EXCCODE(getCAUSE())) {
        case 0:
            ctrl = interrupt_handler();
            break;
        case 1:
        case 2:
        case 3:
            ctrl = tbl_handler();
            break;
        case 8:
            if (active_process == NULL)
                scheduler_panic(
                    "A syscall happened while active_process was NULL\n");
            /* ALWAYS increment the PC to prevent system call loops */
            active_process->p_s.pc_epc += WORD_SIZE;
            active_process->p_s.reg_t9 += WORD_SIZE;
            ctrl = syscall_handler();
            break;
        default: /* 4-7, 9-12 */
            ctrl = trap_handler();
            break;
    }
    schedule(ctrl.pcb, ctrl.enqueue);
}