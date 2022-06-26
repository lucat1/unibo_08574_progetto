#include "os/const.h"
#include "os/types.h"
#include "os/util.h"
#include "support/memory_impl.h"
#include "support/print.h"
#include "support/support.h"
#include <umps/libumps.h>

static support_t support_structures[UPROCMAX];

void test()
{
    state_t pstate;
    size_t support_status;
    memaddr ramtop;
    sysprintf("Initializing...\n");
    swap_pool_sem = 1;
    swap_pool = (swap_t *)(KSEG1 + (32 * PAGESIZE));
    for (size_t i = 0; i < POOLSIZE; ++i)
        swap_pool[i].sw_asid = -1;

    store_state(&pstate);
    pstate.pc_epc = pstate.reg_t9 = UPROCSTARTADDR;
    pstate.reg_sp = USERSTACKTOP;
    status_local_timer_on(&pstate.status);
    status_interrupts_on_process(&pstate.status);
    status_kernel_mode_off_process(&pstate.status);

    status_local_timer_on(&support_status);
    status_interrupts_on_process(&support_status);
    status_kernel_mode_on_process(&support_status);
    RAMTOP(ramtop);
    for (size_t i = 0; i < UPROCMAX; ++i) {
        // NOTE: the ASID of the process is i+1
        const size_t asid = i + 1;

        pstate.entry_hi = asid;

        support_structures[i].sup_asid = asid;
        init_page_table(support_structures[i].sup_private_page_table, asid);
        support_structures[i].sup_except_context[0].pc = (memaddr)support_tbl;
        support_structures[i].sup_except_context[0].stack_ptr =
            ramtop - 2 * (asid - 1) * PAGESIZE;
        support_structures[i].sup_except_context[0].status = support_status;
        support_structures[i].sup_except_context[0].pc =
            (memaddr)support_generic;
        support_structures[i].sup_except_context[0].stack_ptr =
            ramtop - 2 * asid * PAGESIZE;
        support_structures[i].sup_except_context[1].status = support_status;

        SYSCALL(CREATEPROCESS, (int)&pstate, PROCESS_PRIO_LOW,
                (int)(support_structures + i));
    }
    SYSCALL(TERMPROCESS, 0, 0, 0);
}
