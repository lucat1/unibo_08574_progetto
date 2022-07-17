#include "support/support.h"
#include "os/const.h"
#include "os/ctypes.h"
#include "os/scheduler.h"
#include "os/semaphores.h"
#include "os/syscall.h"
#include "os/util.h"
#include "support/pager.h"
#include "support/test.h"
#include "test/tconst.h"
#include "umps/arch.h"
#include "umps/const.h"
#include "umps/cp0.h"

/* hardware constants */
#define PRINTCHR 2
#define RECVD 5

// string length constraints
#define MINLEN 0
#define MAXLEN 128

#define RECEIVE_CHAR 2
#define RECEIVE_STATUS(v) ((v)&0xF)
#define RECEIVE_VALUE(v) (((v) >> 8) & 0xFF)

static int sys3_semaphores[UPROCMAX], sys4_semaphores[UPROCMAX],
    sys5_semaphores[UPROCMAX], master_semaphore = 0;

inline void init_sys_semaphores()
{
    for (int i = 0; i < UPROCMAX; ++i)
        sys3_semaphores[i] = sys4_semaphores[i] = sys5_semaphores[i] = 1;
}

static inline void master_semaphore_v()
{
    SYSCALL(VERHOGEN, (int)&master_semaphore, 0, 0);
}

inline void master_semaphore_p()
{
    SYSCALL(PASSEREN, (int)&master_semaphore, 0, 0);
}

static inline size_t sys_get_tod()
{
    cpu_t time;
    store_tod(&time);
    return time;
}

static inline void sys_terminate()
{
    support_t *const s = ((support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0));
    clean_after_uproc(s->sup_asid);
    deallocate_support(s);
    master_semaphore_v();
    SYSCALL(TERMPROCESS, 0, 0, 0);
}

static inline size_t syscall_writer(void *termid, const char *msg, size_t len)
{
    termreg_t *const device =
        (termreg_t *)DEV_REG_ADDR(IL_TERMINAL, (int)termid);
    const char *s = msg;
    int *const semaphore = &sys4_semaphores[(int)termid];

    SYSCALL(PASSEREN, (int)semaphore, 0, 0);
    for (size_t i = 0; i < len; ++i) {
        const unsigned int value = PRINTCHR | (((unsigned int)*s) << 8);
        const unsigned int status =
            SYSCALL(DOIO, (int)&device->transm_command, (int)value, 0);
        if ((status & TERMSTATMASK) != RECVD) {
            SYSCALL(VERHOGEN, (int)semaphore, 0, 0);
            return -(status & TERMSTATMASK);
        }
        ++s;
    }
    SYSCALL(VERHOGEN, (int)semaphore, 0, 0);
    return len;
}

static inline size_t sys_write_printer()
{
    support_t *const current_support =
        (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    const size_t printerid = current_support->sup_asid - 1;
    const char *s =
        (const char *)current_support->sup_except_state[GENERALEXCEPT].reg_a1;
    const size_t len =
        (size_t)current_support->sup_except_state[GENERALEXCEPT].reg_a2;
    dtpreg_t *const device =
        (dtpreg_t *)DEV_REG_ADDR(IL_PRINTER, (int)printerid);
    int *const semaphore = &sys3_semaphores[printerid];

    if (len < 0 || len > MAXLEN || (memaddr)s < KUSEG)
        SYSCALL(TERMINATE, 0, 0, 0);
    SYSCALL(PASSEREN, (int)semaphore, 0, 0);
    for (size_t i = 0; i < len; ++i) {
        device->data0 = s[i];
        const unsigned int status =
            SYSCALL(DOIO, (int)&device->command, (int)PRINTCHR, 0);
        if (device->status != DEV_STATUS_READY) {
            SYSCALL(VERHOGEN, (int)semaphore, 0, 0);
            return -(status & TERMSTATMASK);
        }
    }
    SYSCALL(VERHOGEN, (int)semaphore, 0, 0);
    return len;
}

static inline size_t sys_write_terminal()
{
    support_t *current_support = (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    size_t asid = current_support->sup_asid - 1;
    char *s = (char *)current_support->sup_except_state[GENERALEXCEPT].reg_a1;
    size_t len =
        (size_t)current_support->sup_except_state[GENERALEXCEPT].reg_a2;
    if (len < 0 || len > MAXLEN || (memaddr)s < KUSEG)
        SYSCALL(TERMINATE, 0, 0, 0);
    return syscall_writer((void *)(asid), s, len);
}

static inline size_t sys_read_terminal()
{
    size_t read = 0;
    char r = EOS;
    support_t *const current_support =
        (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    const int termid = (int)current_support->sup_asid - 1;
    termreg_t *const base = (termreg_t *)(DEV_REG_ADDR(IL_TERMINAL, termid));
    char *const buf =
        (char *)current_support->sup_except_state[GENERALEXCEPT].reg_a1;
    int *const semaphore = &sys5_semaphores[termid];

    if ((memaddr)buf < KUSEG)
        SYSCALL(TERMINATE, 0, 0, 0);
    SYSCALL(PASSEREN, (int)semaphore, 0, 0);
    while (r != '\n') {
        const size_t status =
            SYSCALL(DOIO, (int)&base->recv_command, (int)RECEIVE_CHAR, 0);
        if (RECEIVE_STATUS(status) != DEV_STATUS_TERMINAL_OK) {
            SYSCALL(VERHOGEN, (int)semaphore, 0, 0);
            return -RECEIVE_STATUS(status);
        }
        r = RECEIVE_VALUE(status);
        *(buf + read++) = r;
    }
    SYSCALL(VERHOGEN, (int)semaphore, 0, 0);
    *(buf + read) = EOS;
    return read;
}

static inline void support_syscall(support_t *current_support)
{
    active_process->p_s.pc_epc = active_process->p_s.reg_t9 += WORD_SIZE;
    const int id = (int)current_support->sup_except_state[GENERALEXCEPT].reg_a0;
    pandos_kfprintf(&kdebug, "Syscall #%d\n", id);
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
            result = sys_read_terminal();
            break;
        case TERMINATE:
        default:
            sys_terminate();
            return;
    }
    current_support->sup_except_state[GENERALEXCEPT].reg_v0 = result;
}

inline void support_generic()
{
    support_t *const current_support =
        (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    state_t *const saved_state =
        &current_support->sup_except_state[GENERALEXCEPT];
    const int id = CAUSE_GET_EXCCODE(
        current_support->sup_except_state[GENERALEXCEPT].cause);
    switch (id) {
        case 8: /*Syscall*/
            support_syscall(current_support);
            break;
        default:
            support_trap();
    }
    saved_state->pc_epc += WORD_SIZE;
    saved_state->reg_t9 += WORD_SIZE;
    load_state(saved_state);
}

inline void support_trap() { SYSCALL(TERMINATE, 0, 0, 0); }
