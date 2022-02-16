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

#ifdef PANDOS_TESTING
// The following are utility functions used by test code.
/**
 * \brief Returns the table of semaphore descriptors.
 * \return A pointer to the local static variable `semd_table`.
 */
semd_t *get_semd_table();
/**
 * \brief Returns the list head for the list of free semaphores.
 * \return A pointer to the local static variable `semd_free`.
 */
list_head *get_semd_free();
/**
 * \brief Returns the list head for the list of in use semaphores.
 * \return A pointer to the local static variable `semd_h`.
 */
list_head *get_semd_h();
/**
 * \brief Sets the content of the local variable `semd_free` to the given
 * parameter.
 * \param[in] new The new list head to be assigned to the variable.
 */
void set_semd_free(const list_head new);

// The following local helper functions are made public only during testing to
// assert they behave as expected.

/**
 * \brief Allocates a new sempaphore descriptor returning its address.
 * Internally this function tries to take a semd from the semd_free list, and
 * returns an error if the list is empty. Then the semaphore gets added to the
 * list of busy semaphores and finally returned.
 * \param[in] sem_addr The semaphore key.
 * \return A pointer to the newly allocated semaphore, NULL on error.
 */
semd_t *alloc_semd(int *sem_addr);
/**
 * \brief Deallocates an in-use semaphore.
 * The procedure fails if the semaphore is NULL or it has remaining processes
 * blocked on it. After this preliminary check the semaphore is moved from the
 * free to the busy list.
 * \param[in] sem_addr The semaphore key to look for.
 * \return Returns true if an error occured, false otherwise.
 */
bool free_semd(semd_t *semd);
/**
 * \brief Finds a semaphore descriptor in a given list by its key.
 * An error is returned if either the list is NULL or the address is NULL.
 * Otherwise the result of a list_search is returned, which may be NULL if no
 * matching item is found.
 * \param[in] list The list of seamphores search.
 * \param[in] sem_addr The semaphore key to look for.
 * \return Returns the address of the queried semaphore if found, NULL
 * otherwise.
 */
semd_t *find_semd(list_head *list, int *sem_addr);
#endif

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
