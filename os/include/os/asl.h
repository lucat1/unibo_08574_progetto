/** \file
 * \brief Active Semaphore List
 *
 * \author Luca Tagliavini
 * \author Stefano Volpe
 * \date 17-01-2022
 */

#ifndef PANDOS_ASL_H
#define PANDOS_ASL_H

#include "os/list.h"
#include "os/pcb.h"
#include "os/types.h"

/**
 * \brief Initializes the list of free and in-use semaphores, placing all
 * available semaphores in the free pool.
 */
void init_asl();
/**
 * \brief Blocks the given pcb on the required semaphore, adding a new one if
 * necessary. The function can fail if any of these conditions are met:
 * - The semaphore address is NULL (error code 1).
 * - The pcb pointer is NULL (error code 2).
 * - The pcb is already blocked on another semaphore (error code 3).
 * - There is no more room to allocate a new semaphore (error code 4).
 * \param[in] sem_addr A pointer to the semaphore address
 * \param[in] p A pointer to the pcb to block
 * \return Returns zero if the procedure exited successfully, an error code
 * otherwise.
 */
int insert_blocked(int *sem_addr, pcb_t *p);
/**
 * \brief Removes a a blocked pcb from its semaphore, removing the semaphore
 * from the list of in-use semds if no more processes are blocked on it.
 * \param[in] pcb A pointer to the pcb to unblock
 * \return The unblocked pcb, or NULL if an error is encountered.
 */
pcb_t *out_blocked(pcb_t *pcb);
/**
 * \brief Returns the first pcb blocked on the given semaphore.
 * \param[in] sem_addr A pointer to the semaphore address
 * \return The first pcb blocked on the semaphore, or NULL if an error occoured.
 */
pcb_t *head_blocked(int *sem_addr);
/**
 * \brief Removes the first pcb blocked on the given semaphore.
 * \param[in] sem_addr A pointer to the semaphore address
 * \return The unblocked pcb, or NULL if an error is encountered.
 */
pcb_t *remove_blocked(int *sem_addr);

#endif /* PANDOS_ASL_H */
