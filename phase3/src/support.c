#include "support/support.h"
#include "os/scheduler.h"
#include "os/semaphores.h"
#include "os/syscall.h"
#include "os/util.h"
#include "support/pager.h"
#include "support/print.h"
#include <umps/arch.h>
#include <umps/cp0.h>

#define GETTOD 1
#define TERMINATE 2
#define WRITEPRINTER 3
#define WRITETERMINAL 4
#define READTERMINAL 5

// TODO : uTLB RefillHandler
void support_tlb()
{
    pandos_kprintf("!!!!!support_tlb\n");
    tlb_exceptionhandler();
}

// TODO
void support_trap() {}

void sys_get_tod()
{
    cpu_t time;
    STCK(time);
    /* Points to the current state POPS 8.5*/
    ((state_t *)BIOSDATAPAGE)->reg_v0 = time;
}

void sys_write_printer()
{
    /* TODO: Check for all the possible error causes*/

    state_t *current_state = ((state_t *)BIOSDATAPAGE);
    char *s = (char *)current_state->reg_a1;
    SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    int asid = ((support_t *)current_state->reg_v0)->sup_asid;
    dtpreg_t *base = (dtpreg_t *)DEV_REG_ADDR(IL_PRINTER, asid);
    int *sem_term_mut = get_semaphore(IL_PRINTER, asid, false);
    SYSCALL(PASSEREN, (int)&sem_term_mut, 0, 0);
    while (*s != EOS) {
        base->data0 = (unsigned int)s;
        base->command = TRANSMITCHAR;
        while (base->status == DEV_STATUS_BUSY)
            ;
        if (base->status != DEV_STATUS_READY) {
            PANIC();
        }
        base->command = DEV_C_ACK;
        s++;
    }
    SYSCALL(VERHOGEN, (int)&sem_term_mut, 0, 0);
    current_state->reg_v0 = current_state->reg_a2;
}

void sys_write_terminal()
{
    /* TODO: Check for all the possible error causes*/
    state_t *current_state = ((state_t *)BIOSDATAPAGE);
    SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    current_state->reg_v0 =
        fsysprintf(((support_t *)current_state->reg_v0)->sup_asid,
                   (char *)current_state->reg_a1);
}

#define RECEIVE_CHAR 2
void sys_read_terminal()
{
    // typedef unsigned int devregtr;
    state_t *current_state = ((state_t *)BIOSDATAPAGE);
    SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    int asid = ((support_t *)current_state->reg_v0)->sup_asid;
    termreg_t *base = (termreg_t *)(DEV_REG_ADDR(IL_TERMINAL, asid));
    int *sem_term_mut = get_semaphore(IL_TERMINAL, asid, false);
    SYSCALL(PASSEREN, (int)&sem_term_mut, 0, 0); /* P(sem_term_mut) */
    base->recv_command = RECEIVE_CHAR;
    // status = SYSCALL(DOIO, (int)command, (int)value, 0);
    if (base->recv_status != DEV_STATUS_TERMINAL_OK) {
        PANIC();
    }
    char *msg = (char *)base->recv_command;
    base->recv_command = DEV_C_ACK;
    SYSCALL(VERHOGEN, (int)&sem_term_mut, 0, 0); /* V(sem_term_mut) */
    current_state->reg_a1 = (unsigned int)msg;
}

void support_syscall()
{
    switch (((state_t *)BIOSDATAPAGE)->reg_a0) {
        case GETTOD:
            sys_get_tod();
            break;
        case TERMINATE:
            SYSCALL(TERMPROCESS, 0, 0, 0);
            break;
        case WRITEPRINTER:
            sys_write_printer();
            break;
        case WRITETERMINAL:
            sys_write_terminal();
            break;
        case READTERMINAL:
            break;
        default:
            /*idk*/
            break;
    }
    load_state(((state_t *)BIOSDATAPAGE));
    /*
        TODO:   the Support Level’s SYSCALL exception handler must also incre-
                ment the PC by 4 in order to return control to the instruction
       after the SYSCALL instruction.
    */
}

inline void support_generic()
{
    SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    support_t *current_support =
        (support_t *)(((state_t *)BIOSDATAPAGE)->reg_v0);
    switch (CAUSE_GET_EXCCODE(current_support->sup_except_state->cause)) {
        case 8: /*Syscall*/
            support_syscall();
            break;
        default:
            support_trap();
    }
}
