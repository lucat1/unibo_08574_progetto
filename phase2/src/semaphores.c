/**
 * \file semaphore.c
 * \brief Device semaphore management
 *
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 17-03-2022
 */

#include "semaphores.h"
#include "os/pcb.h"

int disk_semaphores[DEVPERINT];
int tape_semaphores[DEVPERINT];
int ethernet_semaphores[DEVPERINT];
int printer_semaphores[DEVPERINT];
int termr_semaphores[DEVPERINT];
int termw_semaphores[DEVPERINT];
int timer_semaphore;

inline pcb_t *P(int *sem_addr, pcb_t *p)
{
    if (*sem_addr > 0) {
        *sem_addr = *sem_addr - 1;
        return p;
    } else {
        /* TODO: dequeing here is useless if the p is the current_process */
        dequeue_process(p);
        int r = insert_blocked(sem_addr, p);

        if (r > 0) {

            if(r == 3 && p->p_sem_add == sem_addr) {
                stderr("PASSEREN same addr %d\n", r);
                PANIC();
            }else  {
            //scheduler_panic("PASSEREN failed %d");
                stderr("PASSEREN failed %d\n", r);
                PANIC();
            }
        }

        return NULL;
    }
}

inline pcb_t *V(int *sem_addr)
{
    pcb_t *p = remove_blocked(sem_addr);
    if (p == NULL) { /* means that sem_proc is empty */
        *sem_addr = *sem_addr + 1;
    } else {
        enqueue_process(p);
    }

    return p;
}

inline void init_semaphores()
{

    /* Semaphores */
    for (int i = 0; i < DEVPERINT; ++i)
        disk_semaphores[i] = tape_semaphores[i] = ethernet_semaphores[i] =
            printer_semaphores[i] = termr_semaphores[i] = termw_semaphores[i] =
                0;
    timer_semaphore = 0;
}
