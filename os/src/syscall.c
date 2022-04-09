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
#include "os/exception_impl.h"
#include "os/interrupt.h"
#include "os/scheduler.h"
#include "os/semaphores.h"
#include "os/util.h"

#define pandos_syscall(n) pandos_kprintf("<< SYSCALL(" n ")\n")

/* NSYS1 */
static inline scheduler_control_t syscall_create_process()
{
    /* parameters of syscall */
    state_t *p_s = (state_t *)active_process->p_s.reg_a1;
    bool p_prio = (bool)active_process->p_s.reg_a2;
    support_t *p_support_struct = (support_t *)active_process->p_s.reg_a3;
    if (p_s == NULL ||
        (p_prio != PROCESS_PRIO_LOW && p_prio != PROCESS_PRIO_HIGH))
        return pass_up_or_die((memaddr)GENERALEXCEPT);

    /* spawn new process */
    pcb_t *c = spawn_process(p_prio);

    /* checks if there are enough resources */
    if (c == NULL) {
        pandos_kfprintf(&kstderr, "!! ERROR: Cannot create new process\n");
        /* set caller's v0 to -1 */
        active_process->p_s.reg_v0 = -1;
    } else {
        c->p_support = p_support_struct;
        pandos_memcpy(&c->p_s, p_s, sizeof(state_t));
        /* p_time is already set to 0 from the alloc_pcb call inside
         * spawn_process */
        /* p_sem_add is already set to NULL from the alloc_pcb call inside
         * spawn_process */

        /* adds new process as child of caller process */
        insert_child(active_process, c);
        /* sets caller's v0 to new process pid */
        active_process->p_s.reg_v0 = c->p_pid;
    }
    return CONTROL_PRESERVE(active_process);
}

/* NSYS2 */
static inline scheduler_control_t syscall_terminate_process()
{
    /* Generate an interrupt to signal the end of Current Process’s time q
       antum/slice. The PLT is reserved for this purpose. */

    pcb_t *p;
    const pandos_pid_t pid = active_process->p_s.reg_a1;
    if (pid == (pandos_pid_t)NULL) {
        return pass_up_or_die((memaddr)GENERALEXCEPT);
    }

    /* If pid is not 0 then the target must be searched */
    if (pid != 0 && (p = (pcb_t *)find_process(pid)) == NULL)
        scheduler_panic("Could not find process by pid: %p\n", pid);
    else
        p = active_process;

    /* If the process was blocked on a semaphore, decrease the blocked count */
    if (p->p_sem_add != NULL) {
        blocked_count--;
    }

    /* TODO: handle kill_progeny return value */
    kill_progeny(p);

    /* is this is the docs? */
    // if (pid != 0)
    //     p->p_s.reg_v0 = pid;
    return pid == 0 ? CONTROL_BLOCK : CONTROL_RESCHEDULE;
}

/* NSYS3 */
static inline scheduler_control_t syscall_passeren()
{
    if (active_process->p_s.reg_a1 == (memaddr)NULL) {
        return pass_up_or_die((memaddr)GENERALEXCEPT);
    }
    return P((int *)active_process->p_s.reg_a1, active_process);
}

/* NSYS4 */
static inline scheduler_control_t syscall_verhogen()
{
    if (active_process->p_s.reg_a1 == (memaddr)NULL) {
        return pass_up_or_die((memaddr)GENERALEXCEPT);
    }
    V((int *)active_process->p_s.reg_a1);
    // return V((int *)active_process->p_s.reg_a1) != NULL
    //            ? CONTROL_PRESERVE(active_process)
    //            : CONTROL_RESCHEDULE;
    return CONTROL_RESCHEDULE;
}

