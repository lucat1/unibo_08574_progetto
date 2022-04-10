#include "arch/devices.h"
#include "os/exception_impl.h"
#include "os/semaphores.h"
#include "umps/const.h"
#include "umps/libumps.h"
#include <umps/arch.h>
#include <umps/cp0.h>

inline iodev_t get_iodev(size_t *cmd_addr)
{
    iodev_t res = {NULL, 0};
    int sem_i = 0;
    int *base = GET_DEVICE_FROM_COMMAND(cmd_addr);

    for (int i = IL_DISK; i < 3 + N_EXT_IL; i++) {
        for (int j = 0; j < N_DEV_PER_IL; j++) {
            int *a = (int *)DEV_REG_ADDR(i, j);
            if (a == base) {
                res.interrupt_line = i;
                sem_i = j;
                i = 3 + N_EXT_IL;
                break;
            }
        }
    }

    if (res.interrupt_line == IL_DISK) {
        res.semaphore = disk_semaphores + sem_i;
    } else if (res.interrupt_line == IL_FLASH) {
        res.semaphore = tape_semaphores + sem_i;
    } else if (res.interrupt_line == IL_ETHERNET) {
        res.semaphore = ethernet_semaphores + sem_i;
    } else if (res.interrupt_line == IL_PRINTER) {
        res.semaphore = printer_semaphores + sem_i;
    } else if (res.interrupt_line == IL_TERMINAL) {
        if (TERMIMANL_CHECK_IS_WRITING((int *)cmd_addr))
            res.semaphore = termw_semaphores + sem_i;
        else
            res.semaphore = termr_semaphores + sem_i;
    }
    return res;
}

inline size_t il_mask(int line) { return STATUS_IM(line); }
inline size_t il_mask_all() { return STATUS_IM_MASK; }

inline void store_tod(int *time) { STCK(*time); }
inline void load_interval_timer(int time) { LDIT(time); }
inline void load_local_timer(int time) { setTIMER(TRANSLATE_TIME(time)); }
