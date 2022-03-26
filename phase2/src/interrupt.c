/**
 * \file interrupt.c
 * \brief Interrupt and trap handler
 *
 * TODO: Implement all the syscalls.
 * \author Alessandro Frau
 * \author Gianmaria Rovelli
 * \author Luca Tagliavini
 * \date 20-03-2022
 */

#include "interrupt.h"
#include "os/asl.h"
#include "os/pcb.h"
#include "os/scheduler.h"
#include "os/util.h"
#include "semaphores.h"
#include "umps/cp0.h"
#include "umps/types.h"
#include <umps/arch.h>
#include <umps/libumps.h>

typedef enum control {
    control_preserve, /* returns the control to the active_process */
    control_block,   /* the active_process has been blocked, so the scheduler is
                        called */
    control_schedule /* the scheduler will be called including the
                        active_process */
} control_t;

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

static inline bool V(int *sem_addr)
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

#define TERM0ADDR 0x10000254
/* TODO: now is hardcoded */
static inline control_t interrupt_handler()
{
    pandos_kprintf("(::) interrupt\n");

    /* sends back ACK */
    unsigned int *base = (unsigned int *)(TERM0ADDR);
    unsigned int *command = base + 3;
    int _ACK = 1;
    unsigned int value = _ACK;
    *command = value;

    /* The
newly unblocked pcb is enqueued back on the Ready Queue and control
is returned to the Current Process unless the newly unblocked process
has higher prority of the Current Process. */


    int *sem_kind = termw_semaphores, i = 0;
    return V(&sem_kind[i]);
}

static inline control_t tbl_handler()
{
    if (active_process->p_support == NULL)
        kill_process(active_process);
    else {
        support_t *s = active_process->p_support;
        pandos_kprintf("(::) handoff to support %d\n", s->sup_asid);
        /* TODO: tell the scheduler to handoff the control to s->sup_asid;
         * with the appropriate state found in s->sup_except_state ??????
         * read the docs i guess
         */
    }

    return control_schedule;
}

/* TODO: fill me */
static inline control_t trap_handler()
{
    pandos_kprintf("(::) trap\n");
    return control_schedule;
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

static inline control_t syscall_handler()
{
    const int id = (int)active_process->p_s.reg_a0;
    switch (id) {
        case CREATEPROCESS:
            pandos_kprintf("(::) syscall CREATEPROCESS\n");
            /* TODO */
            return syscall_create_process();
            break;
        case TERMPROCESS:
            pandos_kprintf("(::) syscall TERMPROCESS\n");
            /* TODO */
            break;
        case PASSEREN:
            pandos_kprintf("(::) syscall PASSEREN\n");
            /* TODO */
            return syscall_passeren();
            break;
        case VERHOGEN:
            pandos_kprintf("(::) syscall VERHOGEN\n");
            /* TODO */
            return syscall_verhogen();
            break;
        case DOIO:
            pandos_kprintf("(::) syscall DOIO\n");
            /* TODO */
            return syscall_do_io();
            break;
        case GETTIME:
            pandos_kprintf("(::) syscall GETTIME\n");
            /* TODO */
            break;
        case CLOCKWAIT:
            pandos_kprintf("(::) syscall CLOCKWAIT\n");
            /* TODO */
            break;
        case GETSUPPORTPTR:
            pandos_kprintf("(::) syscall GETSUPPORTPTR\n");
            /* TODO */
            break;
        case GETPROCESSID:
            pandos_kprintf("(::) syscall GETPROCESSID\n");
            return syscall_get_process_id();
            break;
        case YIELD:
            pandos_kprintf("(::) syscall YIELD\n");
            /* TODO */
            break;
        default:
            pandos_kprintf("(::) invalid system call %d\n", id);
            PANIC();
            break;
    }

    return control_schedule;
}

void exception_handler()
{
    state_t *p_s;
    control_t ctrl = control_schedule;

    p_s = (state_t *)BIOSDATAPAGE;
    memcpy(&active_process->p_s, p_s, sizeof(state_t));
    /* p_s.cause could have been used instead of getCAUSE() */
    switch (CAUSE_GET_EXCCODE(getCAUSE())) {
        case 0:
            ctrl = interrupt_handler();
            break;
        case 1:
        case 2:
        case 3:
            ctrl = tbl_handler();
            break;
        case 8:
            ctrl = syscall_handler();
            /* ALWAYS increment the PC to prevent system call loops */
            active_process->p_s.pc_epc += WORD_SIZE;
            active_process->p_s.reg_t9 += WORD_SIZE;
            break;
        default: /* 4-7, 9-12 */
            ctrl = trap_handler();
            break;
    }
    /* TODO: maybe rescheduling shouldn't be done all the time */
    /* TODO: increement active_pocess->p_time */
    switch (ctrl) {
        case control_preserve:
            LDST(&active_process->p_s);
            break;
        case control_block:
            schedule();
            break;
        case control_schedule:
            queue_process(active_process);
            schedule();
            break;
    }
}
