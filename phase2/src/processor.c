/**
 * \file processor.c
 * \brief Implementation of architecture-specific functionality for the CPU.
 *
 * \author Luca Tagliavini
 * \date 10-04-2022
 */

#include "arch/processor.h"
#include "os/scheduler.h"
#include "umps/arch.h"
#include <umps/cp0.h>
#include <umps/libumps.h>
#include <umps/types.h>

inline void init_puv(memaddr tlb_refill_handler, memaddr exception_handler)
{
    passupvector_t *const pv = (passupvector_t *)PASSUPVECTOR;
    pv->tlb_refill_handler = tlb_refill_handler;
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
inline void status_interrupts_on_nucleus(size_t *prev) { *prev |= STATUS_IEc; }
inline void status_interrupts_off_nucleus(size_t *prev)
{
    *prev |= STATUS_IEc;
    *prev ^= STATUS_IEc;
}
inline void status_interrupts_on_process(size_t *prev) { *prev |= STATUS_IEp; }
inline void status_local_timer_toggle(size_t *prev) { *prev ^= STATUS_TE; }
inline void status_local_timer_on(size_t *prev)
{
    *prev |= STATUS_TE | STATUS_IM(IL_TIMER);
}
inline void status_local_timer_off(size_t *prev)
{
    *prev |= STATUS_TE | STATUS_IM(IL_TIMER);
    *prev ^= STATUS_TE | STATUS_IM(IL_TIMER);
}
inline void status_local_timer_on_process(size_t *prev)
{
    *prev |= STATUS_TE | STATUS_IM(IL_TIMER);
}
inline void status_kernel_mode_on_nucleus(size_t *prev)
{
    *prev |= STATUS_KUc;
    *prev ^= STATUS_KUc;
}
inline void status_kernel_mode_off_nucleus(size_t *prev)
{
    *prev |= STATUS_KUc;
}
inline void status_kernel_mode_on_process(size_t *prev)
{
    *prev |= STATUS_KUp;
    *prev ^= STATUS_KUp;
}
inline void status_kernel_mode_off_process(size_t *prev)
{
    *prev |= STATUS_KUc;
}

inline void cause_clean(size_t *prev) { *prev &= CLEAREXECCODE; }
inline void cause_reserved_instruction(size_t *prev)
{
    *prev |= (EXC_RI << CAUSESHIFT);
}
