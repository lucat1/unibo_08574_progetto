/**
 * \file glob.c
 * \brief Global variables
 *
 * \author Luca Tagliavini
 * \date 17-03-2022
 */

#ifndef PANDOS_GLOB_H
#define PANDOS_GLOB_H

#include "os/list.h"
#include "os/types.h"

/* Number of started but not yet terminated processes. */
extern int running_count;
/* Number of processes softly blocked, that is they have been started but have
 * been blocked by either an I/O operation or a timer request. */
extern int blocked_count;
/* Tail pointer to a queue of processes that are in the ready state. */
extern list_head ready_queue;
/* Pointer to the currently running process */
extern pcb_t *active_process;

/* Semaphores for each device */
extern int disk_semaphores[DEVPERINT];
extern int tape_semaphores[DEVPERINT];
extern int ethernet_semaphores[DEVPERINT];
extern int printer_semaphores[DEVPERINT];
extern int termr_semaphores[DEVPERINT];
extern int termw_semaphores[DEVPERINT];
extern int timer_semaphore;

/* TODO: figure out what parameters this function takes and write a logic to map
 * each available device into an index, using the minimum sized array possible
 */
extern int devid();
extern void init_glob();

#endif /* PANDOS_GLOB_H */
