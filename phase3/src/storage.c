#include "support/storage.h"
#include "os/const.h"
#include "os/ctypes.h"
#include "os/util.h"
#include "support/pager.h"
#include <umps/arch.h>

#define FLASHCMDSHIFT 8

char data[4 * 1024];

bool read_flash(unsigned int dev, size_t block, void *dest)
{
    deactive_interrupts();
    dtpreg_t *reg = (dtpreg_t *)DEV_REG_ADDR(FLASHINT, dev);
    size_t cmd = FLASHREAD | ((block) << FLASHCMDSHIFT);
    reg->data0 = (memaddr)dest;
    size_t res = SYSCALL(DOIO, (int)&reg->command, cmd, 0) != DEV_STATUS_RERROR;
    active_interrupts();
    return res;
}

bool write_flash(unsigned int dev, size_t block, void *src)
{
    deactive_interrupts();
    dtpreg_t *reg = (dtpreg_t *)DEV_REG_ADDR(FLASHINT, dev);
    size_t cmd = FLASHWRITE | ((block) << FLASHCMDSHIFT);
    reg->data0 = (memaddr)src;
    size_t res = SYSCALL(DOIO, (int)&reg->command, cmd, 0) != DEV_STATUS_RERROR;
    active_interrupts();
    return res;
}
