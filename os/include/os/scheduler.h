/**
 * \file scheduler.h
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

extern int running_count;
extern int blocked_count;
extern list_head ready_queue_lo, ready_queue_hi;
extern pcb_t *active_process;
extern pcb_t *last_process;
extern cpu_t start_tod;
extern state_t *wait_state;

/**
 * \brief Spawns a process and returns the allocated structure.
 * \param[in] priority The proprity of the spawned process. Either 1 for high or
 * 0 for low.
 * \return The allocated process descriptor.
 */
extern pcb_t *spawn_process(bool priority);
extern void kill_process(pcb_t *p);

/* adds a process to the appropriate queue, does not change the running_count.
 * That is up to the caller */
extern void enqueue_process(pcb_t *p);
extern void dequeue_process(pcb_t *p);
extern const pcb_t *find_process(pandos_pid_t pid);

extern void scheduler_panic(const char *fmt, ...);
extern void scheduler_wait();
extern void scheduler_unlock();

/* Externally implemented function to hand the processor over to a new
 * process
 */
extern void scheduler_takeover();
extern bool is_ready_queue_empty();
extern pcb_t *get_first_pcb_ready();

extern void init_scheduler();
extern void scheduler_on_empty_queues();
void schedule(pcb_t *pcb, bool enqueue);

#endif /* PANDOS_SCHEDULER_H */
