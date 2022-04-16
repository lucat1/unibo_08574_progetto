/**
 * \file scheduler_impl.h
 * \brief Implementation details for the scheduler.
 *
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 01-04-2022
 */

#ifndef PANDOS_SCHEDULER_IMPL_H
#define PANDOS_SCHEDULER_IMPL_H

#include "os/const.h"
#include "os/pcb.h"

#define PID_ID_MASK ((1 << MAX_PROC_BITS) - 1)

#define make_pid(id, recycle_count)                                            \
    (((id) | ((recycle_count) << MAX_PROC_BITS)) + 1)
#define mask_pid_id(p) (((p)-1) & PID_ID_MASK)
#define mask_recycle(p) (((p)-1) >> MAX_PROC_BITS)

extern void reset_timer();
extern void reset_local_timer();

#ifdef PANDOS_TESTING
/**
 * \brief Kills a single process by removing it from any queue or semaphore,
 * decrementing the process count and freeing the associated pcb. The caller
 * does not own the provided memory address after this call.
 * \param[in,out] p Pointer to the PCB to be killed.
 * \return 0 on success, 1 on NULL input, 2 if input could not be removed from
 * its parent.
 */
int kill_process(pcb_t *const p);
#endif

#endif /* PANDOS_SCHEDULER_IMPL_H */
