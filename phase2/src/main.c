/**
 * \file main.c
 * \brief Entrypoint for the PandOS+ kernel.
 *
 * \author Luca Tagliavini
 * \date 17-03-2022
 */

#include "init.h"
#include "os/util.h"
#include "scheduler.h"
#include "umps/libumps.h"

int main(int argc)
{
    init();
    schedule();

    pandos_printf(":: scheduler quit. fatal\n");
    PANIC();
    return -1;
}
