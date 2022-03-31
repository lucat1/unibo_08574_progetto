/**
 * \file exception_impl.h
 * \brief Implementation details for the exception handler.
 *
 * \author Alessandro Frau
 * \author Luca Tagliavini
 * \date 30-03-2022
 */

#ifndef PANDOS_EXCEPTION_IMPL_H
#define PANDOS_EXCEPTION_IMPL_H

#include "os/scheduler.h"

extern scheduler_control_t pass_up_or_die(int type);

#endif /* PANDOS_EXCEPTION_IMPL_H */
