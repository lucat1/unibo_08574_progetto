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

typedef struct iodev {
    int *semaphore;
    int interrupt_line;
} iodev_t;

extern iodev_t get_iodev(size_t *cmd_addr);
extern void status_il_on_all(size_t *prev);
extern void status_il_on(size_t *prev, int line);

/* tod = time of day */
extern void store_tod(int *time);
extern void load_interval_timer(int time);
extern void load_local_timer(int time);

#endif /* PANDOS_ARCH_DEVICES_H */
