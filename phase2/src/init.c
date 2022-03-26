/**
 * \file init.c
 * \brief Implementation \ref init.h
 *
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 17-03-2022
 */

#include "init.h"
#include "init_impl.h"
#include "interrupt.h"
#include "native_scheduler.h"
#include "os/asl.h"
#include "os/const.h"
#include "os/pcb.h"
#include "os/scheduler.h"
#include "os/util.h"
#include "p2test.h"
#include "semaphores.h"
#include "umps/cp0.h"
#include "umps/libumps.h"

/* Initialize the Pass Up Vector handlers to the given procedures until the
 * support level is implemented.
 */
inline void init_puv()
{
    passupvector_t *const pv = (passupvector_t *)PASSUPVECTOR;
    pv->tlb_refill_handler = (memaddr)uTLB_RefillHandler;
    pv->tlb_refill_stackPtr = KERNELSTACK;
    pv->exception_handler = (memaddr)exception_handler;
    pv->exception_stackPtr = KERNELSTACK;
}

inline void init_process()
{
    pcb_t *p = spawn_process(true);
    RAMTOP(p->p_s.reg_sp);
    p->p_s.pc_epc = p->p_s.reg_t9 = (memaddr)test;
}

inline void init()
{
    pandos_kprintf("-- init_puv\n");
    init_puv();
    pandos_kprintf("-- init_scheduler\n");
    init_scheduler();
    pandos_kprintf("-- init_semaphores\n");
    init_semaphores();
    pandos_kprintf("-- init_pcbs & init_asl\n");
    init_pcbs();
    init_asl();
    pandos_kprintf("-- reset_timer\n");
    reset_timer();
    pandos_kprintf("-- init_process\n");
    init_process();
}
