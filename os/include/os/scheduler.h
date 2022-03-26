/**
 * \file schedule.h
 * \brief Interfaces for the process scheduler.
 *
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 20-03-2022
 */

#ifndef PANDOS_SCHEDULER_H
#define PANDOS_SCHEDULER_H

#include "os/list.h"
#include "os/pcb.h"
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

extern pcb_t *last_process;

extern pcb_t *V(int *sem_addr);
extern pcb_t *P(int *sem_addr, pcb_t *p);

/**
 * \brief Spawns a process and returns the allocated structure.
 * \param[in] priority The proprity of the spawned process. Either 1 for high or
 * 0 for low.
 * \return The allocated process descriptor.
 */
pcb_t *spawn_process(bool priority);
/* adds a process to the appropriate queue, does not change the running_count.
 * That is up to the caller */
extern void enqueue_process(pcb_t *p);
extern void dequeue_process(pcb_t *p);
void kill_process(pcb_t *p);

extern void scheduler_panic(const char *msg);

extern void scheduler_wait();
/* Externally implemented function to hand the processor over to a new
 * process
 */
extern void scheduler_takeover();

void init_scheduler();
void schedule();

#endif /* PANDOS_SCHEDULER_H */
