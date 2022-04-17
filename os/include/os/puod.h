/**
 * \file puod.h
 * \brief The pass up or die handler
 *
 * \author Alessandro Frau
 * \author Gianmaria Rovelli
 * \author Luca Tagliavini
 * \date 30-03-2022
 */

#ifndef PANDOS_OS_PUOD_H
#define PANDOS_OS_PUOD_H

#include "os/scheduler.h"

/**
 * \brief Passes the control up to the appropriate handler specified by the
 * active process, or kills the active process and all of its progeny if such a
 * handler does not exist.
 * \param[in] type Specifies which old state exception and which new context
 * should be used.
 * \return Always CONTROL_BLOCK.
 */
extern scheduler_control_t pass_up_or_die(memaddr type);

#endif /* PANDOS_OS_PUOD_H */
