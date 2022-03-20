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

#define DEVICE_SEMAPHORES_COUNT 10

/* Number of started but not yet terminated processes. */
extern int running_count;
/* Number of processes softly blocked, that is they have been started but have
 * been blocked by either an I/O operation or a timer request. */
extern int blocked_count;
/* Tail pointer to a queue of processes that are in the ready state. */
extern list_head ready_processes;
/* Pointer to the currently running process */
extern pcb_t *active_process;
/* A semaphore value for each device. If a device doesn't have an active
 * semaphore on it the value in the array is NULL. The right index for a given
 * device can be computed via the `devid` procedure.
 */
extern int device_semaphores[DEVICE_SEMAPHORES_COUNT];

/* TODO: figure out what parameters this function takes and write a logic to map
 * each available device into an index, using the minimum sized array possible
 */
extern int devid();
extern void init_glob();

#endif /* PANDOS_GLOB_H */
