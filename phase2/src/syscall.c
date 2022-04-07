/**
 * \file syscall.c
 * \brief Syscalls.
 *
 * \author Alessandro Frau
 * \author Gianmaria Rovelli
 * \author Luca Tagliavini
 * \date 26-03-2022
 */

#include "syscall.h"
#include "exception_impl.h"
#include "interrupt.h"
#include "os/asl.h"
#include "os/scheduler.h"
#include "os/semaphores.h"
#include "os/util.h"
#include "umps/cp0.h"
#include <umps/arch.h>
#include <umps/libumps.h>

#define pandos_syscall(n, pid) pandos_kprintf("<< SYSCALL(%d, " n ")\n", pid)

int checkUserMode() { return ((active_process->p_s.status << 28) >> 31); }

/* NSYS1 */
static inline scheduler_control_t syscall_create_process()
{
    /* parameters of syscall */
    if (active_process->p_s.reg_a1 == (int)NULL ||
        active_process->p_s.reg_a2 == (int)NULL) {
        return pass_up_or_die((memaddr)GENERALEXCEPT);
    }
    state_t *p_s = (state_t *)active_process->p_s.reg_a1;
    bool p_prio = (bool)active_process->p_s.reg_a2;
    support_t *p_support_struct = (support_t *)active_process->p_s.reg_a3;

    /* spawn new process */
    pcb_t *c = spawn_process(p_prio);

    /* checks if there are enough resources */
    if (c == NULL) { /* lack of resorces */
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

    if (active_process->p_s.reg_a1 == (int)NULL) {
        return pass_up_or_die((memaddr)GENERALEXCEPT);
    }
    pandos_pid_t pid = active_process->p_s.reg_a1;
    pcb_t *p = active_process;

    /* If pid is not 0 then the target must be searched */
    if ((pid == 0 && active_process == NULL) ||
        (pid != 0 && (p = (pcb_t *)find_process(pid)) == NULL))
        scheduler_panic("Could not find process by pid: %p\n", pid);

    /* calls scheduler */
    kill_process(p);

    /* ??? */
    p->p_s.reg_v0 = pid;
    return CONTROL_BLOCK;
}

/* NSYS3 */
static inline scheduler_control_t syscall_passeren()
{
    if (active_process->p_s.reg_a1 == (int)NULL) {
        return pass_up_or_die((memaddr)GENERALEXCEPT);
    }
    return P((int *)active_process->p_s.reg_a1, active_process);
}

/* NSYS4 */
static inline scheduler_control_t syscall_verhogen()
{
    if (active_process->p_s.reg_a1 == (int)NULL) {
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

    if (active_process->p_s.reg_a1 == (int)NULL ||
        active_process->p_s.reg_a2 == (int)NULL) {
        return pass_up_or_die((memaddr)GENERALEXCEPT);
    }

    pandos_kprintf("active_process %p\n", active_process);
    int *cmd_addr = (int *)active_process->p_s.reg_a1;
    int cmd_value = (int)active_process->p_s.reg_a2;

    /* FIND DEVICE NUMBER BY cmd_addr */
    int *base = GET_DEVICE_FROM_COMMAND(cmd_addr);
    int i_n = 0, d_n = 0;

    for (int i = IL_DISK; i < 3 + N_EXT_IL; i++) {
        for (int j = 0; j < N_DEV_PER_IL; j++) {
            int *a = (int *)DEV_REG_ADDR(i, j);
            if (a == base) {
                i_n = i;
                d_n = j;
                i = 3 + N_EXT_IL;
                break;
            }
        }
    }

    int *sem;

    if (i_n == IL_DISK) {
        sem = disk_semaphores;
    } else if (i_n == IL_FLASH) {
        sem = tape_semaphores;
    } else if (i_n == IL_ETHERNET) {
        sem = ethernet_semaphores;
    } else if (i_n == IL_PRINTER) {
        sem = printer_semaphores;
    } else if (i_n == IL_TERMINAL) {

        if (TERMIMANL_CHECK_IS_WRITING(cmd_addr))
            sem = termw_semaphores;
        else
            sem = termr_semaphores;
    } else {
        /* scheduler_panic("Interrupt line not handled\n"); */
        pass_up_or_die(GENERALEXCEPT);
    }

    if (sem[d_n] > 0) {
        /* scheduler_panic("Device is already in use\n"); */
        pass_up_or_die(GENERALEXCEPT);
    }

    scheduler_control_t ctrl = P(&sem[d_n], active_process);

    active_process->p_s.status |= STATUS_IM(i_n);

    /* Finally write the data */
    *cmd_addr = cmd_value;

    return ctrl;
}

/* NSYS6 */
static inline scheduler_control_t syscall_get_cpu_time()
{
    active_process->p_s.reg_v0 = active_process->p_time;
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
    active_process->p_s.reg_v0 = (int)active_process->p_support;
    return CONTROL_RESCHEDULE;
}

/* NSYS9 */
static inline scheduler_control_t syscall_get_process_id()
{
    if (active_process->p_s.reg_a1 == (int)NULL) {
        return pass_up_or_die((memaddr)GENERALEXCEPT);
    }
    bool parent = (bool)active_process->p_s.reg_a1;
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
    const int id = (int)active_process->p_s.reg_a0, pid = active_process->p_pid;
    if (id <= 0 && checkUserMode()) {
        pandos_kfprintf(&kstderr,
                        "Negative syscalls cannot be called in user mode!\n");
        return pass_up_or_die((memaddr)GENERALEXCEPT);
    }
    switch (id) {
        case CREATEPROCESS:
            pandos_syscall("CREATEPROCESS", pid);
            return syscall_create_process();
            break;
        case TERMPROCESS:
            pandos_syscall("TERMPROCESS", pid);
            return syscall_terminate_process();
            break;
        case PASSEREN:
            pandos_syscall("PASSEREN", pid);
            return syscall_passeren();
            break;
        case VERHOGEN:
            pandos_syscall("VERHOGEN", pid);
            return syscall_verhogen();
            break;
        case DOIO:
            pandos_syscall("DOIO", pid);
            return syscall_do_io();
            break;
        case GETTIME:
            pandos_syscall("GETTIME", pid);
            return syscall_get_cpu_time();
            break;
        case CLOCKWAIT:
            pandos_syscall("CLOCKWAIT", pid);
            return syscall_wait_for_clock();
            break;
        case GETSUPPORTPTR:
            pandos_syscall("GETSUPPORTPTR", pid);
            return syscall_get_support_data();
            break;
        case GETPROCESSID:
            pandos_syscall("GETPROCESSID", pid);
            return syscall_get_process_id();
            break;
        case YIELD:
            pandos_syscall("YIELD", pid);
            return syscall_yeld();
            break;
        default:
            return pass_up_or_die((memaddr)GENERALEXCEPT);
            break;
    }

    return CONTROL_RESCHEDULE;
}
