#include "support/storage.h"
#include "os/const.h"
#include "os/ctypes.h"
#include "os/util.h"
#include <umps/arch.h>

#define FLASHCMDSHIFT 8

bool read_flash(unsigned int dev, size_t block, void *dest)
{
    dtpreg_t *reg = (dtpreg_t *)DEV_REG_ADDR(FLASHINT, dev);
    size_t cmd = FLASHREAD | ((block) << FLASHCMDSHIFT);
    reg->data0 = (memaddr)dest;
    pandos_kprintf("Reading memory\n");
    size_t res = SYSCALL(DOIO, (int)&reg->command, cmd, 0) != DEV_STATUS_RERROR;
    pandos_kprintf("Reading memory\n");
    return res;
}

bool write_flash(unsigned int dev, size_t block, void *src)
{
    dtpreg_t *reg = (dtpreg_t *)DEV_REG_ADDR(FLASHINT, dev);
    size_t cmd = FLASHWRITE | ((block) << FLASHCMDSHIFT);
    reg->data0 = (memaddr)src;
    return SYSCALL(DOIO, (int)&reg->command, cmd, 0) != DEV_STATUS_RERROR;
}
