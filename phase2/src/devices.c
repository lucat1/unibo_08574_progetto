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
#include "os/util.h"
#include "semaphores_impl.h"
#include "umps/types.h"
#include <umps/arch.h>
#include <umps/const.h>
#include <umps/cp0.h>
#include <umps/libumps.h>

bool is_in_line(int dev, int il)
{
    if (il == IL_TERMINAL)
        il *= 2;
    return (dev < ((il + 1 - IL_DISK) * DEVPERINT));
}

inline iodev_t get_iodev(size_t *cmd_addr)
{
    iodev_t res = {NULL, 0};
    int int_l = -1;
    int dev_n = ((unsigned int)cmd_addr - DEV_REG_START) / DEV_REG_SIZE;
    for (int i = IL_DISK; i < IL_TERMINAL + 1; i++) {
        if (is_in_line(dev_n, i)) {
            int_l = i;
            break;
        }
    }
    size_t *base = (size_t *)(DEV_REG_SIZE * dev_n + DEV_REG_START);
    bool is_w = false;
    res.interrupt_line = int_l;

    termreg_t *reg = (termreg_t *)base;
    if ((int)&reg->transm_command == (int)cmd_addr)
        is_w = true;

    res.semaphore = get_semaphore(int_l, dev_n % DEVPERINT, is_w);
    return res;
}

inline void status_il_on_all(size_t *prev) { *prev |= STATUS_IM_MASK; }
inline void status_il_on(size_t *prev, int line) { *prev |= STATUS_IM(line); }

inline void store_tod(int *time) { STCK(*time); }
inline void load_interval_timer(int time) { LDIT(time); }
inline void load_local_timer(int time) { setTIMER(TRANSLATE_TIME(time)); }
