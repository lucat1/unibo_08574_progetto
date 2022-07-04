#include "support/support.h"
#include "os/util.h"
#include "support/pager.h"

// TODO : uTLB RefillHandler
void support_tlb()
{
    pandos_kprintf("!!!!!support_tlb\n");
    tlb_exceptionhandler();
}

// TODO
inline void support_generic() { pandos_kprintf("here\n"); }
