/**
 * \file exception_impl.h
 * \brief Exception handlers implementations.
 *
 * \author Alessandro Frau
 * \author Luca Tagliavini
 * \date 30-03-2022
 *
 */

#ifndef PANDOS_EXCEPTION_IMPL_H
#define PANDOS_EXCEPTION_IMPL_H

#include <umps/libumps.h>

#include "os/const.h"
#include "os/scheduler.h"
#include "os/types.h"
#include "os/util.h"

extern scheduler_control_t tbl_handler();

extern scheduler_control_t trap_handler();

static inline scheduler_control_t pass_up_or_die(int type)
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

        stderr("SHOULD BE PASSED UP\n");
        PANIC();
    }
    return CONTROL_BLOCK;
}

#endif /* PANDOS_EXCEPTION_IMPL_H */
