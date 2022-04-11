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
#include "umps/arch.h"

#define PID_ID_MASK ((1 << MAX_PROC_BITS) - 1)

#define mask_pid_id(p) (p & PID_ID_MASK)
#define mask_recycle(p) (p >> MAX_PROC_BITS)

extern void reset_timer();
extern void reset_local_timer();

extern void scheduler_wait();
extern void scheduler_takeover();

#endif /* PANDOS_SCHEDULER_IMPL_H */
