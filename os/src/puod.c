/**
 * \file puod.c
 * \brief Pass up or die
 *
 * \author Gianmaria Rovelli
 * \date 20-03-2022
 */

#include "os/puod.h"
#include "os/util.h"

inline scheduler_control_t pass_up_or_die(memaddr type)
{
    pandos_kprintf("PUOD %d %s\n", active_process->p_pid,
                   type ? "GENERALEXCEPT" : "PGFAULTEXCEPT");
    if (active_process == NULL)
        return CONTROL_BLOCK;

    if (active_process->p_support != NULL) {
        pandos_memcpy(&active_process->p_support->sup_except_state[type],
                      (state_t *)BIOSDATAPAGE, sizeof(state_t));
        pandos_kprintf("running support\n");
        load_context(&active_process->p_support->sup_except_context[type]);
    } else
        kill_progeny(active_process);

    return CONTROL_BLOCK;
}
