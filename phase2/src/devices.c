/**
 * \file devices.c
 * \brief Implementation of architecture-specific device functionality.
 *
 * \author Luca Tagliavini
 * \author Gianmaria Rovelli
 * \date 10-04-2022
 */

#include "arch/devices.h"
#include "os/semaphores.h"
#include "semaphores_impl.h"
#include <umps/arch.h>
#include <umps/const.h>
#include <umps/cp0.h>
#include <umps/libumps.h>

inline iodev_t get_iodev(size_t *cmd_addr)
{
    iodev_t res = {NULL, 0};
    int dev_n = GET_DEVICE_NUMBER_FROM_COMMAND(cmd_addr);
    int int_l = GET_INTERRUPT_LINE_FROM_DEVICE(dev_n);
    int sem_i = dev_n;
    /* first devices */
    const int T = (IL_TERMINAL - IL_DISK) * DEVPERINT;
    /* if it is terminal */
    if (dev_n > T)
        sem_i = (TERMINAL_CHECK_IS_WRITING(cmd_addr) ? dev_n + 1 : dev_n) +
                (dev_n - T);

    res.interrupt_line = int_l;
    res.semaphore = get_semaphores() + sem_i;

    return res;
}

inline void status_il_on_all(size_t *prev) { *prev |= STATUS_IM_MASK; }
inline void status_il_on(size_t *prev, int line) { *prev |= STATUS_IM(line); }

inline void store_tod(int *time) { STCK(*time); }
inline void load_interval_timer(int time) { LDIT(time); }
inline void load_local_timer(int time) { setTIMER(TRANSLATE_TIME(time)); }
