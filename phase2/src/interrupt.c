/**
 * \file interrupt.c
 * \brief Interrupt and trap handler
 *
 * TODO: fill me
 * \author Pino Pallino
 * \date 20-03-2022
 */

#include "interrupt.h"
#include "os/scheduler.h"
#include "os/util.h"
#include "os/pcb.h"
#include "os/asl.h"
#include "semaphore.h"
#include "umps/cp0.h"
#include <umps/arch.h>
#include <umps/libumps.h>

static inline bool P(int *sem_addr, pcb_t *p) {
    if(*sem_addr > 0) {
        *sem_addr = *sem_addr-1;
        return true;
    }else {
        int r = insert_blocked(sem_addr, p);

        if(r > 0) {
            pandos_kprintf("(::) PASSEREN error (%p)\n", r);
            PANIC();
        }

        return false;
    }
}

static inline bool V(int *sem_addr) {
    pcb_t *p = remove_blocked(sem_addr);
    if(p == NULL) { /* means that sem_proc is empty */
        *sem_addr = *sem_addr+1;
        return true;
    }else {
        queue_process(p);
        return false;
    }
}

/* TODO: fill me */
static inline bool interrupt_handler() { 
    pandos_kprintf("(::) iterrupt\n"); 
    
    return false;
}

static inline bool tbl_handler()
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

    return false;
}

/* TODO: fill me */
static inline bool trap_handler() { 
    pandos_kprintf("(::) trap\n"); 
    return false;
}



static inline void syscall_create_process(){
    /* parameters of syscall */
    state_t *p_s = (state_t *)active_process->p_s.reg_a1;
    int p_prio = (int) active_process->p_s.reg_a2;
    support_t *p_supportStruct = (support_t *)active_process->p_s.reg_a3;

    /* spawn new process */
    pcb_t *c = spawn_process(p_prio);
    c->p_support = p_supportStruct;
    c->p_s = *(p_s);

    /* checks if there are enough resources */
    if (c == NULL) { /* lack of resorces */
        pandos_kprintf("(::) cannot create new process due to lack of resouces\n");
        /* set caller's v0 to -1 */
        active_process->p_s.reg_v0 = -1;
        PANIC();
        return;
    }

    /* adds new process to process queue */
    insert_proc_q(&active_process->p_list , c);

    /* adds new process as child of caller process */
    insert_child(active_process , c);

    /* sets caller's v0 to new process pid */
    active_process->p_s.reg_v0 = c->p_pid;
}

/* TODO : generate interrupt to stop time slice */
static inline void syscall_terminate_process() {
    /* Generate an interrupt to signal the end of Current Process’s time quantum/slice. 
       The PLT is reserved for this purpose. */

    int pid = active_process->p_s.reg_a1;
    pcb_t *p = NULL;

    /* if pid is 0 then the target is the caller's process */
    if(pid == 0) p = active_process;
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
static inline bool syscall_verhogen () {
    int *sem_addr = (int *)active_process->p_s.reg_a1;

    return V(sem_addr);
}

/* TODO : NSYS3 */
static inline bool syscall_passeren () {
    int *sem_addr = (int *)active_process->p_s.reg_a1;   

    /* TODO : Update the accumulated CPU time for the Current Process */

    return P(sem_addr, active_process);

    /* TODO : update blocked_count ??? */

    //pcb_t *p = remove_blocked(sem_addr);

    //queue_process(p);
}


/* TODO : NSYS5 */
static inline bool syscall_do_io () {
    int *cmd_addr = (int *)active_process->p_s.reg_a1;
    int cmd_value = (int)active_process->p_s.reg_a2;

    /* TEMP : to be removed */
    unsigned int *addr = (unsigned int *)cmd_addr-1;

    *cmd_addr = cmd_value;

    //devreg_t *dev = (devreg_t *)addr;
    //unsigned int *trans_command = addr + 12;
    pandos_kprintf("(::) TERM (%p)\n", addr);

    int a = DEV_REG_ADDR(7, 0);
    pandos_kprintf("(::) addr (%p)\n", a);

    P(termw_semaphores[0], active_process);

    /* returns control */
    return true;

    //active_process->p_s.reg_v0 = *addr;

    /*  
        Il valore
        ritornato deve essere il contenuto del registro di status
        del dispositivo. 
    */
}


static inline bool syscall_handler()
{
    const int id = (int)active_process->p_s.reg_a0;
    switch (id) {
        case CREATEPROCESS:
            pandos_kprintf("(::) syscall CREATEPROCESS\n");
            /* TODO */
            syscall_create_process();
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
            /* TODO */
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

    return false;
}

void exception_handler()
{
    state_t *p_s;
    bool return_control = false;

    p_s = (state_t *)BIOSDATAPAGE;
    memcpy(&active_process->p_s, p_s, sizeof(state_t));
    /* p_s.cause could have been used instead of getCAUSE() */
    switch (CAUSE_GET_EXCCODE(getCAUSE())) {
        case 0:
            return_control = interrupt_handler();
            break;
        case 1:
        case 2:
        case 3:
            return_control = tbl_handler();
            break;
        case 8:
            return_control = syscall_handler();
            /* ALWAYS increment the PC to prevent system call loops */
            active_process->p_s.pc_epc +=
                WORD_SIZE;
            active_process->p_s.reg_t9 +=
                WORD_SIZE;
            break;
        default: /* 4-7, 9-12 */
            return_control = trap_handler();
            break;
    }
    /* TODO: maybe rescheduling shouldn't be done all the time */
    /* TODO: increement active_pocess->p_time */
    if (return_control) {
        LDST(&active_process->p_s);
    }
    else {
        queue_process(active_process);
        schedule();
    }
}