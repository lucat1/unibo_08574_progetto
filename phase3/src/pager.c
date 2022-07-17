#include "support/pager.h"
#include "os/const.h"
#include "os/scheduler.h"
#include "os/util.h"
#include "support/storage.h"
#include "support/support.h"

#define CAUSE_TLB_MOD 1
#define SWAP_POOL_ADDR                                                         \
    (*(memaddr *)(KERNELSTACK + 0x0018) + *(memaddr *)(KERNELSTACK + 0x0024))

typedef swap_t swap_table_t[POOLSIZE];

static memaddr swap_pool_addr;
static bool swap_pool_batons[UPROCMAX];
static bool swap_pool_batons[UPROCMAX];

static swap_table_t swap_pool_table;
static int sem_swap_pool_table = 1;

static inline void init_swap_pool_table()
{
    for (size_t i = 0; i < POOLSIZE; ++i)
        swap_pool_table[i].sw_asid = -1;
}

inline void init_pager()
{
    init_swap_pool_table();
    swap_pool_addr = SWAP_POOL_ADDR;
}

inline bool init_page_table(page_table_t page_table, int asid)
{
    if (page_table == NULL || asid <= 0 || asid > UPROCMAX)
        return false;

    const size_t last_page_index = MAXPAGES - 1;
    const memaddr data[PAGESIZE / sizeof(memaddr)];
    const int read_status = read_flash(asid, 0, (void *)data);
    if (read_status != DEV_STATUS_READY) {
        pandos_kfprintf(&kstderr,
                        "Failed header read (AISD = %d, status = %d)\n", asid,
                        read_status);
        support_trap();
    }
    const size_t text_file_size = data[5] / PAGESIZE;

    for (size_t i = 0; i < last_page_index; ++i) {
        page_table[i].pte_entry_hi =
            KUSEG + (i << VPNSHIFT) + (asid << ASIDSHIFT);
        page_table[i].pte_entry_lo = i < text_file_size ? 0 : DIRTYON;
    }
    page_table[last_page_index].pte_entry_hi =
        KUSEG + GETPAGENO + (asid << ASIDSHIFT);
    page_table[last_page_index].pte_entry_lo = DIRTYON;

    swap_pool_batons[asid - 1] = false;

    return true;
}

static inline bool is_valid_asid(int asid)
{
    return 1 <= asid && asid <= UPROCMAX;
}

static inline void set_swap_pool_baton(int asid, bool value)
{
    if (!is_valid_asid(asid)) {
        pandos_kfprintf(&kstderr,
                        "memory.c: set_swap_pool_baton: invalid asid\n");
        PANIC();
    }
    swap_pool_batons[asid - 1] = value;
}

static inline bool get_swap_pool_baton(int asid)
{
    if (!is_valid_asid(asid)) {
        pandos_kfprintf(&kstderr,
                        "memory.c: get_swap_pool_baton: invalid asid\n");
        PANIC();
    }
    return swap_pool_batons[asid - 1];
}

static inline void release_sem_swap_pool_table(int asid)
{
    if (get_swap_pool_baton(asid))
        SYSCALL(VERHOGEN, (int)&sem_swap_pool_table, 0, 0);
}

static inline void mark_frames_as_unoccupied(int asid)
{
    for (size_t i = 0; i < POOLSIZE; ++i)
        if (swap_pool_table[i].sw_asid == asid)
            swap_pool_table[i].sw_asid = -1;
}

inline void clean_after_uproc(int asid)
{
    release_sem_swap_pool_table(asid);
    mark_frames_as_unoccupied(asid);
}

static inline size_t vpn_to_index(size_t vpn)
{
    if (vpn < KUSEG >> VPNSHIFT) {
        pandos_kfprintf(
            &kstderr,
            "pager: vpn_to_index: vpn = %p < %p = KUSEG >> VPNSHIFT\n", vpn,
            KUSEG >> VPNSHIFT);
        panic();
    }
    if (vpn == (KUSEG + GETPAGENO) >> VPNSHIFT)
        return MAXPAGES - 1;
    if (vpn >= (KUSEG >> VPNSHIFT) + MAXPAGES) {
        pandos_kfprintf(&kstderr, "pager: vpn_to_index: invalid vpn %p\n", vpn);
        panic();
    }
    return vpn - (KUSEG >> VPNSHIFT);
}

static inline size_t entryhi_to_vpn(memaddr entryhi)
{
    if (entryhi < KUSEG) {
        pandos_kfprintf(&kstderr,
                        "pager: entryhi_to_vpn: entryhi = %p < %p = KUSEG\n",
                        entryhi, KUSEG);
        panic();
    }
    if (entryhi >= KUSEG + (MAXPAGES - 1) * PAGESIZE &&
        (entryhi >> VPNSHIFT) != (KUSEG + GETPAGENO) >> VPNSHIFT) {
        pandos_kfprintf(&kstderr, "pager: entryhi_to_vpn: %p?\n", entryhi,
                        KUSEG);
        panic();
    }
    return entryhi >> VPNSHIFT;
}

