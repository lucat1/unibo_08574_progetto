/**
 * \file devices.h
 * \brief Architecture-dependant functions to interact with various devices.
 *
 * \author Luca Tagliavini
 * \date 9-04-2022
 */

#ifndef PANDOS_ARCH_DEVICES_H
#define PANDOS_ARCH_DEVICES_H

#include "os/ctypes.h"

#ifdef __x86_64__
#define DEVINTNUM 5
#define DEVPERINT 8
#else
#include <umps/const.h>
#endif

/** A descriptor for a I/O device. */
typedef struct iodev {
    /** The semaphore associated to this device. */
    int *semaphore;
    /** The interrupt line associated to this device. */
    int interrupt_line;
} iodev_t;

/**
 * \brief Retrieves the appropriate descriptor for the desired I/O device.
 * \param[in] cmd_addr The address of the command from which to infer the
 * device.
 * \return The desired I/O device descriptor.
 */
extern iodev_t get_iodev(size_t *cmd_addr);

/**
 * \brief Activates every interrupt line on a given status.
 * \param[in,out] prev The status to be updated.
 */
extern void status_il_on_all(size_t *prev);

/**
 * \brief Activates a given line on a given status.
 * \param[in,out] prev The status to be updated.
 * \param[in] line The line to be activated.
 */
extern void status_il_on(size_t *prev, int line);

/**
 * \brief Stores the time of day.
 * \param[out] time The address of the variable where the time of day is to be
 * stored.
 */
extern void store_tod(int *time);

/**
 * \brief Loads a new value for the interval timer.
 * \param[in] time The new value for the interval timer.
 */
extern void load_interval_timer(int time);

/**
 * \brief Loads a new value for the local timer.
 * \param[in] time The new value for the local timer.
 */
extern void load_local_timer(int time);

#endif /* PANDOS_ARCH_DEVICES_H */
