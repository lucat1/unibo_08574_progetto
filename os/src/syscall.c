/**
 * \file syscall.c
 * \brief Implementation of all negative syscalls.
 *
 * \author Alessandro Frau
 * \author Gianmaria Rovelli
 * \author Luca Tagliavini
 * \date 26-03-2022
 */

#include "os/syscall.h"
#include "arch/devices.h"
#include "arch/processor.h"
#include "os/asl.h"
#include "os/puod.h"
#include "os/scheduler.h"
#include "os/semaphores.h"
#include "os/util.h"
#include "umps/arch.h"
#include "umps/cp0.h"

/**
 * \brief CREATEPROCESS syscall implementation (SYSCALL -1)
 * \return Control preserved on current process
 */
static inline scheduler_control_t syscall_create_process()
{
    pcb_t *c;
    state_t *p_s = (state_t *)active_process->p_s.reg_a1;
    bool p_prio = (bool)active_process->p_s.reg_a2;
    support_t *p_support_struct = (support_t *)active_process->p_s.reg_a3;

    if ((active_process->p_s.reg_a2 != PROCESS_PRIO_LOW &&
         active_process->p_s.reg_a2 != PROCESS_PRIO_HIGH) ||
        p_s == NULL) {
        return pass_up_or_die((memaddr)GENERALEXCEPT);
    }

    /* Checks if there are enough resources */
    if ((c = spawn_process(p_prio)) == NULL)
        active_process->p_s.reg_v0 = NULL_PID;
    else {
        c->p_support = p_support_struct;
        pandos_memcpy(&c->p_s, p_s, sizeof(state_t));
        insert_child(active_process, c);
        active_process->p_s.reg_v0 = c->p_pid;
    }
    return CONTROL_PRESERVE(active_process);
}

/**
 * \brief TERMPROCESS syscall implementation (SYSCALL -2)
 * \return Control blocked if caller's `pid` is 0 or if current process has been
 * killed, control reschedule othewise
 */
static inline scheduler_control_t syscall_terminate_process()
{
    pcb_t *p;
    const pandos_pid_t pid = active_process->p_s.reg_a1;

    if (pid == (pandos_pid_t)NULL)
        return pass_up_or_die((memaddr)GENERALEXCEPT);

    /* Search for the target when pid != 0 */
    if (pid == 0)
        p = active_process;
    else if (pid != 0 && (p = (pcb_t *)find_process(pid)) == NULL)
        return pass_up_or_die((memaddr)GENERALEXCEPT);

    if (kill_progeny(p))
        scheduler_panic("Kill progeny failed!");

    return (pid == 0 || active_process->p_pid == NULL_PID) ? CONTROL_BLOCK
                                                           : CONTROL_RESCHEDULE;
}

/**
 * \brief PASSEREN syscall implementation (SYSCALL -3)
 * \return Control depends on semaphore to be P'ed
 */
static inline scheduler_control_t syscall_passeren()
{
    if (active_process->p_s.reg_a1 == (memaddr)NULL) {
        return pass_up_or_die((memaddr)GENERALEXCEPT);
    }
    return P((int *)active_process->p_s.reg_a1, active_process);
}

/**
 * \brief VERHOGEN syscall implementation (SYSCALL -4)
 * \return Control depends on semaphore to be V'ed
 */
static inline scheduler_control_t syscall_verhogen()
{
    if (active_process->p_s.reg_a1 == (memaddr)NULL) {
        return pass_up_or_die((memaddr)GENERALEXCEPT);
    }
    return V((int *)active_process->p_s.reg_a1) == NULL ? CONTROL_BLOCK
                                                        : CONTROL_RESCHEDULE;
}

/**
 * \brief DOIO syscall implementation (SYSCALL -5), writes command to targeted
 * address memory, both passed as parameters \return Control depends on device's
 * semaphore passed as parameter
 */
