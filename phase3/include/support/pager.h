/**
 * \file pager.h
 * \brief The pager module
 *
 * \author Gianmaria Rovelli
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 26-06-2022
 */

#ifndef PANDOS_PAGER_H
#define PANDOS_PAGER_H

#include "arch/processor.h"
#include "os/const.h"

/** Type describing a Page Table as an array of Page Table Entries. */
typedef pte_entry_t pte_entry_table[MAXPAGES];

/**
 * \brief Allocate the Swap Pool and initialize the Swap Pool Table.
 */
extern void init_pager();

/**
 * \brief Initialize a Page Table for a new U-Proc.
 * \param[out] page_table The Page Table to be initialized.
 * \param[in] asid The ASID of the U-Proc.
 * \return True on success, false otherwise.
 */
extern bool init_page_table(pte_entry_table page_table, int asid);

/**
 * \brief Mark a U-Proc's frames in the Swap Pool Table as initialized, and
 * make him release the Swap Pool semaphore if necessary.
 * \param[in] asid The ASID of the U-Proc.
 */
extern void clean_after_uproc(int asid);

/**
 * \brief A handler for TLB-Refill events.
 */
extern void tlb_refill_handler();

/**
 * \brief A handler for TLB Management exceptions.
 */
extern void support_tlb();

/**
 * \brief Disable the interrups, starting a new critical session.
 */
extern void deactive_interrupts();

/**
 * \brief Enable interrupts back, ending the current critical session.
 */
extern void active_interrupts();

#endif /* PANDOS_PAGER_H */
