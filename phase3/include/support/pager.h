#ifndef PANDOS_PAGER_H
#define PANDOS_PAGER_H

#include "arch/processor.h"
#include "os/ctypes.h"
#include "os/types.h"

// la roba che va utilizzata da altre parti deve essere non static tipo
// page_addr

extern size_t pick_page();
extern memaddr page_addr(size_t i);
extern void add_random_in_tlb(pte_entry_t pte);
extern void update_tlb(size_t index, pte_entry_t pte);
extern bool check_in_tlb(pte_entry_t pte);
extern size_t page_num(memaddr entryhi);
extern void update_page_table(pte_entry_t page_table[], size_t page_no,
                              memaddr frame_addr);
extern bool check_frame_occupied(swap_t frame);
extern void mark_page_not_valid(pte_entry_t page_table[], size_t page_no);
extern void add_entry_swap_pool_table(size_t frame_no, size_t asid,
                                      size_t page_no, pte_entry_t page_table[]);
extern void active_interrupts();
extern void deactive_interrupts();

extern void tlb_exceptionhandler();

#endif /* PANDOS_PAGER_H */
