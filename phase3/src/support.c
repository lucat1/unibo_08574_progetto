#include "support/support.h"
#include "arch/devices.h"
#include "arch/processor.h"
#include "os/const.h"
#include "os/ctypes.h"
#include "os/scheduler.h"
#include "os/syscall.h"
#include "os/util.h"
#include "support/pager.h"
#include "support/print.h"
#include "test/tconst.h"
#include "umps/arch.h"
#include "umps/const.h"
#include "umps/cp0.h"

inline void support_trap()
{
    pandos_kprintf("!!!!!support_trap\n");
    release_sem_swap_pool_table();
    SYSCALL(TERMPROCESS, 0, 0, 0);
}

static inline size_t sys_get_tod()
{
    cpu_t time;
    store_tod(&time);
    return time;
}

/* hardware constants */
#define PRINTCHR 2
#define RECVD 5

static inline size_t sys_write_printer()
{
    support_t *current_support = (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    size_t termid = current_support->sup_asid - 1;
    char *s = (char *)current_support->sup_except_state[GENERALEXCEPT].reg_a1;
    size_t len =
        (size_t)current_support->sup_except_state[GENERALEXCEPT].reg_a2;

    dtpreg_t *device = (dtpreg_t *)DEV_REG_ADDR(IL_PRINTER, (int)termid);
    unsigned int status;

    size_t i;
    for (i = 0; i < len; ++i) {
        device->data0 = s[i];
        status = SYSCALL(DOIO, (int)&device->command, (int)PRINTCHR, 0);
        if (device->status != DEV_STATUS_READY)
            return -(status & TERMSTATMASK);
    }
    return i;
}

#define RECEIVE_CHAR 2
#define RECEIVE_STATUS(v) ((v)&0xF)
#define RECEIVE_VALUE(v) (((v) >> 8) & 0xFF)
static inline size_t sys_read_terminal_v2()
{
    size_t read = 0;
    char r = EOS;
    support_t *current_support = (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    termreg_t *base = (termreg_t *)(DEV_REG_ADDR(
        IL_TERMINAL, (int)current_support->sup_asid - 1));
    char *buf = (char *)current_support->sup_except_state[GENERALEXCEPT].reg_a1;
    if ((memaddr)buf >= KUSEG + (MAXPAGES - 1) * PAGESIZE &&
        ((memaddr)buf >> VPNSHIFT) != (KUSEG + GETPAGENO) >> VPNSHIFT) {
        support_trap();
    }
    while (r != '\n') {
        size_t status =
            SYSCALL(DOIO, (int)&base->recv_command, (int)RECEIVE_CHAR, 0);
        if (RECEIVE_STATUS(status) != DEV_STATUS_TERMINAL_OK) {
            return -RECEIVE_STATUS(status);
        }
        r = RECEIVE_VALUE(status);
        // if (r != '\n')
        //     pandos_kfprintf(&kdebug, "reading %c to %p\n", r, (buf + read));
        *(buf + read) = r;
        ++read;
    }
    *(buf + read) = EOS;
    // pandos_kfprintf(&kdebug, "read: %d\n", read);
    return read;
}

static inline size_t syscall_writer(void *termid, char *msg, size_t len)
{
    termreg_t *device = (termreg_t *)DEV_REG_ADDR(IL_TERMINAL, (int)termid);
    // if ((size_t)termid == 0)
    //     pandos_kfprintf(&kdebug, "devregaddr: %p\n",
    //     &device->transm_command);
    char *s = msg;
    unsigned int status;

    // if ((size_t)termid == 0)
    //     pandos_kfprintf(&kdebug, "writing from %p\n", s);
    while (*s != EOS) {
        unsigned int value = PRINTCHR | (((unsigned int)*s) << 8);
        status = SYSCALL(DOIO, (int)&device->transm_command, (int)value, 0);
        if ((status & TERMSTATMASK) != RECVD) {
            return -(status & TERMSTATMASK);
        }
        // if ((size_t)termid == 0)
        //     pandos_kfprintf(&kdebug, "writing %c\n", *s);
        s++;
    }
    return len;
}

static inline size_t syscall_reader(void *termid, char *s)
{

    termreg_t *device = (termreg_t *)DEV_REG_ADDR(IL_TERMINAL, (int)termid);
    device->recv_command = RECEIVE_CHAR;
    unsigned int status;
    size_t len = 0;

    do {
        unsigned int value = PRINTCHR | (((unsigned int)*s) << 8);
        status = SYSCALL(DOIO, (int)&device->transm_command, (int)value, 0);
        ++len;
        if ((status & TERMSTATMASK) != RECVD) {
            return -(status & TERMSTATMASK);
        }
    } while (*s++ != EOS);
    return len;
}

static inline size_t sys_write_terminal()
{
    support_t *current_support = (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    size_t asid = current_support->sup_asid - 1;
    char *s = (char *)current_support->sup_except_state[GENERALEXCEPT].reg_a1;
    size_t len =
        (size_t)current_support->sup_except_state[GENERALEXCEPT].reg_a2;
    return syscall_writer((void *)(asid), s, len);
}

static inline void support_syscall(support_t *current_support)
{
    pandos_kprintf("!!!!!support_syscall\n");
    active_process->p_s.pc_epc = active_process->p_s.reg_t9 += WORD_SIZE;
    const int id = (int)current_support->sup_except_state[GENERALEXCEPT].reg_a0;
    pandos_kprintf("Code %d\n", id);
    size_t result;
    switch (id) {
        case GET_TOD:
            result = sys_get_tod();
            break;
        case WRITEPRINTER:
            result = sys_write_printer();
            break;
        case WRITETERMINAL:
            result = sys_write_terminal();
            break;
        case READTERMINAL:
            result = sys_read_terminal_v2();
            break;
        case TERMINATE:
        default:
            SYSCALL(TERMPROCESS, 0, 0, 0);
            return;
    }
    current_support->sup_except_state[GENERALEXCEPT].reg_v0 = result;
}

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
