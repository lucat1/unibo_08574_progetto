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

#include "arch/devices.h"
#include "os/types.h"

#define SEMAPHORES_NUM (DEVINTNUM + 1) * DEVPERINT + 1

extern int semaphores[SEMAPHORES_NUM];

extern scheduler_control_t P(int *const sem_addr, pcb_t *const p);
extern pcb_t *V(int *const sem_addr);
extern int *get_semaphore(int int_l, int dev_n, bool is_w);
extern int *get_timer_semaphore();
extern void init_semaphores();

#endif /* PANDOS_SEMAPHORE_H */
