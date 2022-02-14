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
 * parameter. \param new The new list head to be assigned to the variable.
 */
void set_semd_free(const list_head new);

// The following local helper functions are made public only during testing to
// assert they behave as expected.

/**
 * \brief Allocates a new sempaphore descriptor returning its address.
 * Internally this function tries to take a semd from the semd_free list, and
 * returns an error if the list is empty. Then the semaphore gets added to the
 * list of busy semaphores and finally returned.
 * \param sem_addr The semaphore key.
 * \return A pointer to the newly allocated semaphore, NULL on error.
 */
semd_t *alloc_semd(int *sem_addr);
/**
 * \brief Deallocates an in-use semaphore.
 * The procedure fails if the semaphore is NULL or it has no process blocked on
 * it (which means it shouldn't be regarded as `in-use`). After this preliminary
 * check the semaphore is moved from the free to the busy list.
 * \param sem_addr The semaphore key to look for.
 * \return Returns true if an error occured, false otherwise.
 */
bool free_semd(semd_t *semd);
/**
 * \brief Finds a semaphore descriptor in a given list by its key.
 * An error is returned if either the list is NULL or the address is NULL.
 * Otherwise the result of a list_search is returned, which may be NULL if no
 * matching item is found. \param  The list of seamphores search. \param  The
 * semaphore key to look for. \return Returns the address of the queried
 * semaphore if found, NULL otherwise.
 */
semd_t *find_semd(list_head *list, int *sem_addr);
#endif

void init_asl();
int insert_blocked(int *sem_addr, pcb_t *p);
pcb_t *out_blocked(pcb_t *pcb);
pcb_t *head_blocked(int *sem_addr);
pcb_t *remove_blocked(int *sem_addr);

#endif /* PANDOS_ASL_H */
