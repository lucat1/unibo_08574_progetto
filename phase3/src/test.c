#include "arch/devices.h"
#include "arch/processor.h"
#include "os/const.h"
#include "os/ctypes.h"
#include "os/list.h"
#include "os/types.h"
#include "os/util.h"
#include "support/pager.h"
#include "support/support.h"

static support_t support_table[UPROCMAX];
static list_head support_free;

static inline support_t *allocate()
{
    list_head *const lh = list_next(&support_free);
    if (lh == NULL) {
        pandos_kfprintf(&kstderr, "Out of support_t\n");
        PANIC();
    }
    list_del(lh);
    support_t *const res = container_of(lh, support_t, p_list);
    pandos_kfprintf(&kdebug, "Alloc support_t #%d...\n", res - support_table);
    pandos_kfprintf(&kdebug, "|support_free| = %d\n", list_size(&support_free));
    return res;
}

static inline void deallocate(support_t *s)
{
    if (s == NULL) {
        pandos_kfprintf(&kstderr, "NULL deallocation.\n");
        PANIC();
    }
    list_head *const lh = &s->p_list;
    if (list_contains(lh, &support_free)) {
        pandos_kfprintf(&kstderr, "Double support_t %p deallocation.\n", s);
        PANIC();
    }
    pandos_kfprintf(&kdebug, "Deallocating #%d...\n", s - support_table);
    list_add(lh, &support_free);
    pandos_kfprintf(&kdebug, "|support_free| = %d\n", list_size(&support_free));
}

static inline void init_supports()
{
    INIT_LIST_HEAD(&support_free);
    for (size_t i = 0; i < UPROCMAX; ++i)
        deallocate(support_table + i);
}

void test()
{
    state_t pstate;
    size_t support_status;
    memaddr ramtop;
    init_sys_semaphores();
    init_pager();
    init_supports();

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
        const size_t asid = i + 1;

        pstate.entry_hi = asid << ASIDSHIFT;

        support_t *support_structure = allocate();
        support_structure->sup_asid = asid;
        if (!init_page_table(support_structure->sup_private_page_table, asid)) {
            pandos_kfprintf(&kstderr,
                            "Failed to initialize page table (ASID = %d)\n",
                            asid);
            PANIC();
        }
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

void deallocate_support(support_t *s) { deallocate(s); }
