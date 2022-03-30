/**
 * \file semaphore.c
 * \brief Device semaphore global values and public interface.
 *
 * \author Gianmaria Rovelli
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 17-03-2022
 */

#ifndef PANDOS_SEMAPHORE_H
#define PANDOS_SEMAPHORE_H
/* TODO: comment */

#include "os/types.h"

extern int disk_semaphores[DEVPERINT];
extern int tape_semaphores[DEVPERINT];
extern int ethernet_semaphores[DEVPERINT];
extern int printer_semaphores[DEVPERINT];
extern int termr_semaphores[DEVPERINT];
extern int termw_semaphores[DEVPERINT];
extern int timer_semaphore;

extern pcb_t *P(int *sem_addr, pcb_t *p);
extern pcb_t *V(int *sem_addr);
extern scheduler_control_t mask_V(pcb_t *p);
extern scheduler_control_t mask_P(pcb_t *p);
extern void init_semaphores();

#endif /* PANDOS_SEMAPHORE_H */
