#include "support/support.h"
#include "arch/processor.h"
#include "os/const.h"
#include "os/ctypes.h"
#include "os/scheduler.h"
#include "os/semaphores.h"
#include "os/syscall.h"
#include "os/util.h"
#include "support/pager.h"
#include "support/print.h"
#include "umps/arch.h"
#include "umps/const.h"
#include "umps/cp0.h"

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
inline void support_generic()
{
    pandos_kprintf("!!!!!support_generic\n");
    support_t *current_support = (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    state_t *saved_state = &current_support->sup_except_state[GENERALEXCEPT];
    int id = CAUSE_GET_EXCCODE(
        current_support->sup_except_state[GENERALEXCEPT].cause);
    switch (id) {
        case 0:
        case 1:
        case 2:
        case 3:
            pandos_kfprintf(&kstderr, "Invalid cause %d for Support Level's "
                                      "general exception handler\n");
            PANIC();
        case 8: /*Syscall*/
            support_syscall(current_support);
            break;
        default:
            support_trap();
    }

    pandos_kprintf("!!!!!support_generic fine\n");
    saved_state->pc_epc += WORD_SIZE;
    saved_state->reg_t9 += WORD_SIZE;
    load_state(saved_state);
}

void support_trap()
{
    pandos_kprintf("!!!!!support_trap\n");
    release_sem_swap_pool_table();
    SYSCALL(TERMPROCESS, 0, 0, 0);
}

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
    support_t *current_support = (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    int asid = current_support->sup_asid;
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
    support_t *current_support = (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    size_t asid = current_support->sup_asid;
    char *s = (char *)current_support->sup_except_state[GENERALEXCEPT].reg_a1;
    size_t len =
        (size_t)current_support->sup_except_state[GENERALEXCEPT].reg_a2;

    syscall_writer((void *)(asid), s, len);
}

#define RECEIVE_CHAR 2
void sys_read_terminal_v2()
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

/* hardware constants */
#define PRINTCHR 2
#define RECVD 5

void syscall_writer(void *termid, char *msg, size_t len)
{

    int *sem_term_mut = get_semaphore(IL_TERMINAL, (int)termid, true);

    termreg_t *device = (termreg_t *)DEV_REG_ADDR(IL_TERMINAL, (int)termid);
    char *s = msg;
    unsigned int status;

    SYSCALL(PASSEREN, (int)&sem_term_mut, 0, 0); /* P(sem_term_mut) */
    while (*s != EOS) {
        unsigned int value = PRINTCHR | (((unsigned int)*s) << 8);
        status = SYSCALL(DOIO, (int)&device->transm_command, (int)value, 0);
        if ((status & TERMSTATMASK) != RECVD) {
            ((state_t *)BIOSDATAPAGE)->reg_a1 = -(status & TERMSTATMASK);
        }
        s++;
    }
    SYSCALL(VERHOGEN, (int)&sem_term_mut, 0, 0); /* V(sem_term_mut) */
    ((state_t *)BIOSDATAPAGE)->reg_v0 = len;
}

size_t syscall_reader(void *termid, char *s)
{

    int *sem_term_mut = get_semaphore(IL_TERMINAL, (int)termid, false);

    termreg_t *device = (termreg_t *)DEV_REG_ADDR(IL_TERMINAL, (int)termid);
    device->recv_command = RECEIVE_CHAR;
    unsigned int status;
    size_t len = 0;

    SYSCALL(PASSEREN, (int)&sem_term_mut, 0, 0); /* P(sem_term_mut) */
    do {
        unsigned int value = PRINTCHR | (((unsigned int)*s) << 8);
        status = SYSCALL(DOIO, (int)&device->transm_command, (int)value, 0);
        ++len;
        if ((status & TERMSTATMASK) != RECVD) {
            return -(status & TERMSTATMASK);
        }
    } while (*s++ != EOS);
    SYSCALL(VERHOGEN, (int)&sem_term_mut, 0, 0); /* V(sem_term_mut) */
    return len;
}

void support_syscall(support_t *current_support)
{
    pandos_kprintf("!!!!!support_syscall\n");
    const int id = (int)current_support->sup_except_state[GENERALEXCEPT].reg_a0;
    pandos_kprintf("Code %d\n", id);
    switch (id) {
        case GETTOD:
            sys_get_tod();
            break;
        case WRITEPRINTER:
            sys_write_printer();
            break;
        case WRITETERMINAL:
            sys_write_terminal();
            break;
        case READTERMINAL:
            sys_read_terminal_v2();
            break;
        case TERMINATE:
        default:
            SYSCALL(TERMPROCESS, 0, 0, 0);
            return;
    }
    active_process->p_s.pc_epc = active_process->p_s.reg_t9 += WORD_SIZE;
}
