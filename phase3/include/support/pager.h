#ifndef PANDOS_PAGER_H
#define PANDOS_PAGER_H

#include "arch/processor.h"
#include "os/const.h"
#include "os/types.h"

typedef pte_entry_t pte_entry_table[MAXPAGES];

extern void init_pager();

extern bool init_page_table(pte_entry_table page_table, int asid);
extern void clean_after_uproc(int asid);

extern void tlb_refill_handler();
extern void support_tlb();

extern void active_interrupts();
extern void deactive_interrupts();

#endif /* PANDOS_PAGER_H */
