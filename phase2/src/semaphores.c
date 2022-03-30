/**
 * \file semaphore.c
 * \brief Device semaphore management
 *
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 17-03-2022
 */
#include "os/globals.h"
#include "semaphores.h"



inline void init_semaphores()
{

    /* Semaphores */
    for (int i = 0; i < DEVPERINT; ++i)
        disk_semaphores[i] = tape_semaphores[i] = ethernet_semaphores[i] =
            printer_semaphores[i] = termr_semaphores[i] = termw_semaphores[i] =
                0;
    timer_semaphore = 0;
}
