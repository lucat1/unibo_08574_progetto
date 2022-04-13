#include "arch/processor.h"
#include "os/scheduler.h"
#include <umps/cp0.h>
#include <umps/libumps.h>
#include <umps/types.h>

inline void init_puv(memaddr tbl_refill_handler, memaddr exception_handler)
{
    passupvector_t *const pv = (passupvector_t *)PASSUPVECTOR;
    pv->tlb_refill_handler = tbl_refill_handler;
    pv->tlb_refill_stackPtr = KERNELSTACK;
    pv->exception_handler = exception_handler;
    pv->exception_stackPtr = KERNELSTACK;
}

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
inline void load_context(context_t *ctx)
{
    LDCXT(ctx->stack_ptr, ctx->status, ctx->pc);
}
inline void store_state(state_t *s) { STST(s); }

inline void halt() { HALT(); }
inline void panic() { PANIC(); }
inline void wait() { WAIT(); }

inline void set_status(size_t status) { setSTATUS(status); }
inline size_t get_status() { return getSTATUS(); }
inline size_t get_cause() { return getCAUSE(); }
inline void status_interrupts_on_nucleus(size_t *prev)
{
    *prev |= STATUS_IEc | STATUS_TE;
}
inline void status_interrupts_on_process(size_t *prev)
{
    *prev |= STATUS_IEp | STATUS_TE;
}
inline void status_toggle_local_timer(size_t *prev) { *prev ^= STATUS_TE; }
inline void status_kernel_mode_on_nucleus(size_t *prev) { *prev |= STATUS_KUc; }
inline void status_kernel_mode_on_process(size_t *prev) { *prev |= STATUS_KUp; }
inline void status_reserved_instruction(size_t *prev)
{
    *prev |= (EXC_RI << CAUSESHIFT);
}
