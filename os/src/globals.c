/** \file globals.c
 * \brief Global variables
 *
 * \author Alessandro Frau
 * \author Gianmaria Rovelli
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 17-01-2022
 */


#include "os/globals.h"



int running_count;
int blocked_count;
list_head ready_queue_lo, ready_queue_hi;
pcb_t *active_process;
pcb_t *last_process;
cpu_t start_tod;

state_t *wait_state;

/* Semaphores for each device */
int disk_semaphores[DEVPERINT];
int tape_semaphores[DEVPERINT];
int ethernet_semaphores[DEVPERINT];
int printer_semaphores[DEVPERINT];
int termr_semaphores[DEVPERINT];
int termw_semaphores[DEVPERINT];
int timer_semaphore;