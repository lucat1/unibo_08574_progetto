/**
 * \file init.c
 * \brief Implementation \ref init.h
 *
 * \author Alessandro Frau
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 17-03-2022
 */

#include "os/init.h"
#include "os/asl.h"
#include "os/pcb.h"
#include "os/scheduler.h"
#include "os/scheduler_impl.h"
#include "os/semaphores.h"
#include "os/util.h"

static inline void init_process(memaddr init_pc)
{
    pcb_t *p = spawn_process(false);
    RAMTOP(p->p_s.reg_sp);
    p->p_s.pc_epc = p->p_s.reg_t9 = init_pc;
    status_interrupts_on_process(&p->p_s.status);
    // TODO : da riattivare
    // status_il_on_all(&p->p_s.status);
    status_kernel_mode_on_nucleus(&p->p_s.status);
}

inline void init(memaddr tlb_refill_handler, memaddr exception_handler,
                 memaddr init_pc)
{
    init_puv(tlb_refill_handler, exception_handler);
    init_scheduler();
    init_semaphores();
    init_pcbs();
    init_asl();
    reset_timer();
    reset_local_timer();
    init_process(init_pc);
}
