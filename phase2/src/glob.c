/**
 * \file glob.c
 * \brief Global variables
 *
 * \author Luca Tagliavini
 * \date 17-03-2022
 */

#include "glob.h"
#include "os/pcb.h"

int running_count;
int blocked_count;
list_head ready_processes;
pcb_t *active_process;
int device_semaphores[DEVICE_SEMAPHORES_COUNT];

inline int devid() { return 0; }

inline void init_glob()
{
    running_count = 0;
    blocked_count = 0;
    mk_empty_proc_q(&ready_processes);
    active_process = NULL;
    for (int i = 0; i < DEVICE_SEMAPHORES_COUNT; ++i)
        device_semaphores[i] = 0;
}
