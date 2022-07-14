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

inline void support_generic()
{
    // pandos_kprintf("Start of support generic\n");
    support_t *current_support = (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    state_t *saved_state = &current_support->sup_except_state[GENERALEXCEPT];
    int id = CAUSE_GET_EXCCODE(
        current_support->sup_except_state[GENERALEXCEPT].cause);
    switch (id) {
        case 8: /*Syscall*/
            support_syscall(current_support);
            break;
        default:
            support_trap();
    }

    // pandos_kprintf("End of support generic\n");
    saved_state->pc_epc += WORD_SIZE;
    saved_state->reg_t9 += WORD_SIZE;
    load_state(saved_state);
}

/* TODO: */
void support_trap() { pandos_kprintf("!!!!!support_trap\n"); }

void sys_get_tod()
{
    cpu_t time;
    STCK(time);
    /* Points to the current state POPS 8.5*/
    ((state_t *)BIOSDATAPAGE)->reg_v0 = time;
}

/* hardware constants */
#define PRINTCHR 2
#define RECVD 5

void sys_write_printer()
{
    /* TODO: Check for all the possible error causes*/
    /*support_t *current_support = (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    char *s = (char *)current_support->sup_except_state[GENERALEXCEPT].reg_a1;
    int asid = current_support->sup_asid;
    dtpreg_t *base = (dtpreg_t *)DEV_REG_ADDR(IL_PRINTER, asid);
    int *sem_term_mut = get_semaphore(IL_PRINTER, asid, false);

    SYSCALL(PASSEREN, (int)&sem_term_mut, 0, 0);
    while (*s != EOS) {
        base->data0 = (char)s;
        pandos_kfprintf(&kdebug, "About to print %c\n", base->data0);
        base->command = TRANSMITCHAR;
        while (base->status == DEV_STATUS_BUSY);
        if (base->status != DEV_STATUS_READY) {
            PANIC();
        }
        pandos_kfprintf(&kdebug, "printed a thing\n");
        base->command = DEV_C_ACK;
        s++;
    }
    SYSCALL(VERHOGEN, (int)&sem_term_mut, 0, 0);
    current_support->sup_except_state[GENERALEXCEPT].reg_v0 =
    current_support->sup_except_state[GENERALEXCEPT].reg_a2;*/
    support_t *current_support = (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    size_t termid = current_support->sup_asid;
    char *s = (char *)current_support->sup_except_state[GENERALEXCEPT].reg_a1;
    size_t len =
        (size_t)current_support->sup_except_state[GENERALEXCEPT].reg_a2;

    int *sem_term_mut = get_semaphore(IL_PRINTER, (int)termid, true);

    dtpreg_t *device = (dtpreg_t *)DEV_REG_ADDR(IL_PRINTER, (int)termid);
    unsigned int status;

    SYSCALL(PASSEREN, (int)&sem_term_mut, 0, 0);
    while (*s != EOS) {
        device->data0 = *s;
        pandos_kfprintf(&kdebug, "About to print %c\n", *s);
        status = SYSCALL(DOIO, (int)&device->command, (int)PRINTCHR, 0);
        pandos_kfprintf(&kdebug, "Printed %c\n", *s);
        if ((status & TERMSTATMASK) != RECVD) {
            current_support->sup_except_state[GENERALEXCEPT].reg_v0 =
                -(status & TERMSTATMASK);
            return;
        }
        s++;
    }
    SYSCALL(VERHOGEN, (int)&sem_term_mut, 0, 0);
    current_support->sup_except_state[GENERALEXCEPT].reg_v0 = len;
}

size_t sys_write_terminal()
{
    support_t *current_support = (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    size_t asid = current_support->sup_asid;
    char *s = (char *)current_support->sup_except_state[GENERALEXCEPT].reg_a1;
    size_t len =
        (size_t)current_support->sup_except_state[GENERALEXCEPT].reg_a2;
    return syscall_writer((void *)(asid), s, len);
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

void support_syscall(support_t *current_support)
{
    pandos_kprintf("!!!!!support_syscall\n");
    // state_t *saved_state = (state_t *)BIOSDATAPAGE;
    const int id = (int)current_support->sup_except_state[GENERALEXCEPT].reg_a0;
    pandos_kprintf("Code %d\n", id);
    size_t res = -1;
    switch (id) {
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
            res = sys_write_terminal();
            break;
        case READTERMINAL:
            break;
        default:
            /*idk*/
            break;
    }
    current_support->sup_except_state[GENERALEXCEPT].reg_v0 = res;
    /*
        TODO:   the Support Level’s SYSCALL exception handler must also incre-
                ment the PC by 4 in order to return control to the instruction
       after the SYSCALL instruction.
    */
}

size_t syscall_writer(void *termid, char *msg, size_t len)
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
            return -(status & TERMSTATMASK);
        }
        s++;
    }
    SYSCALL(VERHOGEN, (int)&sem_term_mut, 0, 0); /* V(sem_term_mut) */
    return len;
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
