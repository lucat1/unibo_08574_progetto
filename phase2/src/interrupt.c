/**
 * \file interrupt.c
 * \brief Interrupt and trap handler
 *
 * \author Alessandro Frau
 * \author Gianmaria Rovelli
 * \author Luca Tagliavini
 * \date 20-03-2022
 */

#include "os/interrupt.h"
#include "os/asl.h"
#include "os/const.h"
#include "os/scheduler.h"
#include "os/scheduler_impl.h"
#include "os/semaphores.h"
#include "os/util.h"
#include <umps/arch.h>
#include <umps/libumps.h>

#define pandos_interrupt(str) pandos_kprintf("<< INTERRUPT(" str ")\n")

/* finds device number of device that generates interrupt */
int find_device_number(memaddr *bitmap)
{
    int device_n = 0;

    while (*bitmap > 1) {
        ++device_n;
        *bitmap >>= 1;
    }
    return device_n;
}

static inline scheduler_control_t interrupt_ipi()
{
    /* Could be safetly ignored */

    return CONTROL_PRESERVE(active_process);
}

static inline scheduler_control_t interrupt_local_timer()
{
    reset_local_timer();
    return CONTROL_RESCHEDULE;
}

static inline scheduler_control_t interrupt_timer()
{
    // pcb_t *p;

    reset_timer();
    while (timer_semaphore != 1)
        V(&timer_semaphore);
    // timer_semaphore = 0;
    pandos_kprintf("act: %p\n", active_process);
    return CONTROL_PRESERVE(active_process);
}

static inline scheduler_control_t interrupt_generic(int cause)
{

    /* TODO */
    int il = IL_DISK;
    int *sem[] = {disk_semaphores, tape_semaphores, ethernet_semaphores,
                  printer_semaphores};
    /* inverse priority */
    for (int i = IL_DISK; i < IL_PRINTER; i++) {
        if (IL_ACTIVE(cause, i)) {
            il = i;
            break;
        }
    }

    int devicenumber = find_device_number((memaddr *)CDEV_BITMAP_ADDR(il));

    int i = il - IL_DISK;

    devregarea_t *device_regs = (devregarea_t *)RAMBASEADDR;
    dtpreg_t *dtp_reg = &device_regs->devreg[i][devicenumber].dtp;

    int status = dtp_reg->status;

    scheduler_control_t ctrl = CONTROL_BLOCK;

    if ((status & TERMSTATMASK) != DEV_STATUS_NOTINSTALLED) {

        pcb_t *p = V(&sem[i][devicenumber]);
        if (p == NULL || p == active_process) {
            if (active_process != NULL) {
                active_process->p_s.reg_v0 = status;
                ctrl = CONTROL_RESCHEDULE;
            } else {
                scheduler_panic("No active process (Interrupt Generic)\n");
            }
        } else {
            p->p_s.reg_v0 = status;
            ctrl = CONTROL_PRESERVE(active_process);
        }
    } else {
        scheduler_panic("Device is not installed!\n");
    }

    /* ACK al device */
    dtp_reg->command = DEV_C_ACK;
    return ctrl;
}

static inline scheduler_control_t interrupt_terminal()
{

    int devicenumber =
        find_device_number((memaddr *)CDEV_BITMAP_ADDR(IL_TERMINAL));

    devregarea_t *device_regs = (devregarea_t *)RAMBASEADDR;
    termreg_t *term_reg =
        &device_regs->devreg[IL_TERMINAL - IL_DISK][devicenumber].term;

    /* TODO : order is important, check */
    memaddr(statuses[2]) = {term_reg->transm_status, term_reg->recv_status};
    memaddr(*commands[2]) = {&term_reg->transm_command,
                             &term_reg->recv_command};
    int *sem[] = {termw_semaphores, termr_semaphores};

    // pandos_kfprintf(&kverb, "\n[-] TERM INT START (%d)\n", act_pid);
    for (int i = 0; i < 2; ++i) {
        int status = statuses[i];
        if ((status & TERMSTATMASK) == DEV_STATUS_NOTINSTALLED)
            scheduler_panic("Device is not installed!\n");

        pcb_t *p = V(&sem[i][devicenumber]);
        scheduler_control_t ctrl;
        if (p == NULL || p == active_process) {
            if (active_process != NULL) {
                active_process->p_s.reg_v0 = status;
                ctrl = CONTROL_RESCHEDULE;
            } else {
                scheduler_panic("No active process (Interrupt Terminal)\n");
            }
        } else {
            p->p_s.reg_v0 = status;
            ctrl = CONTROL_PRESERVE(active_process);
        }

        *commands[i] = DEV_C_ACK;
        /* do the first one */
        return ctrl;
    }
    scheduler_panic("Terminal did not ACK\n");
    /* Make C happy */
    return CONTROL_BLOCK;
}

scheduler_control_t interrupt_handler()
{
    int cause = getCAUSE();

    if (IL_ACTIVE(cause, IL_IPI)) {
        pandos_interrupt("IL_IPI");
        return interrupt_ipi();
    } else if (IL_ACTIVE(cause, IL_CPUTIMER)) {
        pandos_interrupt("LOCAL_TIMER");
        return interrupt_local_timer();
    } else if (IL_ACTIVE(cause, IL_TIMER)) {
        pandos_interrupt("TIMER");
        return interrupt_timer();
    } else if (IL_ACTIVE(cause, IL_DISK) || IL_ACTIVE(cause, IL_FLASH) ||
               IL_ACTIVE(cause, IL_ETHERNET) || IL_ACTIVE(cause, IL_PRINTER)) {
        pandos_interrupt("GENERIC");
        return interrupt_generic(cause);
    } else if (IL_ACTIVE(cause, IL_TERMINAL)) {
        pandos_interrupt("TERMINAL");
        return interrupt_terminal();
    } else {
        pandos_interrupt("UNKNOWN");
        pandos_kfprintf(&kstdout, "UNKNOWN CODE : %d\n",
                        IL_ACTIVE(cause, IL_IPI));
        return CONTROL_PRESERVE(active_process);
    }

    /* The newly unblocked pcb is enqueued back on the Ready Queue and control
     * is returned to the Current Process unless the newly unblocked process
     * has higher prority of the Current Process.
     * */
    return CONTROL_RESCHEDULE;
}
