/**
 * \file semaphores.c
 * \brief Gather device semaphore data specific to the architecture.
 *
 * \author Gianmaria Rovelli
 * \date 13-04-2022
 */

#include "os/semaphores.h"
#include "os/asl.h"
#include "os/list.h"
#include "os/util.h"
#include "semaphores_impl.h"
#include <umps/arch.h>

static int semaphores[SEMAPHORES_NUM];

inline int *get_semaphores() { return semaphores; }

inline int *get_semaphore(int int_l, int dev_n, bool is_w)
{
    int sem;

    if (int_l > IL_TERMINAL || int_l < IL_DISK)
        scheduler_panic("Semaphore not found\n");

    sem = (int_l - IL_DISK) * DEVPERINT;
    if (int_l == IL_TERMINAL) {
        sem += 2 * dev_n + (is_w ? 1 : 0);
    } else {
        sem += dev_n;
    }

    return &semaphores[sem];
}

inline int *get_timer_semaphore() { return &semaphores[SEMAPHORES_NUM - 1]; }

inline void init_semaphores()
{
    for (int i = 0; i < SEMAPHORES_NUM; i++)
        semaphores[i] = 0;
}
