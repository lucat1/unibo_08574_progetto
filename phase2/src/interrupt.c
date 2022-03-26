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
#include "syscall.h"
#include "os/asl.h"
#include "os/pcb.h"
#include "os/scheduler.h"
#include "os/util.h"
#include "semaphores.h"
#include "umps/cp0.h"
#include "umps/types.h"
#include <umps/arch.h>
#include <umps/libumps.h>



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
            return syscall_terminate_process();
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
            return syscall_get_cpu_time();
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
