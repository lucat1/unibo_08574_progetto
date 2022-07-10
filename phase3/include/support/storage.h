#ifndef PANDOS_STORAGE_H
#define PANDOS_STORAGE_H

#include "os/ctypes.h"
#include "os/types.h"

bool read_flash(unsigned int dev, size_t block, void *dest);
bool write_flash(unsigned int dev, size_t block, void *src);

#endif /* PANDOS_STORAGE_H */
