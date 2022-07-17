/**
 * \file pager.c
 * \brief The Pager module
 *
 * \author Gianmaria Rovelli
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 26-06-2022
 */

#include "support/pager.h"
#include "os/const.h"
#include "os/scheduler.h"
#include "os/util.h"
#include "support/storage.h"
#include "support/support.h"

/** TLB-Modification exception. */
#define CAUSE_TLB_MOD 1

/** Swap Pool starting address (right after the OS .text and .data). */
#define SWAP_POOL_ADDR                                                         \
    (*(memaddr *)(KERNELSTACK + 0x0018) + *(memaddr *)(KERNELSTACK + 0x0024))

/** Type describing a Swap Pool Table as an array of Swap Pool Table entries. */
typedef swap_t swap_table_t[POOLSIZE];

/** Stores the value of \ref SWAP_POOL_ADDR. */
static memaddr swap_pool_addr;

/**
 * \brief Keeps track of whether each U-Proc has currently got access to the
 * Swap Pool table or not.
 * \remark We use different variables instead of a single one in order to avoid
 * race condition.
 */
static bool swap_pool_batons[UPROCMAX];

/** The Swap Pool Table describes the current state of the Swap Pool. */
static swap_table_t swap_pool_table;

/** A mutex for the Swap Pool Table. */
static int sem_swap_pool_table = 1;

/**
 * \brief Mark every entry in the Swap Pool Table as unoccupied.
 */
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

    // Compute how many pages from the beginning are in the .text area, and can
    // therefore be marked as read-only.
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

    // Initialize each Page Table entry.
    for (size_t i = 0; i < last_page_index; ++i) {
        page_table[i].pte_entry_hi =
            KUSEG + (i << VPNSHIFT) + (asid << ASIDSHIFT);
        page_table[i].pte_entry_lo = i < text_file_size ? 0 : DIRTYON;
    }
    // The last Page Table entry is actually the stack.
    page_table[last_page_index].pte_entry_hi =
        KUSEG + GETPAGENO + (asid << ASIDSHIFT);
    page_table[last_page_index].pte_entry_lo = DIRTYON;

    // Initialize the corresponding baton as well.
    swap_pool_batons[asid - 1] = false;

    return true;
}

/**
 * \brief Check whether an AISD is valid or not.
 * \param[in] asid The ASID to be checked.
 * \return True if the ASID is valid, false otherwise.
 */
static inline bool is_valid_asid(int asid)
{
    return 1 <= asid && asid <= UPROCMAX;
}

/**
 * \brief Update a Swap Pool Table baton with a new value.
 * \param[in] asid The ASID whose baton is to be updated.
 * \param[in] value The new value for the baton.
 */
static inline void set_swap_pool_baton(int asid, bool value)
{
    if (!is_valid_asid(asid)) {
        pandos_kfprintf(&kstderr,
                        "memory.c: set_swap_pool_baton: invalid asid\n");
        PANIC();
    }
    swap_pool_batons[asid - 1] = value;
}

/**
 * \brief Get the value of a Swap Pool Table baton.
 * \param[in] asid The ASID whose baton value is to be retrieved.
 * \return The value of the desired baton.
 */
static inline bool get_swap_pool_baton(int asid)
{
    if (!is_valid_asid(asid)) {
        pandos_kfprintf(&kstderr,
                        "memory.c: get_swap_pool_baton: invalid asid\n");
        PANIC();
    }
    return swap_pool_batons[asid - 1];
}

/**
 * \brief If the specified U-Proc is holding hostage the Swap Pool Table,
 * release the latter.
 * \param[in] asid The ASID of the U-proc.
 */
static inline void release_sem_swap_pool_table(int asid)
{
    if (get_swap_pool_baton(asid))
        SYSCALL(VERHOGEN, (int)&sem_swap_pool_table, 0, 0);
}

/**
 * \brief Mark each frame in the Swap Pool currently occupied by the specified
 * U-Proc as unoccupied.
 * \param[in] asid The ASID of the U-Proc.
 */
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

/**
 * \brief Translate a VPN to the corresponding Page Table entry index.
 * \param[in] vpn The VPN to be converted.
 * \return The result of the translation, i.e. a Page Table entry index.
 */
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

/**
 * \brief Translate a EntryHI to the corresponding VPN.
 * \param[in] entryhi The EntryHI to be converted.
 * \return The result of the translation, i.e. a VPN.
 */
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

/**
 * \brief Translate a EntryHI to the corresponding Page Table entry index.
 * \param[in] entryhi The EntryHI to be converted.
 * \return The result of the translation, i.e. a Page Table entry index.
 */
#define entryhi_to_index(entryhi) (vpn_to_index(entryhi_to_vpn(entryhi)))

/**
 * \brief Add a Page Table entry to a random position in the TLB.
 * \param[in] pte The Page Table entry to be added.
 */
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

/**
 * \brief Check whether the specified Page Table entry is in the TLB and, if
 * it's there, update CP0-Index. \param[in] pte The Page Table entry to look
 * for. \return True if the specified Page Table entry is in the TLB, false
 * otherwise.
 */
static inline bool check_in_tlb(pte_entry_t pte)
{
    setENTRYHI(pte.pte_entry_hi);
    TLBP();
    return !(getINDEX() & PRESENTFLAG);
}

