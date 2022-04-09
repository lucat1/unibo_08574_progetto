#include "arch/processor.h"
#include "os/scheduler.h"
#include <umps/libumps.h>

inline bool is_user_mode()
{
    return ((active_process->p_s.status << 28) >> 31);
}

inline void null_state(state_t *s)
{
    s->entry_hi = 0;
    s->cause = 0;
    s->status = UNINSTALLED;
    s->pc_epc = 0;
    s->hi = 0;
    s->lo = 0;
    for (int i = 0; i < STATE_GPR_LEN; i++)
        s->gpr[i] = 0;
}

inline void halt() { HALT(); }
inline void panic() { PANIC(); }
inline void wait() { WAIT(); }
