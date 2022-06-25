#ifndef PANDOS_STORAGE_H
#define PANDOS_STORAGE_H

#include "os/types.h"

bool read_flash(unsigned int dev, unsigned int block, void *dest);

#endif /* PANDOS_STORAGE_H */
