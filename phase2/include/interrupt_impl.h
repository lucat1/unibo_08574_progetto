/**
 * \file interrupt_impl.h
 * \brief Implementation of internal init routines.
 *
 * \author Alessandro Frau
 * \author Gianmaria Rovelli
 * \author Luca Tagliavini
 * \date 26-03-2022
 *
 */
#ifndef PANDOS_INTERRUPT_IMPL_H
#define PANDOS_INTERRUPT_IMPL_H

#include "os/scheduler.h"
#include "os/types.h"

static inline scheduler_control_t mask_V(pcb_t *p)
{
    if (p == NULL)
        return CONTROL_PRESERVE(active_process);
    return CONTROL_RESCHEDULE;
}

static inline scheduler_control_t mask_P(pcb_t *p)
{
    if (p == NULL)
        return CONTROL_BLOCK;
    return CONTROL_RESCHEDULE;
}

#endif /* PANDOS_INTERRUPT_IMPL_H */
