#include "arch/processor.h"
#include "os/scheduler.h"
#include <umps/cp0.h>
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
inline void load_state(state_t *s) { LDST(s); }
inline void store_state(state_t *s) { STST(s); }

inline void halt() { HALT(); }
inline void panic() { PANIC(); }
inline void wait() { WAIT(); }

inline void set_status(size_t status) { setSTATUS(status); }
inline size_t get_status() { return getSTATUS(); }
inline size_t status_interrupts_on_nucleus(size_t prev)
{
    return prev | STATUS_IEc | STATUS_IM_MASK | STATUS_TE;
}
inline size_t status_interrupts_on_process(size_t prev)
{
    return prev | STATUS_IEp | STATUS_IM_MASK | STATUS_TE;
}
inline size_t status_toggle_local_timer(size_t prev)
{
    return prev ^ STATUS_TE;
}
