#ifndef PANDOS_PAGER_H
#define PANDOS_PAGER_H

#include "arch/processor.h"
#include "os/const.h"
#include "os/types.h"

#define STACK_PAGE_NUMBER (GETPAGENO >> VPNSHIFT)

typedef swap_t swap_table_t[MAXPAGES];
typedef pte_entry_t pte_entry_table[MAXPAGES];

extern swap_t swap_pool_table[POOLSIZE];
extern int swap_pool_sem;

extern bool init_page_table(pte_entry_table page_table, int asid);
extern void set_swap_pool_baton(int asid, bool value);
extern bool get_swap_pool_baton(int asid);
extern void support_tlb();
extern void tlb_refill_handler();
extern void release_sem_swap_pool_table();

extern void set_swap_pool_baton(int asid, bool value);
extern bool get_swap_pool_baton(int asid);

// controllare l'utilita' di questi export
extern size_t index_to_vpn(size_t index);
extern size_t vpn_to_index(size_t vpn);
extern size_t entryhi_to_vpn(memaddr entryhi);
extern size_t entryhi_to_index(memaddr entryhi);
extern size_t pick_page();
extern memaddr page_addr(size_t i);
extern void add_random_in_tlb(pte_entry_t pte);
extern void update_tlb(size_t index, pte_entry_t pte);
extern bool check_in_tlb(pte_entry_t pte);
extern void update_page_table(pte_entry_t page_table[], size_t page_no,
                              memaddr frame_addr);
extern bool check_frame_occupied(swap_t frame);
extern void mark_page_not_valid(pte_entry_t page_table[], size_t page_no);
extern void add_entry_swap_pool_table(size_t frame_no, size_t asid,
                                      size_t page_no, pte_entry_t page_table[]);
extern void active_interrupts();
extern void deactive_interrupts();
extern void release_sem_swap_pool_table();

extern void tlb_exceptionhandler();

#endif /* PANDOS_PAGER_H */
