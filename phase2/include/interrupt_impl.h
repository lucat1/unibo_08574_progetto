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
#include "os/scheduler.h"

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
