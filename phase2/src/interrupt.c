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
#include "umps/cp0.h"
#include <umps/arch.h>
#include <umps/libumps.h>

/* TODO: fill me */
static inline bool interrupt_handler()
{
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
static inline bool trap_handler()
{
    pandos_kprintf("(::) trap\n");
    return false;
}

static inline bool syscall_handler()
{
    const int id = (int)active_process->p_s.reg_a0;
    switch (id) {
        case CREATEPROCESS:
            pandos_kprintf("(::) syscall CREATEPROCESS\n");
            /* TODO */
            break;
        case TERMPROCESS:
            pandos_kprintf("(::) syscall TERMPROCESS\n");
            /* TODO */
            break;
        case PASSEREN:
            pandos_kprintf("(::) syscall PASSEREN\n");
            /* TODO */
            break;
        case VERHOGEN:
            pandos_kprintf("(::) syscall VERHOGEN\n");
            /* TODO */
            break;
        case DOIO:
            pandos_kprintf("(::) syscall DOIO\n");
            /* TODO */
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
            active_process->p_s.pc_epc = active_process->p_s.reg_t9 +=
                WORD_SIZE;
            break;
        default: /* 4-7, 9-12 */
            return_control = trap_handler();
            break;
    }
    /* TODO: maybe rescheduling shouldn't be done all the time */
    /* TODO: increement active_pocess->p_time */
    if (return_control)
        LDST(&active_process->p_s);
    else
        schedule();
}
