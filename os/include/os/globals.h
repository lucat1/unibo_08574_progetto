/**
 * \file globals.h
 * \brief Global variables
 *
 * \author Alessandro Frau
 * \author Gianmaria Rovelli
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 30-03-2022
 */

#ifndef PANDOS_GLOBALS_H
#define PANDOS_GLOBALS_H

#include "os/types.h"

/* Number of started but not yet terminated processes. */
extern int running_count;

/* Number of processes softly blocked, that is they have been started but have
 * been blocked by either an I/O operation or a timer request. */
extern int blocked_count;

/* Tail pointer to a queue of processes that are in the ready state. */
extern list_head ready_queue_lo, ready_queue_hi;

/* Pointer to the currently running process */
extern pcb_t *active_process;

/* ???? */
extern pcb_t *last_process;

/* Process cpu time accumulator */
extern cpu_t start_tod;

extern state_t *wait_state;


/* Semaphores for each device */
extern int disk_semaphores[DEVPERINT];
extern int tape_semaphores[DEVPERINT];
extern int ethernet_semaphores[DEVPERINT];
extern int printer_semaphores[DEVPERINT];
extern int termr_semaphores[DEVPERINT];
extern int termw_semaphores[DEVPERINT];
extern int timer_semaphore;

#endif /* PANDOS_GLOBALS_H */