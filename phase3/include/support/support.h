/**
 * \file support.h
 * \brief Non-TLB exceptions handling and Support Level semaphores
 *
 * \author Gianmaria Rovelli
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 05-05-2022
 */

#ifndef PANDOS_SUPPORT_H
#define PANDOS_SUPPORT_H

/**
 * \brief Initialize all mutex for uninterrupted streaming devices sequential
 * access.
 */
extern void init_sys_semaphores();

/**
 * \brief Perform a PASSEREN operation of the Master Semaphore: that is,
 * wait for a U-Proc to terminate.
 */
extern void master_semaphore_p();

/**
 * \brief A general exception handler for the Support Level.
 */
extern void support_generic();

/**
 * \brief A Support Level exception handler for Program Trap exceptions.
 */
extern void support_trap();

#endif /* PANDOS_SUPPORT_H */
