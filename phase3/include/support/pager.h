#ifndef PANDOS_PAGER_H
#define PANDOS_PAGER_H

#include "os/ctypes.h"

extern size_t pick_page();
extern memaddr page_addr(size_t i);

#endif /* PANDOS_PAGER_H */