/**
 * \brief Adds the specified Page Table entry using the CP0-Index as location.
 * \param[in] pte The Page Table entry to be added.
 */
static inline void update_tlb(pte_entry_t pte)
{
    if (check_in_tlb(pte)) {
        setENTRYHI(pte.pte_entry_hi);
        setENTRYLO(pte.pte_entry_lo);
        TLBWI();
    }
}

/**
 * Check whether the given frame is occupied or not.
 * \param[in] frame The frame to be checked.
 * \return True if the frame is occupied, false otherwise.
 */
static inline bool check_frame_occupied(swap_t frame)
{
    return frame.sw_asid != -1;
}

/**
 * \brief The Page Replacement Algorithm: picks a frame index from the swap pool
 * to be replaced.
 * \remark If there are any unoccupied frame, picks the first of those
 * (O(\ref POOLSIZE)). If every frame is occupied, uses a First In First
 * Out approach. \return The selected frame index.
 */
static inline size_t pick_page()
{
    // Unoccupied frames have priority
    for (int index = 0; index < POOLSIZE; ++index)
        if (!check_frame_occupied(swap_pool_table[index]))
            return index;
    // Fallback FIFO approach when every frame is already occupied
    static size_t i = -1;
    return i = (i + 1) % POOLSIZE;
}

/**
 * \brief Compute a Swap Pool page address.
 * \param[in] i The 0-based index of the page whose address is to be computed.
 * \return The computed address, or NULL if the index was invalid.
 */
static inline memaddr page_addr(size_t i)
{
    if (i < 0 || i >= POOLSIZE)
        return (memaddr)NULL;
    return swap_pool_addr + i * PAGESIZE;
}

/**
 * \brief Mark a given Page Table entry as invalid.
 * \param[in,out] page_table The Page Table where the entry is found.
 * \param[in] index The index of the Page Table entry.
 */
static inline void mark_page_not_valid(page_table_t page_table, size_t index)
{
    page_table[index].pte_entry_lo &= !VALIDON;
}

/**
 * \brief Add a new entry to the Swap Pool table.
 * \param[in] frame_no The 0-based index of the frame to be occupied.
 * \param[in] asid The ASID of the U-Proc.
 * \param[in] vpn The VPN.
 * \param[in] page_table The U-Proc's Page Table.
 */
static inline void add_entry_swap_pool_table(size_t frame_no, size_t asid,
                                             size_t vpn,
                                             page_table_t page_table)
{
    swap_pool_table[frame_no].sw_asid = asid;
    swap_pool_table[frame_no].sw_page_no = vpn;
    swap_pool_table[frame_no].sw_pte = page_table + vpn_to_index(vpn);
}

/**
 * \brief Update a Page Table entry.
 * \param[in,out] page_table The Page Table whose entry is to be updated.
 * \param[in] index The entry's 0-based index in the Page Table.
 * \param[in] frame_addr The new frame addr.
 */
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
        // TLB-Modification
        support_trap();
    else {
        // Gain mutual exclusion over the Swap Pool table.
        SYSCALL(PASSEREN, (int)&sem_swap_pool_table, 0, 0);
        set_swap_pool_baton(support->sup_asid, true);
        state_t *const saved_state = &support->sup_except_state[PGFAULTEXCEPT];
        // Pick a frame, i, from the Swap Pool. Which frame is selected is
        // determined by the Pandos page replacement algorithm.
        const int victim_frame = pick_page();
        const size_t victim_frame_addr =
            swap_pool_addr + victim_frame * PAGESIZE;
        pandos_kfprintf(&kdebug, "Victim: %p\n", victim_frame_addr);
        const size_t p_vpn = entryhi_to_vpn(saved_state->entry_hi),
                     p_index = vpn_to_index(p_vpn);

        const swap_t swap = swap_pool_table[victim_frame];
        // Determine if frame i is occupied.
        if (check_frame_occupied(swap)) {
            const size_t k_vpn = entryhi_to_vpn(swap.sw_pte->pte_entry_hi),
                         k_index = vpn_to_index(k_vpn);

            // Atomically
            deactive_interrupts();
            // Update process x’s Page Table: mark Page Table entry k as not
            // valid.
            mark_page_not_valid(support->sup_private_page_table, k_index);
            // Update the TLB, if needed. The TLB is a cache of the most
            // recently executed process’s Page Table entries.
            update_tlb(*swap.sw_pte);
            active_interrupts();

            // Write the contents of frame victim_frame
            // to the correct location on process x’s backing store/flash device
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

        //  Read the contents of the Current Process’s backing store/flash
        //  device logical page p into frame victim_frame.
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
        // Update the Swap Pool table’s entry.
        add_entry_swap_pool_table(victim_frame, support->sup_asid, p_vpn,
                                  support->sup_private_page_table);
        // Update the Current Process’s Page Table entry.
        update_page_table(support->sup_private_page_table, p_index,
                          victim_frame_addr);
        // Update the TLB.
        update_tlb(support->sup_private_page_table[p_index]);
        active_interrupts();

        // Release mutual exclusion over the Swap Pool table.
        SYSCALL(VERHOGEN, (int)&sem_swap_pool_table, 0, 0);
        set_swap_pool_baton(support->sup_asid, false);

        // Return control to the Current Process to retry the instruction that
        // caused the page fault: LDST on the saved exception state.
        load_state(saved_state);
    }
}
