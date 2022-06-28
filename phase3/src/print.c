#include "support/print.h"
#include "os/util.h"
#include "os/util_impl.h"
#include "os/semaphores.h"
#include <umps/arch.h>

/* hardware constants */
#define PRINTCHR 2
#define RECVD 5
#define TERM0ADDR 0x10000254

int *sem_term_mut; /* for mutual exclusion on terminal */

size_t syscall_writer(void *termid, const char *msg, size_t len)
{
    typedef unsigned int devregtr;
    const char *s = msg;
    devregtr *base = (devregtr *)(DEV_REG_ADDR(IL_TERMINAL, termid));
    devregtr *command = base + 3;
    devregtr status;

    SYSCALL(PASSEREN, (int)&sem_term_mut, 0, 0); /* P(sem_term_mut) */
    while (*s != EOS) {
        devregtr value = PRINTCHR | (((devregtr)*s) << 8);
        status = SYSCALL(DOIO, (int)command, (int)value, 0);
        if ((status & TERMSTATMASK) != RECVD) {
            PANIC();
        }
        s++;
    }
    SYSCALL(VERHOGEN, (int)&sem_term_mut, 0, 0); /* V(sem_term_mut) */
    return s - msg;
}

size_t fsysprintf(int termid, const char *fmt, ...)
{
    termreg_t *term = (termreg_t *)DEV_REG_ADDR(IL_TERMINAL, termid);
    sem_term_mut = get_semaphore(IL_TERMINAL, termid, false);
    va_list varg;
    va_start(varg, fmt);
    size_t res = __pandos_printf(term, syscall_writer, fmt, varg);
    va_end(varg);
    return res;
}
