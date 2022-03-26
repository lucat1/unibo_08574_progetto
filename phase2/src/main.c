/**
 * \file main.c
 * \brief Entrypoint for the PandOS+ kernel.
 *
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 17-03-2022
 */

#include "init.h"
#include "os/scheduler.h"
#include "os/util.h"
#include "umps/libumps.h"

int main(int argc)
{
    init();
    schedule();

    pandos_kfprintf(&kstderr, "!! PANIC: Scheduler quit\n");
    PANIC();
    return -1;
}
