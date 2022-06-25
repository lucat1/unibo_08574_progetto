#include "support/storage.h"
#include "os/util.h"
#include <umps/arch.h>

bool read_flash(unsigned int dev, unsigned int block, void *dest)
{
    dtpreg_t *reg = (dtpreg_t *)DEV_REG_ADDR(FLASHINT, dev);
    reg->data0 = (memaddr)dest;
    return SYSCALL(DOIO, (int)&reg->command, FLASHREAD, 0) != DEV_STATUS_RERROR;
}
