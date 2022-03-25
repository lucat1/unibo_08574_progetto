/**
 * \file semaphore.c
 * \brief Device semaphore global values.
 *
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 17-03-2022
 */

#ifndef PANDOS_SEMAPHORE_H
#define PANDOS_SEMAPHORE_H

#include "os/types.h"

/* Semaphores for each device */
int disk_semaphores[DEVPERINT];
int tape_semaphores[DEVPERINT];
int ethernet_semaphores[DEVPERINT];
int printer_semaphores[DEVPERINT];
int termr_semaphores[DEVPERINT];
int termw_semaphores[DEVPERINT];
int timer_semaphore;

extern void init_semaphores();

#endif /* PANDOS_SEMAPHORE_H */
