#ifndef PANDOS_MEMORY_H
#define PANDOS_MEMORY_H

#include "os/types.h"

typedef pte_entry_t pte_entry_table[MAXPAGES];

extern int swap_pool_sem;
extern swap_t swap_pool_table[POOLSIZE];

extern bool init_page_table(pte_entry_table page_table, int asid);

extern void set_swap_pool_baton(int asid, bool value);
extern bool get_swap_pool_baton(int asid);

#endif /* PANDOS_MEMORY_H */
