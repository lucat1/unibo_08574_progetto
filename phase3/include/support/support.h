#ifndef PANDOS_SUPPORT_H
#define PANDOS_SUPPORT_H

#include "arch/processor.h"

extern void init_sys_semaphores();

extern void support_generic();

extern void support_trap();

extern void master_semaphore_p();

#endif /* PANDOS_SUPPORT_H */
