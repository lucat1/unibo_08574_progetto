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
#include "exception_impl.h"
#include "interrupt.h"
#include "os/util.h"
#include "syscall.h"
#include <umps/arch.h>
#include <umps/cp0.h>
#include <umps/libumps.h>

inline scheduler_control_t pass_up_or_die(memaddr type)
{
    if (active_process->p_support == NULL)
        kill_process(active_process);
    else {
        memcpy(&active_process->p_support->sup_except_state[type],
               (state_t *)BIOSDATAPAGE, sizeof(state_t));
        context_t c;
        c.stack_ptr =
            active_process->p_support->sup_except_context[type].stack_ptr;
        c.status = active_process->p_support->sup_except_context[type].status;
        c.pc = active_process->p_support->sup_except_context[type].pc;

        LDCXT(c.stack_ptr, c.status, c.pc);

        scheduler_panic("Control should have been handed off to the support "
                        "layer by now\n");
    }
    return CONTROL_BLOCK;
}

scheduler_control_t tbl_handler()
{
    return pass_up_or_die((memaddr)PGFAULTEXCEPT);
}

static scheduler_control_t trap_handler()
{
    int pid = active_process != NULL ? active_process->p_pid : 0;
    pandos_kprintf("<< TRAP (%d)\n", pid);

    return pass_up_or_die((memaddr)GENERALEXCEPT);
}

void exception_handler()
{
    scheduler_control_t ctrl;

    if (active_process != NULL)
        memcpy(&active_process->p_s, (state_t *)BIOSDATAPAGE, sizeof(state_t));

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
