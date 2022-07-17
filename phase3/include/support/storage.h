#ifndef PANDOS_STORAGE_H
#define PANDOS_STORAGE_H

#include "os/ctypes.h"
#include "os/types.h"

extern int read_flash(unsigned int dev, size_t block, void *dest);
extern int write_flash(unsigned int dev, size_t block, void *src);

#endif /* PANDOS_STORAGE_H */
