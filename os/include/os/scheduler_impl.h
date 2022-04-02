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

#define PID_ID_MASK (-1 >> (WORD_SIZE * 8 - MAX_PROC_BITS))
#define RECYCLE_MASK (-1 << MAX_PROC_BITS)

#define mask_pid_id(p) (p & PID_ID_MASK)
#define mask_recycle(p) (p & RECYCLE_MASK)

#endif /* PANDOS_SCHEDULER_IMPL_H */
