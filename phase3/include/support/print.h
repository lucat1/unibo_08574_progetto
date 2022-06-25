#ifndef PANDOS_PRINT_H
#define PANDOS_PRINT_H

#include "os/types.h"

#define sysprintf(...) fsysprintf(0, __VA_ARGS__)

size_t fsysprintf(int termid, const char *fmt, ...);

#endif /* PANDOS_PRINT_H */
