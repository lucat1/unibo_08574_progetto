/**
 * \file semaphore.c
 * \brief Device semaphore management
 *
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 17-03-2022
 */

#include "semaphores.h"
#include "os/pcb.h"

// int disk_semaphores[DEVPERINT];
// int tape_semaphores[DEVPERINT];
// int ethernet_semaphores[DEVPERINT];
// int printer_semaphores[DEVPERINT];
// int termr_semaphores[DEVPERINT];
// int termw_semaphores[DEVPERINT];
// int timer_semaphore;

inline void init_semaphores()
{

    /* Semaphores */
    for (int i = 0; i < DEVPERINT; ++i)
        disk_semaphores[i] = tape_semaphores[i] = ethernet_semaphores[i] =
            printer_semaphores[i] = termr_semaphores[i] = termw_semaphores[i] =
                0;
    timer_semaphore = 0;
}
