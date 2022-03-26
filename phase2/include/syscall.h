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

#include "os/asl.h"
#include "os/pcb.h"
#include "os/util.h"
#include "semaphores.h"
#include "os/scheduler.h"
#include "syscall.h"
#include "umps/cp0.h"
#include "umps/types.h"
#include <umps/arch.h>
#include <umps/libumps.h>

static inline pcb_t *P(int *sem_addr, pcb_t *p)
{
    if (*sem_addr > 0) {
        *sem_addr = *sem_addr - 1;
        return p;
    } else {
        /* TODO: dequeing here is useless if the p is the current_process */
        dequeue_process(p);
        int r = insert_blocked(sem_addr, p);

        if (r > 0) {
            pandos_kprintf("(::) PASSEREN error (%p)\n", r);
            PANIC();
        }

        return NULL;
    }
}

static inline pcb_t *V(int *sem_addr)
{
    pcb_t *p = remove_blocked(sem_addr);
    if (p == NULL) { /* means that sem_proc is empty */
        *sem_addr = *sem_addr + 1;
    } else {
        enqueue_process(p);
    }

    return p;
}

static inline control_t mask_V(pcb_t *p){
    if(p == NULL) return control_preserve;
    return control_schedule;
}

static inline control_t mask_P(pcb_t *p){
    if(p == NULL) return control_block;
    return control_schedule;
}

/* TODO: Maybe optimize this solution */
static inline void delete_progeny(pcb_t *p){
    list_head *myqueue = NULL;
    mk_empty_proc_q(myqueue);
    insert_proc_q(myqueue, p);
    while((p = remove_proc_q(myqueue)) != NULL){
        pcb_t *child;
        while((child = remove_child(p)) != NULL){
            insert_proc_q(myqueue, child);
        }
        kill_process(p);
    }
}

/* TODO: Optimize this implementation and change it when we change how the pid are generated */
static inline pcb_t* find_pcb_by_pid(list_head *list, int pid){
    pcb_t *pos;
    list_for_each_entry(pos, list, p_list){
        if(pos->p_pid == pid){
            return pos;
        }
    }
    return NULL;
}

static inline pcb_t* search_all_lists(int pid){
    pcb_t *param;
    if((param = find_pcb_by_pid(&ready_queue_lo, pid)) != NULL){
        return param;
    }
    if((param = find_pcb_by_pid(&ready_queue_hi, pid)) != NULL){
        return param;
    }
    if(pid == active_process->p_pid){
        return active_process;
    }
    
    /* TODO: Search on the semaphores !!!!!!!!!!!!!*/
    return NULL;
}

/* TODO: NSYS1 */
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
    } else {
        c->p_support = p_support_struct;
        c->p_s = *(p_s);
        /* p_time is already set to 0 from the alloc_pcb call inside
         * spawn_process */
        /* p_sem_add is already set to NULL from the alloc_pcb call inside
         * spawn_process */

        /* adds new process as child of caller process */
        insert_child(active_process, c);
        /* sets caller's v0 to new process pid */
        active_process->p_s.reg_v0 = c->p_pid;
        return control_schedule;
    }
}

/* TODO: finish testing NSYS2 */
/* TODO : generate interrupt to stop time slice */
static inline control_t syscall_terminate_process()
{
    /* Generate an interrupt to signal the end of Current Process’s time q
       antum/slice. The PLT is reserved for this purpose. */

    int pid = active_process->p_s.reg_a1;
    pcb_t *p = NULL;

    /* if pid is 0 then the target is the caller's process */
    if (pid == 0)
        p = active_process;
    else {
        p = search_all_lists(pid);
        if(p == NULL){
            pandos_kprintf("(::) Could not find a pcb with this pid");
        }
    }

    /* checks that process with requested pid exists */
    if (p == NULL) {
        pandos_kprintf("(::) Could not terminate a NULL process\n");
        PANIC();
        return control_schedule;
    }

    /* recursively removes progeny of process that should be terminated */
    delete_progeny(p);

    /* removes process that should be terminated from parent's children */
    out_child(p);

    /* calls scheduler */
    kill_process(p);

    /* ??? */
    active_process->p_s.reg_v0 = pid;
    return control_schedule;
}

/* NSYS3 */
static inline control_t syscall_passeren()
{
    /* TODO : Update the accumulated CPU time for the Current Process */
    /* TODO : update blocked_count ??? */
    return mask_P(P((int *)active_process->p_s.reg_a1, active_process));
}

/* NSYS4 */
static inline control_t syscall_verhogen()
{
    return mask_V(V((int *)active_process->p_s.reg_a1));
}

/* TODO : NSYS5 */
static inline control_t syscall_do_io()
{
    int *cmd_addr = (int *)active_process->p_s.reg_a1;
    int cmd_value = (int)active_process->p_s.reg_a2;

    /* FIND DEVICE NUMBER BY cmd_addr */
    int *base = GET_DEVICE_FROM_COMMAND(cmd_addr);
    int i_n = 0, d_n = 0;

    for (int i = 3; i < 3 + N_EXT_IL; i++) {
        for (int j = 0; j < N_DEV_PER_IL; j++) {
            int *a = (int *)DEV_REG_ADDR(i, j);
            if (a == base) {
                i_n = i;
                d_n = j;
                i = 3 + N_EXT_IL;
                break;
            }
        }
    }

    if (i_n == IL_TERMINAL) {
        pandos_kprintf(":: addr : (%p)\n", cmd_addr);
        pandos_kprintf(":: start : (%p)\n", (int *)DEV_REG_START);
        pandos_kprintf(":: base : (%p)\n", base);
        pandos_kprintf(":: c : (%p)\n", TERMINAL_GET_COMMAND_TYPE(cmd_addr));
        pandos_kprintf(":: device : (%p, %p)\n", i_n, d_n);

        int *sem_kind, i = d_n;
        if(TERMIMANL_CHECK_IS_WRITING(cmd_addr)) sem_kind = termw_semaphores;
        else sem_kind = termr_semaphores;
        pcb_t *p = P(&sem_kind[i], active_process);
        control_t ctrl = mask_P(p);

        active_process->p_s.status |= STATUS_IM(i_n);

        /* Finally write the data */
        *cmd_addr = cmd_value;

        return ctrl;
    }

    return control_preserve;
}

/* TODO: test NSYS6 */
static inline control_t syscall_get_cpu_time()
{
    active_process->p_s.reg_v0 = active_process->p_time;
    return control_schedule;
}

/* TODO: test NSYS7 */
static inline control_t syscall_wait_for_clock()
{
    return control_schedule;
}

/* TODO: test  NSYS8 */
static inline control_t syscall_get_support_data()
{
    //active_process->p_s.reg_v0 = active_process->p_support;
    return control_schedule;
}

/* TODO: test NSYS9 */
static inline control_t syscall_get_process_id()
{
    int parent = (int)active_process->p_s.reg_a1;
    /* if parent then return parent pid, else return active process pid */
    if (!parent) {
        active_process->p_s.reg_v0 = active_process->p_pid;
    } else {
        active_process->p_s.reg_v0 = active_process->p_parent->p_pid;
    }
    return control_preserve;
}

/* TODO: test NSYS10 */
static inline control_t syscall_yeld()
{
    /* TODO: The active process should note be in any queue so this action is useless
     * I'm leaving it just in case I'm wrong.
     */
    dequeue_process(active_process);


    /* TODO: Understand if this action should be made by the scheduler or by the syscall itself */
    enqueue_process(active_process);
    return control_schedule;
}

#endif /* PANDOS_SYSCALL_H */