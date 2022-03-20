/**
 * \file pcb.h
 * \brief PCB implementation
 *
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 20-03-2022
 */

#include "util.h"
#include "glob.h"
#include "os/pcb.h"
#include "umps/const.h"
#include "umps/libumps.h"

#define TIMER_VALUE 100
inline void reset_timer() { setTIMER(TIMER_VALUE * *(int *)(TIMESCALEADDR)); }

pcb_t *spawn_process()
{
    pcb_t *p = alloc_pcb();
    /* TODO: assign a pid, set priority to  */
    ++running_count;
    list_add(&p->p_list, &ready_queue);
    return p;
}
