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
#include "os/scheduler.h"
#include "os/asl.h"
#include "os/list.h"
#include "os/pcb.h"
#include "os/util.h"
#include "umps/libumps.h"

/* Semaphores for each device */
extern int disk_semaphores[DEVPERINT];
extern int tape_semaphores[DEVPERINT];
extern int ethernet_semaphores[DEVPERINT];
extern int printer_semaphores[DEVPERINT];
extern int termr_semaphores[DEVPERINT];
extern int termw_semaphores[DEVPERINT];
extern int timer_semaphore;

extern void init_semaphores();

extern pcb_t *V(int *sem_addr);
extern pcb_t *P(int *sem_addr, pcb_t *p);

#endif /* PANDOS_SEMAPHORE_H */
