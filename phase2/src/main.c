/**
 * \file main.c
 * \brief Entrypoint for the PandOS+ kernel.
 *
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 17-03-2022
 */

#include "exception.h"
#include "os/init.h"
#include "os/scheduler.h"
#include "os/util.h"
#include "p2test.h"
#include "support/support.h"
#include <umps/libumps.h>

int main(int argc)
{
    init((memaddr)uTLB_RefillHandler, (memaddr)support_generic, (memaddr)test);
    schedule(NULL, false);
    return 1;
}
