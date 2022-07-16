#include "support/storage.h"
#include "os/const.h"
#include "os/ctypes.h"
#include "os/util.h"
#include "support/pager.h"
#include <umps/arch.h>

#define FLASHCMDSHIFT 8

int read_flash(unsigned int dev, size_t block, void *dest)
{
    deactive_interrupts();
    dtpreg_t *reg = (dtpreg_t *)DEV_REG_ADDR(FLASHINT, dev - 1);
    size_t cmd = FLASHREAD | ((block) << FLASHCMDSHIFT);
    reg->data0 = (memaddr)dest;
    const int res = SYSCALL(DOIO, (int)&reg->command, cmd, 0);
    // TODO: check status
    active_interrupts();
    return res;
}

int write_flash(unsigned int dev, size_t block, void *src)
{
    deactive_interrupts();
    dtpreg_t *reg = (dtpreg_t *)DEV_REG_ADDR(FLASHINT, dev - 1);
    size_t cmd = FLASHWRITE | ((block) << FLASHCMDSHIFT);
    reg->data0 = (memaddr)src;
    const int res = SYSCALL(DOIO, (int)&reg->command, cmd, 0);
    // TODO: check status
    active_interrupts();
    return res;
}