#define entryhi_to_index(entryhi) (vpn_to_index(entryhi_to_vpn(entryhi)))

static inline void add_random_in_tlb(pte_entry_t pte)
{
    setENTRYHI(pte.pte_entry_hi);
    setENTRYLO(pte.pte_entry_lo);
    TLBWR();
}

inline void tlb_refill_handler()
{
    state_t *const saved_state = (state_t *)BIOSDATAPAGE;
    const size_t index = entryhi_to_index(saved_state->entry_hi);
    const pte_entry_t pte =
        active_process->p_support->sup_private_page_table[index];

    pandos_kfprintf(&kdebug, "TLB-Refill on %p\n", saved_state->entry_hi);
    add_random_in_tlb(pte);
    load_state(saved_state);
}

static inline bool check_in_tlb(pte_entry_t pte)
{
    setENTRYHI(pte.pte_entry_hi);
    TLBP();
    return !(getINDEX() & PRESENTFLAG);
}

static inline void update_tlb(size_t index, pte_entry_t pte)
{
    if (check_in_tlb(pte)) {
        setENTRYHI(pte.pte_entry_hi);
        setENTRYLO(pte.pte_entry_lo);
        TLBWI();
    }
}

static inline int check_frame_occupied(swap_t frame)
{
    return frame.sw_asid != -1;
}

static inline size_t pick_page()
{
    for (int index = 0; index < POOLSIZE; ++index)
        if (!check_frame_occupied(swap_pool_table[index]))
            return index;
    static size_t i = -1;
    return i = (i + 1) % POOLSIZE;
}

static inline memaddr page_addr(size_t i)
{
    if (i < 0 || i >= POOLSIZE)
        return (memaddr)NULL;
    return swap_pool_addr + i * PAGESIZE;
}

static inline void mark_page_not_valid(page_table_t page_table, size_t index)
{
    page_table[index].pte_entry_lo &= !VALIDON;
}

static inline void add_entry_swap_pool_table(size_t frame_no, size_t asid,
                                             size_t vpn,
                                             page_table_t page_table)
{
    swap_pool_table[frame_no].sw_asid = asid;
    swap_pool_table[frame_no].sw_page_no = vpn;
    swap_pool_table[frame_no].sw_pte = page_table + vpn_to_index(vpn);
}

static inline void update_page_table(page_table_t page_table, size_t index,
                                     memaddr frame_addr)
{
    page_table[index].pte_entry_lo = frame_addr | VALIDON | DIRTYON;
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

inline void support_tlb()
{
    support_t *const support = (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    if (support->sup_except_state[PGFAULTEXCEPT].cause == CAUSE_TLB_MOD)
        support_trap();
    else {
        SYSCALL(PASSEREN, (int)&sem_swap_pool_table, 0, 0);
        set_swap_pool_baton(support->sup_asid, true);
        state_t *const saved_state = &support->sup_except_state[PGFAULTEXCEPT];
        const int victim_frame = pick_page();
        const size_t victim_frame_addr =
            swap_pool_addr + victim_frame * PAGESIZE;
        pandos_kfprintf(&kdebug, "Victim: %p\n", victim_frame_addr);
        const size_t p_vpn = entryhi_to_vpn(saved_state->entry_hi),
                     p_index = vpn_to_index(p_vpn);

        const swap_t swap = swap_pool_table[victim_frame];
        if (check_frame_occupied(swap)) {
            const size_t k_vpn = entryhi_to_vpn(swap.sw_pte->pte_entry_hi),
                         k_index = vpn_to_index(k_vpn);

            // Atomically
            deactive_interrupts();
            mark_page_not_valid(support->sup_private_page_table, k_index);
            update_tlb(k_index, *swap.sw_pte);
            active_interrupts();

            const int write_status = write_flash(
                swap.sw_asid, k_index, (void *)page_addr(victim_frame));
            if (write_status != DEV_STATUS_READY) {
                pandos_kfprintf(
                    &kstderr,
                    "Error on flash write (k_index = %d, status = %d)\n",
                    k_index, write_status);
                support_trap();
            }
        }

        const int read_status =
            read_flash(support->sup_asid, p_index, (void *)victim_frame_addr);
        if (read_status != DEV_STATUS_READY) {
            pandos_kfprintf(&kstderr,
                            "Error on flash read (p_index = %d, status = %d)\n",
                            p_index, read_status);
            support_trap();
        }

        // Atomically
        deactive_interrupts();
        add_entry_swap_pool_table(victim_frame, support->sup_asid, p_vpn,
                                  support->sup_private_page_table);
        update_page_table(support->sup_private_page_table, p_index,
                          victim_frame_addr);
        update_tlb(p_index, support->sup_private_page_table[p_index]);
        active_interrupts();

        SYSCALL(VERHOGEN, (int)&sem_swap_pool_table, 0, 0);
        set_swap_pool_baton(support->sup_asid, false);

        load_state(saved_state);
    }
}
