#include "support/memory.h"
#include "support/memory_impl.h"

#define PAGE_TABLE_ENTRY_LOW 5

int swap_pool_sem;
swap_t swap_pool_table[POOLSIZE];

inline bool init_page_table(pte_entry_table page_table, int asid)
{
    if (page_table == NULL || asid <= 0 || asid > UPROCMAX)
        return false;

    const size_t last_page_index = MAXPAGES - 1;

    for (size_t i = 0; i < last_page_index; ++i) {
        page_table[i].pte_entry_hi =
            KUSEG + (i << VPNSHIFT) + (asid << ASIDSHIFT);
        page_table[i].pte_entry_lo = PAGE_TABLE_ENTRY_LOW;
    }
    page_table[last_page_index].pte_entry_hi = GETPAGENO + (asid << ASIDSHIFT);
    page_table[last_page_index].pte_entry_lo = PAGE_TABLE_ENTRY_LOW;

    return true;
}
