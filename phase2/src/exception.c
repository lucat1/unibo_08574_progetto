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
#include "os/ctypes.h"
#include "os/puod.h"
#include "os/scheduler.h"
#include "os/scheduler_impl.h"
#include "os/semaphores.h"
#include "os/syscall.h"
#include "os/util.h"
#include <umps/arch.h>
#include <umps/cp0.h>
#include <umps/libumps.h>

/**
 * \brief Finds device number of device that generated an interrupt.
 * \param[in] bitmap Bitmap of the interrupt line where device has to be looked
 * for. \return Device number that generated an interrupt.
 */
size_t find_device_number(memaddr *bitmap)
{
    size_t device_n = 0;

    size_t val = *bitmap;
    while (val > 1 && device_n < N_DEV_PER_IL) {
        ++device_n;
        val >>= 1;
    }
    return device_n;
}

/**
 * \brief Handler for inter-processor interrupts
 * \return Control preserve as current process
 */
static inline scheduler_control_t interrupt_ipi()
{
    /* Could be safely ignored */
    return CONTROL_PRESERVE(active_process);
}

/**
 * \brief Handler for processor local timer interrupts (PLT)
 * \return Control reschedule
 */
static inline scheduler_control_t interrupt_local_timer()
{
    reset_local_timer();
    return CONTROL_RESCHEDULE;
}

/**
 * \brief Handler for interval timer interrupts (Bus)
 * \return Control preserve as current process
 */
static inline scheduler_control_t interrupt_timer()
{
    reset_timer();
    while (*get_timer_semaphore() != 1)
        V(get_timer_semaphore());
    return CONTROL_PRESERVE(active_process);
}

static inline scheduler_control_t return_status(pcb_t *p, int status)
{
    if (p == active_process) {
        if (active_process == NULL)
            scheduler_panic("No active process (Interrupt Generic)\n");
        active_process->p_s.reg_v0 = status;
        return CONTROL_RESCHEDULE;
    } else {
        p->p_s.reg_v0 = status;
        return CONTROL_PRESERVE(active_process);
    }
}

/**
 * \brief Handler for generic interrupts (Disk, Flash, Network, Printer)
 * \return Control depends on semaphore of the device involved
 */
static inline scheduler_control_t interrupt_generic(int cause)
{
    int il = IL_DISK;
    /* inverse priority */
    for (int i = IL_DISK; i < IL_PRINTER + 1; i++) {
        if (cause & CAUSE_IP(i)) {
            il = i;
            break;
        }
    }
    size_t devicenumber = find_device_number((memaddr *)CDEV_BITMAP_ADDR(il));
    int *sem = get_semaphore(il, devicenumber, false);

    devregarea_t *device_regs = (devregarea_t *)RAMBASEADDR;
    dtpreg_t *dtp_reg =
        &device_regs->devreg[il - DEV_IL_START][devicenumber].dtp;
    int status = dtp_reg->status;

    if ((status & TERMSTATMASK) == DEV_STATUS_NOTINSTALLED)
        scheduler_panic("Device is not installed!\n");

    pcb_t *p = V(sem);
    scheduler_control_t ctrl = return_status(p, status);

    /* Send the acknowledgement to the device */
    dtp_reg->command = DEV_C_ACK;
    return ctrl;
}

/**
 * \brief Handler for terminal interrupts
 * \return Control depends on semaphore of the terminal involved
 */
static inline scheduler_control_t interrupt_terminal()
{
    size_t devicenumber =
        find_device_number((memaddr *)CDEV_BITMAP_ADDR(IL_TERMINAL));

    devregarea_t *device_regs = (devregarea_t *)RAMBASEADDR;
    termreg_t *term_reg =
        &device_regs->devreg[IL_TERMINAL - IL_DISK][devicenumber].term;

    memaddr(statuses[2]) = {term_reg->transm_status, term_reg->recv_status};
    memaddr(*commands[2]) = {&term_reg->transm_command,
                             &term_reg->recv_command};
    int *sem[] = {get_semaphore(IL_TERMINAL, devicenumber, true),
                  get_semaphore(IL_TERMINAL, devicenumber, false)};

    for (int i = 0; i < 2; ++i) {
        int status = statuses[i];
        if ((status & TERMSTATMASK) == DEV_STATUS_NOTINSTALLED)
            scheduler_panic("Device is not installed!\n");

        if ((status & TERMSTATMASK) == DEV_STATUS_TERMINAL_OK) {
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
            /* Send the acknowledgement to the device */
            return ctrl;
        }
    }

    scheduler_panic("Unexpected interrupt from terminal device %d\n",
                    devicenumber);
    return CONTROL_BLOCK;
}

/**
 * \brief Handler for interrupts
 * \param[in] cause Interrupt cause
 * \return Control depends on semaphore of the terminal involved
 */
static inline scheduler_control_t interrupt_handler(size_t cause)
{
    if (cause & CAUSE_IP(IL_IPI))
        return interrupt_ipi();
    else if (cause & CAUSE_IP(IL_CPUTIMER))
        return interrupt_local_timer();
    else if (cause & CAUSE_IP(IL_TIMER))
        return interrupt_timer();
    else if (cause & (CAUSE_IP(IL_DISK) | CAUSE_IP(IL_FLASH) |
                      CAUSE_IP(IL_ETHERNET) | CAUSE_IP(IL_PRINTER)))
        return interrupt_generic(cause);
    else if (cause & CAUSE_IP(IL_TERMINAL))
        return interrupt_terminal();

    /* The newly unblocked pcb is enqueued back on the Ready Queue and
     * control is returned to the Current Process unless the newly unblocked
     * process has higher prority of the Current Process.
     * */
    return CONTROL_RESCHEDULE;
}

/**
 * \brief Exceptions handler
 */
inline void exception_handler()
{
    int now_tod;
    if (active_process != NULL) {
        store_tod(&now_tod);
        active_process->p_time += (now_tod - start_tod);
    }
    scheduler_control_t ctrl;
    if (active_process != NULL) {
        pandos_memcpy(&active_process->p_s, (state_t *)BIOSDATAPAGE,
                      sizeof(state_t));
    }

    switch (CAUSE_GET_EXCCODE(get_cause())) {
        case 0:
            ctrl = interrupt_handler(get_cause());
            break;
        case 1:
        case 2:
        case 3:
            pandos_kprintf("PGFAULT %d\n", CAUSE_GET_EXCCODE(get_cause()));
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

            pandos_kprintf("EXCP %d\n", CAUSE_GET_EXCCODE(get_cause()) + 1);
            ctrl = pass_up_or_die((memaddr)GENERALEXCEPT);
            break;
    }

    schedule(ctrl.pcb, ctrl.enqueue);
}
