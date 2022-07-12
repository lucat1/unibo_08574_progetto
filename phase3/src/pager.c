#include "support/pager.h"
#include "arch/processor.h"
#include "os/const.h"
#include "os/ctypes.h"
#include "os/scheduler.h"
#include "os/util.h"
#include "support/memory.h"
#include "support/memory_impl.h"
#include "support/storage.h"
#include "support/support.h"
#include "umps/arch.h"
#include "umps/const.h"
#include "umps/cp0.h"
#include "umps/libumps.h"

#define SWAP_POOL_ADDR (KSEG1 + (32 * PAGESIZE))

int sem_swap_pool_table = 1;

// TODO: use another algorithm
size_t i = -1;

inline void update_tlb(size_t index, pte_entry_t pte)
{
    // TODO : reactive this one
    if (check_in_tlb(pte)) {
        // TODO : to remove prob
        // setINDEX(index);
        setENTRYHI(pte.pte_entry_hi);
        setENTRYLO(pte.pte_entry_lo);
        TLBWI();
    }

    // TODO : remove this one
    // TLBCLR();
}

inline bool check_in_tlb(pte_entry_t pte)
{
    setENTRYHI(pte.pte_entry_hi);
    TLBP();
    // hardcoded now
    size_t index = (getINDEX() >> 8) & 63;
    return index != -1;
}

inline void add_random_in_tlb(pte_entry_t pte)
{
    setENTRYHI(pte.pte_entry_hi);
    setENTRYLO(pte.pte_entry_lo);
    TLBWR();
}

inline size_t pick_page() { return (i = (i + 1) % POOLSIZE); }

inline memaddr page_addr(size_t i)
{
    if (i < 0 || i >= POOLSIZE)
        return (memaddr)NULL;
    return (memaddr)(SWAP_POOL_ADDR + i * PAGESIZE);
}

inline size_t page_num(memaddr entryhi)
{
    // reverse operation done in memory.c
    size_t vpn = (entryhi - KUSEG) >> VPNSHIFT;
    if (vpn > MAXPAGES - 2 && vpn != STACK_PAGE_NUMBER)
        pandos_kprintf("Error: vpn = %d?\n", vpn);
    return vpn;
}

inline bool check_frame_occupied(swap_t frame) { return frame.sw_asid != -1; }

inline void mark_page_not_valid(pte_entry_t page_table[], size_t page_no)
{
    page_table[page_no].pte_entry_lo |= VALIDON;
    page_table[page_no].pte_entry_lo ^= VALIDON;
}

inline void add_entry_swap_pool_table(size_t frame_no, size_t asid,
                                      size_t page_no, pte_entry_t page_table[])
{
    swap_pool_table[frame_no].sw_asid = asid;
    swap_pool_table[frame_no].sw_page_no = page_no;
    swap_pool_table[frame_no].sw_pte = &page_table[page_no];
}

inline void update_page_table(pte_entry_t page_table[], size_t page_no,
                              memaddr frame_addr)
{
    page_table[page_no].pte_entry_lo = frame_addr | VALIDON | DIRTYON;
    pandos_kprintf("entry lo %b\n", page_table[page_no].pte_entry_lo);
}

inline void deactive_interrupts()
{
    size_t state = get_status();
    status_interrupts_off_nucleus(&state);
    set_status(state);
}

inline void active_interrupts()
{
    size_t state = get_status();
    status_interrupts_on_nucleus(&state);
    set_status(state);
}

inline void tlb_exceptionhandler()
{
    support_t *support = (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    size_t cause = support->sup_except_state[PGFAULTEXCEPT].cause;
    if (cause == 1 /* TODO : 1 is an example */) {
        // TODO : program trap
    } else {
        pandos_kprintf("tlb_exceptionhandler %p\n", SWAP_POOL_ADDR);
        // gain mutual exclusion over swap pool table
        SYSCALL(PASSEREN, (int)&sem_swap_pool_table, 0,
                0); /* P(sem_swap_pool_table) */
        state_t *saved_state = &support->sup_except_state[PGFAULTEXCEPT];
        int victim_frame = pick_page();
        size_t victim_frame_addr = SWAP_POOL_ADDR + (victim_frame * PAGESIZE);
        size_t p = page_num(saved_state->entry_hi);
        pandos_kprintf("page num %d\n", p);
        // checks if frame victim_frame is occupied
        swap_t swap = swap_pool_table[victim_frame];
        if (check_frame_occupied(swap)) {
            size_t k = page_num(swap.sw_pte->pte_entry_hi);

            // ATOMICALLY
            // interrupts need to be disabled ???
            // is there a function ?
            deactive_interrupts();

            mark_page_not_valid(support->sup_private_page_table, k);
            // TODO : update TLB if needed
            update_tlb(k, *swap.sw_pte);

            active_interrupts();
            // END ATOMICALLY

            // Write the contents of frame victim_frame
            // to the correct location on process x’s backing store/flash device
            if (!write_flash(swap.sw_asid, k,
                             (void *)page_addr(victim_frame))) {
                // call trap
                pandos_kprintf("ERRORE IN SCRITTURA FLASH\n");
            }
        }

        //  Read the contents of the Current Process’s backing store/flash
        //  device logical page p into frame victim_frame.
        if (!read_flash(support->sup_asid, p, (void *)victim_frame_addr)) {
            // call trap
            pandos_kprintf("ERRORE IN LETTURA FLASH\n");
        }

        pandos_kprintf("frame addr %b\n", victim_frame_addr & 0xFFFFF000);

        // ATOMICALLY
        deactive_interrupts();

        add_entry_swap_pool_table(victim_frame, support->sup_asid, p,
                                  support->sup_private_page_table);
        update_page_table(support->sup_private_page_table, p,
                          victim_frame_addr);
        update_tlb(p, support->sup_private_page_table[p]);

        active_interrupts();
        // END ATOMICALLY

        SYSCALL(VERHOGEN, (int)&sem_swap_pool_table, 0,
                0); /* P(sem_swap_pool_table) */

        pandos_kprintf("fine tlb_handler %d\n",
                       page_num(saved_state->entry_hi));
        load_state(saved_state);
    }
}
