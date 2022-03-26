/**
 * \file syscall.h
 * \brief Syscalls.
 *
 * \author Alessandro Frau
 * \author Gianmaria Rovelli
 * \author Luca Tagliavini
 * \date 26-03-2022
 */

#ifndef PANDOS_SYSCALL_H
#define PANDOS_SYSCALL_H

#include "os/types.h"
#include "os/asl.h"
#include "os/util.h"
#include "syscall.h"
#include "semaphores.h"
#include "os/scheduler.h"
#include "umps/cp0.h"
#include "umps/types.h"
#include <umps/arch.h>
#include <umps/libumps.h>



static inline control_t P(int *sem_addr, pcb_t *p)
{
    if (*sem_addr > 0) {
        *sem_addr = *sem_addr - 1;
        return control_schedule;
    } else {
        /* TODO: dequeing here is useless if the p is the current_process */
        dequeue_process(p);
        int r = insert_blocked(sem_addr, p);

        if (r > 0) {
            pandos_kprintf("(::) PASSEREN error (%p)\n", r);
            PANIC();
        }

        return control_block;
    }
}

static inline control_t V(int *sem_addr)
{
    pcb_t *p = remove_blocked(sem_addr);
    if (p == NULL) { /* means that sem_proc is empty */
        *sem_addr = *sem_addr + 1;
        return control_preserve;
    } else {
        queue_process(p);
        return control_schedule;
    }
}

static inline control_t syscall_create_process()
{
    /* parameters of syscall */
    state_t *p_s = (state_t *)active_process->p_s.reg_a1;
    bool p_prio = (bool)active_process->p_s.reg_a2;
    support_t *p_support_struct = (support_t *)active_process->p_s.reg_a3;

    /* spawn new process */
    pcb_t *c = spawn_process(p_prio);

    /* checks if there are enough resources */
    if (c == NULL) { /* lack of resorces */
        pandos_kprintf(
            "(::) cannot create new process due to lack of resources\n");
        /* set caller's v0 to -1 */
        active_process->p_s.reg_v0 = -1;
        return control_preserve;
    } else{
        c->p_support = p_support_struct;
        c->p_s = *(p_s);
        /* p_time is already set to 0 from the alloc_pcb call inside spawn_process */
        /* p_sem_add is already set to NULL from the alloc_pcb call inside spawn_process */

        /* adds new process as child of caller process */
        insert_child(active_process, c);
        /* sets caller's v0 to new process pid */
        active_process->p_s.reg_v0 = c->p_pid;
        return control_schedule;
    }
}

/* TODO : generate interrupt to stop time slice */
static inline void syscall_terminate_process()
{
    /* Generate an interrupt to signal the end of Current Process’s time q
       antum/slice. The PLT is reserved for this purpose. */

    int pid = active_process->p_s.reg_a1;
    pcb_t *p = NULL;

    /* if pid is 0 then the target is the caller's process */
    if (pid == 0)
        p = active_process;
    else {
        /* TODO : finds pcb by pid */
    }

    /* checks that process with requested pid exists */
    if (p == NULL) {
        pandos_kprintf("(::) Could not terminate a NULL process\n");
        PANIC();
        return;
    }

    /* recursively removes progeny of active process */

    /* removes active process from parent's children */
    out_child(p);

    /* calls scheduler */
    kill_process(p);

    /* ??? */
    active_process->p_s.reg_v0 = pid;
}

/* TODO : NSYS4 */
static inline control_t syscall_verhogen()
{
    return V((int *)active_process->p_s.reg_a1);
}

/* TODO : NSYS3 */
static inline control_t syscall_passeren()
{
    /* TODO : Update the accumulated CPU time for the Current Process */
    /* TODO : update blocked_count ??? */
    return P((int *)active_process->p_s.reg_a1, active_process);
}

/* TODO : NSYS5 */
static inline control_t syscall_do_io()
{
    int *cmd_addr = (int *)active_process->p_s.reg_a1;
    int cmd_value = (int)active_process->p_s.reg_a2;

    /* TODO: chose the correct index and semaphore */
    int *sem_kind = termw_semaphores, i = 0;
    control_t ctrl = P(&sem_kind[i], active_process);

    /* TODO : now is hardcoded (7) */
    active_process->p_s.status |= STATUS_IM(7);

    /* Finally write the data */
    *cmd_addr = cmd_value;

    //termreg_t *d = (termreg_t *)cmd_addr-3;
    //int base = (int)((*cmd_addr - DEV_REG_START));
   //int status = *(base+2);

    /* TODO : now is hardcoded -1 */
    int acc = 0;
    while(acc < 1000){
        acc++;
    }
    /*pandos_kprintf("(::) v0 (%p)\n", *(cmd_addr-1));*/
    active_process->p_s.reg_v0 = *(cmd_addr-1);

    return ctrl;
}

static inline control_t syscall_get_process_id()
{
    int parent = (int) active_process->p_s.reg_a1;
    /* if parent then return parent pid, else return active process pid */
    if(!parent){
        active_process->p_s.reg_v0 = active_process->p_pid;
    }else{
        active_process->p_s.reg_v0 = active_process->p_parent->p_pid;
    }
    return control_preserve;
}


#endif /* PANDOS_SYSCALL_H */