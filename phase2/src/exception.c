/**
 * \file exception.c
 * \brief Implementation \ref exception.h and \ref exception_impl.h
 *
 * \author Alessandro Frau
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 30-03-2022
 */

#include "exception.h"
#include "os/interrupt.h"
#include "os/puod.h"
#include "os/syscall.h"
#include "os/util.h"
#include <umps/arch.h>
#include <umps/cp0.h>
#include <umps/libumps.h>

scheduler_control_t tbl_handler()
{
    pandos_kprintf("<< TBL\n");
    return pass_up_or_die((memaddr)PGFAULTEXCEPT);
}

static scheduler_control_t trap_handler()
{
    if (active_process != NULL) {
        int pid = active_process != NULL ? active_process->p_pid : 0;
        pandos_kprintf("<< TRAP (%d)\n", pid);
    } else {
        pandos_kprintf("<< TRAP\n");
    }

    return pass_up_or_die((memaddr)GENERALEXCEPT);
}

void exception_handler()
{
    scheduler_control_t ctrl;
    if (active_process != NULL) {
        pandos_memcpy(&active_process->p_s, (state_t *)BIOSDATAPAGE,
                      sizeof(state_t));
    }

    switch (CAUSE_GET_EXCCODE(getCAUSE())) {
        case 0:
            ctrl = interrupt_handler();
            break;
        case 1:
            break;
        case 2:
            break;
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
