#include "support/memory.h"
#include "support/memory_impl.h"

#define PAGE_TABLE_ENTRY_LOW 5

int swap_pool_sem;
swap_t swap_pool_table[POOLSIZE];

static bool swap_pool_batons[UPROCMAX];

inline bool init_page_table(pte_entry_table page_table, int asid)
{
    if (page_table == NULL || asid <= 0 || asid > UPROCMAX)
        return false;

    const size_t last_page_index = MAXPAGES - 1;

    for (size_t i = 0; i < last_page_index; ++i) {
        page_table[i].pte_entry_hi =
            KUSEG + (i << VPNSHIFT) + (asid << ASIDSHIFT);
        page_table[i].pte_entry_lo = DIRTYON;
    }
    page_table[last_page_index].pte_entry_hi =
        KUSEG + GETPAGENO + (asid << ASIDSHIFT);
    page_table[last_page_index].pte_entry_lo = DIRTYON;

    return true;
}

inline static bool is_valid_asid(int asid)
{
    return 0 < asid && asid <= UPROCMAX;
}

inline void set_swap_pool_baton(int asid, bool value)
{
    if (!is_valid_asid(asid)) {
        pandos_kfprintf(&kstderr,
                        "memory.c: set_swap_pool_baton: invalid asid\n");
        PANIC();
    }
    swap_pool_batons[asid - 1] = value;
}

inline bool get_swap_pool_baton(int asid)
{
    if (!is_valid_asid(asid)) {
        pandos_kfprintf(&kstderr,
                        "memory.c: get_swap_pool_baton: invalid asid\n");
        PANIC();
    }
    return swap_pool_batons[asid - 1];
}
