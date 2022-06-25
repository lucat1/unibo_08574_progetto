#ifndef PANDOS_MEMORY_IMPL_H
#define PANDOS_MEMORY_IMPL_H

#include "os/const.h"
#include "os/types.h"

typedef pte_entry_t pte_entry_table[MAXPAGES];

typedef swap_t swap_table_t[MAXPAGES];
extern swap_t *swap_pool;

extern int swap_pool_sem;

extern bool init_page_table(pte_entry_table page_table, int asid);

#endif /* PANDOS_MEMORY_IMPL_H */
