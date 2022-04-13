/**
 * \file semaphores.h
 * \brief Device semaphore global values and public interface.
 *
 * \author Gianmaria Rovelli
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 17-03-2022
 */

#ifndef PANDOS_OS_SEMAPHORE_H
#define PANDOS_OS_SEMAPHORE_H

#include "arch/devices.h"
#include "os/const.h"
#include "os/scheduler.h"

extern scheduler_control_t P(int *const sem_addr, pcb_t *const p);
extern pcb_t *V(int *const sem_addr);

extern void init_semaphores();
extern int *get_semaphore(int int_l, int dev_n, bool is_w);
extern int *get_timer_semaphore();

#endif /* PANDOS_OS_SEMAPHORE_H */
