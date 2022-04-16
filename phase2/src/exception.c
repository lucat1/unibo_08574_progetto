/**
 * \file exception.c
 * \brief Implementation \ref exception.h.
 *
 * \author Alessandro Frau
 * \author Gianmaria Rovelli
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 30-03-2022
 */

#include "exception.h"
#include "os/asl.h"
#include "os/puod.h"
#include "os/scheduler.h"
#include "os/scheduler_impl.h"
#include "os/semaphores.h"
#include "os/syscall.h"
#include "os/util.h"
#include <umps/arch.h>
#include <umps/cp0.h>
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
    pandos_interrupt("IL_IPI");

    return CONTROL_PRESERVE(active_process);
}

static inline scheduler_control_t interrupt_local_timer()
{
    pandos_interrupt("LOCAL_TIMER");
    reset_local_timer();
    return CONTROL_RESCHEDULE;
}

static inline scheduler_control_t interrupt_timer()
{
    pandos_interrupt("TIMER");
    reset_timer();
    while (*get_timer_semaphore() != 1)
        V(get_timer_semaphore());
    return CONTROL_PRESERVE(active_process);
}

static inline scheduler_control_t interrupt_generic(int cause)
{
    pandos_interrupt("GENERIC");
    /* TODO */
    int il = IL_DISK;
    /* inverse priority */
    for (int i = IL_DISK; i < IL_PRINTER; i++) {
        if (cause & CAUSE_IP(i)) {
            il = i;
            break;
        }
    }
    int devicenumber = find_device_number((memaddr *)CDEV_BITMAP_ADDR(il));
    int *sem = get_semaphore(il, devicenumber, false);

    devregarea_t *device_regs = (devregarea_t *)RAMBASEADDR;
    dtpreg_t *dtp_reg = &device_regs->devreg[il - IL_DISK][devicenumber].dtp;

    int status = dtp_reg->status;

    scheduler_control_t ctrl = CONTROL_BLOCK;

    if ((status & TERMSTATMASK) == DEV_STATUS_NOTINSTALLED)
        scheduler_panic("Device is not installed!\n");

    pcb_t *p = V(sem);
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

    /* ACK al device */
    dtp_reg->command = DEV_C_ACK;
    return ctrl;
}

static inline scheduler_control_t interrupt_terminal()
{
    pandos_interrupt("TERMINAL");
    int devicenumber =
        find_device_number((memaddr *)CDEV_BITMAP_ADDR(IL_TERMINAL));

    devregarea_t *device_regs = (devregarea_t *)RAMBASEADDR;
    termreg_t *term_reg =
        &device_regs->devreg[IL_TERMINAL - IL_DISK][devicenumber].term;

    /* TODO : order is important, check */
    memaddr(statuses[2]) = {term_reg->transm_status, term_reg->recv_status};
    memaddr(*commands[2]) = {&term_reg->transm_command,
                             &term_reg->recv_command};
    int *sem[] = {get_semaphore(IL_TERMINAL, devicenumber, false),
                  get_semaphore(IL_TERMINAL, devicenumber, true)};

    // pandos_kfprintf(&kverb, "\n[-] TERM INT START (%d)\n", act_pid);
    for (int i = 0; i < 2; ++i) {
        int status = statuses[i];
        /* todo: dobbiamo davvero farlo?, lo fa gia' il codice livello utente */
        if ((status & TERMSTATMASK) != DEV_STATUS_TERMINAL_OK)
            scheduler_panic("Device is not installed!\n");

        pcb_t *p = V(sem[i]);
        scheduler_control_t ctrl;
        if (p == NULL || p == active_process) {
            if (active_process == NULL)
                scheduler_panic("No active process (Interrupt Terminal)\n");
            active_process->p_s.reg_v0 = status;
            ctrl = CONTROL_RESCHEDULE;
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

static inline scheduler_control_t interrupt_handler(size_t cause)
{
    if (cause & CAUSE_IP(IL_IPI))
        return interrupt_ipi();
    else if (cause & CAUSE_IP(IL_CPUTIMER))
        return interrupt_local_timer();
    else if (cause & CAUSE_IP(IL_TIMER))
        return interrupt_timer();
    else if (cause & CAUSE_IP(IL_DISK) & CAUSE_IP(IL_FLASH) &
             CAUSE_IP(IL_ETHERNET) & CAUSE_IP(IL_PRINTER))
        return interrupt_generic(cause);
    else if (cause & CAUSE_IP(IL_TERMINAL))
        return interrupt_terminal();
    else
        pandos_interrupt("UNKNOWN");

    /* The newly unblocked pcb is enqueued back on the Ready Queue and
     * control is returned to the Current Process unless the newly unblocked
     * process has higher prority of the Current Process.
     * */
    return CONTROL_RESCHEDULE;
}

inline void exception_handler()
{
    int now_tod;
    if (active_process != NULL) {
        store_tod(&now_tod);
        active_process->p_time += (now_tod - start_tod);
    }
    scheduler_control_t ctrl;
    if (active_process != NULL)
        pandos_memcpy(&active_process->p_s, (state_t *)BIOSDATAPAGE,
                      sizeof(state_t));

    switch (CAUSE_GET_EXCCODE(get_cause())) {
        case 0:
            ctrl = interrupt_handler(get_cause());
            break;
        case 1:
        case 2:
        case 3:
            ctrl = pass_up_or_die((memaddr)PGFAULTEXCEPT);
            break;
        case 8:
            if (active_process == NULL)
                scheduler_panic(
                    "A syscall happened while active_process was NULL\n");
            store_tod(&start_tod);
            ctrl = syscall_handler();
            store_tod(&now_tod);
            active_process->p_time += (now_tod - start_tod);
            /* ALWAYS increment the PC to prevent system call loops */
            active_process->p_s.pc_epc += WORD_SIZE;
            active_process->p_s.reg_t9 += WORD_SIZE;
            break;
        default: /* 4-7, 9-12 */
            ctrl = pass_up_or_die((memaddr)GENERALEXCEPT);
            break;
    }

    schedule(ctrl.pcb, ctrl.enqueue);
}
