/**
 * \file puod.h
 * \brief The pass up or die handler
 *
 * \author Alessandro Frau
 * \author Luca Tagliavini
 * \date 30-03-2022
 */

#ifndef PANDOS_OS_PUOD_H
#define PANDOS_OS_PUOD_H

#include "os/scheduler.h"

extern scheduler_control_t pass_up_or_die(memaddr type);

#endif /* PANDOS_OS_PUOD_H */
