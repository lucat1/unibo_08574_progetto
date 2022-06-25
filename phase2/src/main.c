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
#include "support/support.h"
#include "support/test.h"
#include <umps/libumps.h>

int main(int argc)
{
    init((memaddr)support_tbl, (memaddr)exception_handler, (memaddr)test);
    schedule(NULL, false);
    return 1;
}
