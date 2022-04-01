/**
 * \file main.c
 * \brief Entrypoint for the PandOS+ kernel.
 *
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 17-03-2022
 */

#include <umps/libumps.h>

#include "os/scheduler.h"
#include "os/util.h"

#include "init.h"

int main(int argc)
{
    init();
    schedule(NULL, false);

    scheduler_panic("Scheduler quit\n");
    return -1;
}
