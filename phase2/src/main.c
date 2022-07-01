/**
 * \file main.c
 * \brief Entrypoint for the PandOS+ kernel.
 *
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 17-03-2022
 */

#include "exception.h"
#include "os/const.h"
#include "os/init.h"
#include "os/puod.h"
#include "os/scheduler.h"
#include "os/util.h"
#include "support/pager.h"
#include "support/support.h"
#include "support/test.h"
#include <umps/libumps.h>

#define STACK_PAGE_NUMBER ((GETPAGENO >> VPNSHIFT) - KUSEG)

// TODO:
void tlb_refill_handler()
{
    // TODO: locate the appropriate page entry
    state_t *saved_state = (state_t *)BIOSDATAPAGE;
    size_t p = (saved_state->entry_hi >> VPNSHIFT) - KUSEG;
    if (p == STACK_PAGE_NUMBER)
        p = MAXPAGES - 1;
    pte_entry_t pte = active_process->p_support->sup_private_page_table[p];

    add_random_in_tlb(pte);
    LDST(saved_state);
}

int main(int argc)
{
    init((memaddr)tlb_refill_handler, (memaddr)exception_handler,
         (memaddr)test);
    schedule(NULL, false);
    return 1;
}
