#include "arch/devices.h"
#include "arch/processor.h"
#include "os/const.h"
#include "os/ctypes.h"
#include "os/list.h"
#include "os/semaphores.h"
#include "os/types.h"
#include "os/util.h"
#include "support/pager.h"
#include "support/print.h"
#include "support/storage.h"
#include "support/support.h"
#include "umps/arch.h"
#include "umps/types.h"
#include <umps/libumps.h>

static support_t support_table[UPROCMAX];
static list_head support_free;

static inline support_t *allocate()
{
    list_head *res = list_next(&support_free);
    if (res == NULL) {
        pandos_kfprintf(&kstderr, "Out of support_t\n");
        PANIC();
    }
    list_del(res);
    pandos_kfprintf(&kdebug, "Allocating #%d...\n",
                    container_of(res, support_t, p_list) - support_table);
    pandos_kfprintf(&kdebug, "|support_free| = %d\n", list_size(&support_free));
    return container_of(res, support_t, p_list);
}

static inline void deallocate(support_t *s)
{
    if (s == NULL) {
        pandos_kfprintf(&kstderr, "NULL deallocation.\n");
        PANIC();
    }
    if (list_contains(&s->p_list, &support_free)) {
        pandos_kfprintf(&kstderr, "Double support_t %p deallocation.\n", s);
        PANIC();
    }
    pandos_kfprintf(&kdebug, "Deallocating #%d...\n", s - support_table);
    list_add(&s->p_list, &support_free);
    pandos_kfprintf(&kdebug, "|support_free| = %d\n", list_size(&support_free));
}

void deallocate_support(support_t *s) { deallocate(s); }

static inline void init_supports()
{
    INIT_LIST_HEAD(&support_free);
    for (size_t i = 0; i < UPROCMAX; ++i)
        deallocate(support_table + i);
}

void test()
{
    // NON TOCCARE GUAI A TE
    // for (int j = IL_DISK; j < IL_TERMINAL; j++) {
    //     for (int i = 0; i < 8; ++i) {
    //         // pandos_kprintf("DEV N %d - %d\n", j, i);
    //         int *sem = get_semaphore(j, i, false);
    //         int addr = DEV_REG_ADDR(j, i);
    //         dtpreg_t *reg = (dtpreg_t *)addr;
    //         int dev_n = ((int)&reg->command - DEV_REG_START) / DEV_REG_SIZE %
    //         8; iodev_t dev = get_iodev(&reg->command); if (sem !=
    //         dev.semaphore) {
    //             pandos_kprintf("EQ %d - %p, %p\n", dev_n == i, dev_n, i);
    //             pandos_kprintf("EQ2 %d - %p, %p\n", sem == dev.semaphore,
    //             sem,
    //                            dev.semaphore);
    //         }
    //     }
    // }

    state_t pstate;
    size_t support_status;
    memaddr ramtop;
    init_sys_semaphores();
    init_pager();
    init_supports();

    // store_state(&pstate);
    pstate.reg_sp = (memaddr)USERSTACKTOP;
    pstate.pc_epc = pstate.reg_t9 = (memaddr)UPROCSTARTADDR;
    status_local_timer_on(&pstate.status);
    status_interrupts_on_process(&pstate.status);
    status_il_on_all(&pstate.status);
    status_kernel_mode_off_process(&pstate.status);

    support_status = pstate.status;
    status_local_timer_on(&support_status);
    status_interrupts_on_process(&support_status);
    status_il_on_all(&support_status);
    status_kernel_mode_on_process(&support_status);

    RAMTOP(ramtop);
    for (size_t i = 0; i < UPROCMAX; ++i) {
        // NOTE: the ASID of the process is i+1
        const size_t asid = i + 1;

        pstate.entry_hi = asid << ASIDSHIFT;

        support_t *support_structure = allocate();
        support_structure->sup_asid = asid;
        init_page_table(support_structure->sup_private_page_table, asid);
        support_structure->sup_except_context[PGFAULTEXCEPT].pc =
            (memaddr)support_tlb;
        support_structure->sup_except_context[PGFAULTEXCEPT].stack_ptr =
            ramtop - 2 * (asid)*PAGESIZE;
        support_structure->sup_except_context[PGFAULTEXCEPT].status =
            support_status;
        support_structure->sup_except_context[GENERALEXCEPT].pc =
            (memaddr)support_generic;
        support_structure->sup_except_context[GENERALEXCEPT].stack_ptr =
            ramtop - 2 * asid * PAGESIZE + PAGESIZE;
        support_structure->sup_except_context[GENERALEXCEPT].status =
            support_status;

        SYSCALL(CREATEPROCESS, (int)&pstate, PROCESS_PRIO_LOW,
                (int)support_structure);
    }
    for (size_t i = 0; i < UPROCMAX; ++i)
        master_semaphore_p();
    SYSCALL(TERMPROCESS, 0, 0, 0);
}
