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

static inline size_t syscall_writer(size_t il_n)
{
    support_t *const current_support =
        (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    const int id = current_support->sup_asid - 1;
    const char *s =
        (char *)current_support->sup_except_state[GENERALEXCEPT].reg_a1;
    const size_t len =
        (size_t)current_support->sup_except_state[GENERALEXCEPT].reg_a2;
    if (len < 0 || len > MAXLEN || (memaddr)s < KUSEG)
        SYSCALL(TERMINATE, 0, 0, 0);
    void *device = (void *)DEV_REG_ADDR(il_n, id);
    int *semaphores;
    bool terminal = false;
    switch (il_n) {
        case IL_PRINTER:
            semaphores = sys3_semaphores;
            break;
        case IL_TERMINAL:
            semaphores = sys4_semaphores;
            terminal = true;
            break;
        default:
            pandos_kfprintf(&kstderr, "Unimplemented write for class #%d\n",
                            il_n);
            PANIC();
    }
    int *const semaphore = &semaphores[id];

    SYSCALL(PASSEREN, (int)semaphore, 0, 0);
    for (size_t i = 0; i < len; ++i) {
        if (!terminal)
            ((dtpreg_t *)device)->data0 = s[i];
        const unsigned int
            value = PRINTCHR | (terminal ? ((unsigned int)s[i] << 8) : 0),
            status =
                SYSCALL(DOIO,
                        (int)(terminal ? &((termreg_t *)device)->transm_command
                                       : &((dtpreg_t *)device)->command),
                        (int)value, 0);
        if ((status & (terminal ? TERMSTATMASK : -1)) !=
            (terminal ? RECVD : DEV_STATUS_READY)) {
            SYSCALL(VERHOGEN, (int)semaphore, 0, 0);
            return -(status & (terminal ? TERMSTATMASK : -1));
        }
    }
    SYSCALL(VERHOGEN, (int)semaphore, 0, 0);
    return len;
}

static inline size_t sys_write_printer() { return syscall_writer(IL_PRINTER); }

static inline size_t sys_write_terminal()
{
    return syscall_writer(IL_TERMINAL);
}

static inline size_t sys_read_terminal()
{
    size_t read = 0;
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
    for (char r = EOS; r != '\n';) {
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