static inline scheduler_control_t syscall_do_io()
{
    iodev_t dev;
    size_t *cmd_addr = (size_t *)active_process->p_s.reg_a1;
    size_t cmd_value = (size_t)active_process->p_s.reg_a2;


    if (cmd_addr == (size_t *)NULL ||
        (dev = get_iodev(cmd_addr)).semaphore == NULL ||
        head_blocked(dev.semaphore) != NULL) {
        return pass_up_or_die((memaddr)GENERALEXCEPT);
    }

    if (*dev.semaphore > 0)
        scheduler_panic("A device syncronization semaphore has a value > 0");

    scheduler_control_t ctrl = P(dev.semaphore, active_process);
    if (active_process->p_prio)
        status_il_on(&active_process->p_s.status, dev.interrupt_line);
    else
        status_il_on_all(&active_process->p_s.status);

    /* Finally write the data */
    *cmd_addr = cmd_value;

    return ctrl;
}

/**
 * \brief GETTIME syscall implementation (SYSCALL -6)
 * \return Control reschedule
 */
static inline scheduler_control_t syscall_get_cpu_time()
{
    active_process->p_s.reg_v0 = active_process->p_time;
    return CONTROL_RESCHEDULE;
}

/**
 * \brief CLOCKWAIT syscall implementation (SYSCALL -7), blocks current process
 *        in clock's semaphore
 * \return Control depends on clock's semaphore status
 */
static inline scheduler_control_t syscall_wait_for_clock()
{
    return P(get_timer_semaphore(), active_process);
}

/**
 * \brief GETSUPPORTPTR syscall implementation (SYSCALL -8) , blocks current
 * process in clock semaphore \return Control reschedule
 */
static inline scheduler_control_t syscall_get_support_data()
{
    active_process->p_s.reg_v0 = (memaddr)active_process->p_support;
    return CONTROL_RESCHEDULE;
}

/**
 * \brief GETPROCESSID syscall implementation (SYSCALL -9)
 * \return Control reschedule
 */
static inline scheduler_control_t syscall_get_process_id()
{
    bool parent = (bool)active_process->p_s.reg_a1;
    if ((parent != (bool)0 && parent != (bool)1))
        return pass_up_or_die((memaddr)GENERALEXCEPT);

    /* Return its own pid, the parent's or 0 otherwhise */
    if (!parent)
        active_process->p_s.reg_v0 = active_process->p_pid;
    else if (active_process->p_parent != NULL)
        active_process->p_s.reg_v0 = active_process->p_parent->p_pid;
    else
        active_process->p_s.reg_v0 = 0;

    return CONTROL_RESCHEDULE;
}

/**
 * \brief YIELD syscall implementation (SYSCALL -10), current process
 *        release processor's control
 * \return Control blocked
 */
static inline scheduler_control_t syscall_yield()
{
    yield_process = active_process;
    return CONTROL_BLOCK;
}

/**
 * \brief Syscall handler
 * \return Control depends on syscall executed , pass up or die
 *         is called in case of errors
 */
inline scheduler_control_t syscall_handler()
{
    if (active_process == NULL)
        scheduler_panic("Syscall recieved while active_process was NULL");

    const int id = (int)active_process->p_s.reg_a0;
    if (id <= 0 && is_user_mode()) {
        cause_clean(&active_process->p_s.cause);
        cause_reserved_instruction(&active_process->p_s.cause);
        return pass_up_or_die((memaddr)GENERALEXCEPT);
    }

    pandos_kprintf("syscall %d %s\n", id, is_user_mode() ? "user" : "kern");
    switch (id) {
        case CREATEPROCESS:
            return syscall_create_process();
        case TERMPROCESS:
            return syscall_terminate_process();
        case PASSEREN:
            return syscall_passeren();
        case VERHOGEN:
            return syscall_verhogen();
        case DOIO:
            return syscall_do_io();
        case GETTIME:
            return syscall_get_cpu_time();
        case CLOCKWAIT:
            return syscall_wait_for_clock();
        case GETSUPPORTPTR:
            return syscall_get_support_data();
        case GETPROCESSID:
            return syscall_get_process_id();
        case YIELD:
            return syscall_yield();
        default:
            pandos_kprintf("syscall not handled\n");
            return pass_up_or_die((memaddr)GENERALEXCEPT);
    }
}
