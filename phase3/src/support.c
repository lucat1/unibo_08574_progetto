#include "support/support.h"
#include "os/util.h"

int support_tbl() { return 0; }

int support_generic()
{
    pandos_kprintf("here");
    return 0;
}
