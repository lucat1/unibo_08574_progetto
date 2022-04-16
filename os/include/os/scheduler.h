/**
 * \file scheduler.h
 * \brief Interfaces for the process scheduler.
 *
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 20-03-2022
 */

#ifndef PANDOS_OS_SCHEDULER_H
#define PANDOS_OS_SCHEDULER_H

#include "os/list.h"
#include "os/pcb.h"
#include "os/types.h"

extern size_t process_count;
extern size_t softblock_count;
extern list_head ready_queue_lo, ready_queue_hi;
extern pcb_t *active_process;
extern pcb_t *yield_process;
extern cpu_t start_tod;
extern state_t *wait_state;

#ifdef PANDOS_TESTING
size_t get_recycle_count();
#endif

/**
 * \brief Spawns a process and returns the allocated structure.
 * \param[in] priority The proprity of the spawned process. Either 1 for high or
 * 0 for low.
 * \return The allocated process descriptor.
 */
extern pcb_t *spawn_process(bool priority);

/**
 * \brief Recursively kills a process and all of its progeny. This procedure
 * abruptly comes to a halt on the first failed kill.
 * \param[in,out] p Pointer to the PCB whose progeny is to be killed.
 * \return 0 on success, 1 on NULL input, 2 if input could not be removed
 * from its parent, or 3 if any of its descendants could not be killed.
 */
extern int kill_progeny(pcb_t *const p);

/**
 * \brief Enqueues a process on the appropriate queue based on its priority.
 * \param[in,out] p Pointer to the PCB of the process to be enqueued: if NULL,
 * nothing happens.
 */
extern void enqueue_process(pcb_t *p);

/**
 * \brief Removes a given process from the queue it is currently in.
 * \param[in] p Pointer to the PCB of the process to be removed.
 * \return A pointer to the PCB of the removed process, or NULL on failure.
 */
extern pcb_t *dequeue_process(pcb_t *p);

/**
 * \brief Retrieves a process from its pid.
 * \param[in] pid The pid whose process is to be retrieved.
 * \return A pointer to the PCB of the desired process, or NULL if such a
 * process does not currently exist.
 */
extern pcb_t *const find_process(pandos_pid_t pid);

/**
 * \brief Initializes the state variables and the data structures used by the
 * scheduler.
 */
extern void init_scheduler();

/**
 * \brief Decides which process should be running next.
 * \param[in,out] pcb An optional pointer to a PCB describing a new process the
 * scheduler should now take in account.
 * \param[in] enqueue When the first parameter is specified, tells whether it
 * should be enqueued or immediately set as running process.
 */
void schedule(pcb_t *pcb, bool enqueue);

/**
 * \brief Prints a formatted message before making the kernel panic.
 * \param[in] fmt The format string to be printed as message.
 * \param[in] ... Additional parameters for the format string.
 */
void scheduler_panic(const char *fmt, ...);

#endif /* PANDOS_OS_SCHEDULER_H */
