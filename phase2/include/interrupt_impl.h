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

#include "os/types.h"

typedef enum control {
    control_preserve, /* returns the control to the active_process */
    control_block,   /* the active_process has been blocked, so the scheduler is
                        called */
    control_schedule /* the scheduler will be called including the
                        active_process */
} control_t;

static inline control_t mask_V(pcb_t *p)
{
    if (p == NULL)
        return control_preserve;
    return control_schedule;
}

static inline control_t mask_P(pcb_t *p)
{
    if (p == NULL)
        return control_block;
    return control_schedule;
}

#endif /* PANDOS_INTERRUPT_IMPL_H */
