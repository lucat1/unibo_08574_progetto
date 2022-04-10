#include "os/puod.h"
#include "os/util.h"

inline scheduler_control_t pass_up_or_die(memaddr type)
{
    if (active_process == NULL)
        return CONTROL_BLOCK;

    if (active_process->p_support != NULL) {
        pandos_memcpy(&active_process->p_support->sup_except_state[type],
                      (state_t *)BIOSDATAPAGE, sizeof(state_t));
        load_context(&active_process->p_support->sup_except_context[type]);
    } else
        /* TODO: kill single process and handle children or kill progeny? */
        kill_progeny(active_process);

    return CONTROL_BLOCK;
}
