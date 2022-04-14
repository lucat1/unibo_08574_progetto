/**
 * \file semaphores.h
 * \brief Device semaphore global values and public interface.
 *
 * \author Gianmaria Rovelli
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 17-03-2022
 */

#ifndef PANDOS_OS_SEMAPHORE_H
#define PANDOS_OS_SEMAPHORE_H

#include "arch/devices.h"
#include "os/const.h"
#include "os/scheduler.h"

/**
 * \brief Executes a passeren operation by a given process on the specified
 * semaphore.
 * \param[in,out] sem_addr The semaphore on which the operation should be
 * executed.
 * \param[in,out] p The PCB describing the process that should execute the
 * operation.
 * \return The appropriate control for the scheduler after the operation.
 */
extern scheduler_control_t P(int *const sem_addr, pcb_t *const p);

/**
 * \brief Executes a verhogen operation by a given process on the specified
 * semaphore.
 * \param[in,out] sem_addr The semaphore on which the operation should be
 * executed.
 * \return The appropriate control for the scheduler after the operation.
 */
extern pcb_t *V(int *const sem_addr);

/**
 * \brief Initializes every semaphore at value 0.
 */
extern void init_semaphores();

/**
 * \brief Retrieves the desired device semaphore.
 * \param[in] int_l The class of the device associated to the semaphore.
 * \param[in] dev_n The number of the device associated to the semaphore.
 * \param[in] is_w Whether the semaphore should be in write mode or not.
 * \return A pointer to the requested semaphore if it exist, or NULL on
 * failure.
 */
extern int *get_semaphore(int int_l, int dev_n, bool is_w);

/**
 * \brief Retrieves the timer semaphore.
 * \return A pointer to the timer semaphore.
 */
extern int *get_timer_semaphore();

#endif /* PANDOS_OS_SEMAPHORE_H */
