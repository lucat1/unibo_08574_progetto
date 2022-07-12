/**
 * \file main.c
 * \brief Entrypoint for the PandOS+ kernel.
 *
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 17-03-2022
 */

#include "arch/processor.h"
#include "exception.h"
#include "os/const.h"
#include "os/init.h"
#include "os/puod.h"
#include "os/scheduler.h"
#include "os/util.h"
#include "support/pager.h"
#include "support/support.h"
#include "support/test.h"
#include "umps/arch.h"
#include <umps/libumps.h>

void tlb_refill_handler()
{
    state_t *saved_state = (state_t *)BIOSDATAPAGE;
    pandos_kprintf("tlb_refill_handler saved_state %p\n",
                   saved_state->entry_hi);
    size_t index = entryhi_to_index(saved_state->entry_hi);
    pandos_kprintf("tlb_refill of #%d -> %p done\n", index, saved_state);
    pte_entry_t pte = active_process->p_support->sup_private_page_table[index];

    add_random_in_tlb(pte);
    load_state(saved_state);
}

int main(int argc)
{
    init((memaddr)tlb_refill_handler, (memaddr)exception_handler,
         (memaddr)test);
    schedule(NULL, false);
    return 1;
}