/* NSYS5 */
static inline scheduler_control_t syscall_do_io()
{
    size_t *cmd_addr = (size_t *)active_process->p_s.reg_a1;
    size_t cmd_value = (size_t)active_process->p_s.reg_a2;
    if (cmd_addr == (size_t *)NULL || cmd_value == (size_t)NULL)
        return pass_up_or_die((memaddr)GENERALEXCEPT);

    iodev_t dev = get_iodev(cmd_addr);
    if (dev.semaphore == NULL || *dev.semaphore > 0)
        /* scheduler_panic("Invalid interrupt line\n"); */
        return pass_up_or_die((memaddr)GENERALEXCEPT);

    scheduler_control_t ctrl = P(dev.semaphore, active_process);
    active_process->p_s.status |= interrupt_mask(dev.interrupt_line);

    /* Finally write the data */
    *cmd_addr = cmd_value;

    pandos_kprintf("do_io result\n");
    return ctrl;
}

/* NSYS6 */
static inline scheduler_control_t syscall_get_cpu_time()
{
    int now;
    store_tod(&now);
    int diff = now - last_plt;
    active_process->p_s.reg_v0 = active_process->p_time + diff;
    return CONTROL_RESCHEDULE;
}

/* NSYS7 */
static inline scheduler_control_t syscall_wait_for_clock()
{
    return P(&timer_semaphore, active_process);
}

/* NSYS8 */
static inline scheduler_control_t syscall_get_support_data()
{
    active_process->p_s.reg_v0 = (memaddr)active_process->p_support;
    return CONTROL_RESCHEDULE;
}

/* NSYS9 */
static inline scheduler_control_t syscall_get_process_id()
{
    bool parent = (bool)active_process->p_s.reg_a1;
    if (parent == (bool)NULL) {
        return pass_up_or_die((memaddr)GENERALEXCEPT);
    }
    /* if parent then return parent pid, else return active process pid */
    if (!parent)
        active_process->p_s.reg_v0 = active_process->p_pid;
    else if (active_process->p_parent != NULL)
        active_process->p_s.reg_v0 = active_process->p_parent->p_pid;
    else
        active_process->p_s.reg_v0 = 0;

    return CONTROL_PRESERVE(active_process);
}

/* NSYS10 */
static inline scheduler_control_t syscall_yeld() { return CONTROL_RESCHEDULE; }

inline scheduler_control_t syscall_handler()
{
    if (active_process == NULL)
        scheduler_panic("Syscall recieved while active_process was NULL");
    const int id = (int)active_process->p_s.reg_a0;
    if (id <= 0 && is_user_mode()) {
        pandos_kfprintf(&kstderr,
                        "Negative syscalls cannot be called in user mode!\n");
        return pass_up_or_die((memaddr)GENERALEXCEPT);
    }
    switch (id) {
        case CREATEPROCESS:
            pandos_syscall("CREATEPROCESS");
            return syscall_create_process();
            break;
        case TERMPROCESS:
            pandos_syscall("TERMPROCESS");
            return syscall_terminate_process();
            break;
        case PASSEREN:
            pandos_syscall("PASSEREN");
            return syscall_passeren();
            break;
        case VERHOGEN:
            pandos_syscall("VERHOGEN");
            return syscall_verhogen();
            break;
        case DOIO:
            pandos_syscall("DOIO");
            return syscall_do_io();
            break;
        case GETTIME:
            pandos_syscall("GETTIME");
            return syscall_get_cpu_time();
            break;
        case CLOCKWAIT:
            pandos_syscall("CLOCKWAIT");
            return syscall_wait_for_clock();
            break;
        case GETSUPPORTPTR:
            pandos_syscall("GETSUPPORTPTR");
            return syscall_get_support_data();
            break;
        case GETPROCESSID:
            pandos_syscall("GETPROCESSID");
            return syscall_get_process_id();
            break;
        case YIELD:
            pandos_syscall("YIELD");
            return syscall_yeld();
            break;
        default:
            return pass_up_or_die((memaddr)GENERALEXCEPT);
            break;
    }

    return CONTROL_RESCHEDULE;
}
