/**
 * \file interrupt.h
 * \brief Implementation of internal init routines.
 *
 * \author Luca Tagliavini
 * \date 20-03-2022
 *
 */
#ifndef PANDOS_INTERRUPT_H
#define PANDOS_INTERRUPT_H

#include "os/scheduler.h"

void exception_handler();
scheduler_control_t pass_up_or_die(int type);

#endif /* PANDOS_INTERRUPT_H */
