/**
 * \file pcb.h
 * \brief PCB implementation
 *
 * \author Alessandro Frau
 * \author Gianmaria Rovelli
 * \date 17-01-2022
 */

#ifndef PANDOS_PCB_H
#define PANDOS_PCB_H

#include "os/const.h"
#include "os/list.h"
#include "os/types.h"

#define NULL_PID 0u

#ifdef PANDOS_TESTING

/**
 * \brief Returns the list head for the list of free pcbs.
 * \return A pointer to the local static variable `pcb_free`.
 */
const list_head *get_pcb_free();
#endif

/**
 * \brief Returns a pointer to the pcb table.
 * \return A pointer to the local static variable `pcb_table`.
 */
const pcb_t *get_pcb_table();

/**
 * \brief Initializes the table of pcb, namely `pcb_table`, and adds each entry
 * to the list list of free pcbs, namely `pcb_free`.
 * This function should be called only once during the initialization phase.
 */
void init_pcbs();

/**
 * \brief Frees the pcb pointer by `p` and adds it to the list of free pcbs.
 * \param[in] p The pcb to free.
 */
void free_pcb(pcb_t *p);

/**
 * \brief Returns the first free pcb from the list of pcbs.
 * \return Returns NULL if the list of free pcbs is empty, else returns the
 * first free pcb.
 */
pcb_t *alloc_pcb();

/**
 * \brief Checks if the `pcb_free` list contains a pcb.
 * \param[in] p The pcb that needs to be searched.
 * \return Returns true if the list is not null and contains the pcb, otherwise
 * returns false.
 */
bool pcb_free_contains(pcb_t *p);

/**
 * \brief Creates an empty list of pcb.
 * \param[in] head The head of the list to initialize.
 */
void mk_empty_proc_q(list_head *head);

/**
 * \brief Checks if the given list is empty.
 * \param[in] head The head of the list.
 * \return Returns true if the given list is empty or the head is NULL, false
 * otherwise.
 */
int empty_proc_q(list_head *head);

/**
 * \brief Inserts a pcb element at the tail of a pcb list, following a FIFO
 * pattern.
 * \param[in] head The head of the list.
 * \param[in] p The element which shall be added to the list.
 */
void insert_proc_q(list_head *head, pcb_t *p);

/**
 * \brief Returns a pointer to the first element of the list.
 * \param[in] head The head of the list.
 * \return Returns a pointer to the first element of the list. If the head is
 * NULL or empty it returns NULL.
 */
pcb_t *head_proc_q(list_head *head);

/**
 * \brief Removes the element from the process list pointed by `head` in a FIFO
 * fashion.
 * \param[in] head The head of the list (the dummy element) from where to remove
 * the first element.
 * \return Returns NULL if the list is empty or the head parameter is NULL,
 * otherwise returns the deleted element.
 */
pcb_t *remove_proc_q(list_head *head);

/**
 * \brief Removes the process pointed by `p` from the list of processes pointed
 *  by `head`.
 * \param[in] head The head of the list.
 * \param[in] p A pointer of the pcb to remove from list pointed by `head`.
 * \return Returns NULL if the list pointed by head doesn't contain the pcb
 * pointed by `p`, otherwise returns the deleted element.
 */
pcb_t *out_proc_q(list_head *head, pcb_t *p);

/**
 * \brief Checks if the pcb pointed by `p` has no children.
 * \param[in] p A pointer to the pcb.
 * \return Returns true if the pcb pointed by `p` has no child or is NULL,
 * otherwise returns FALSE.
 */
int empty_child(pcb_t *p);

/**
 * \brief Adds the pcb pointer by `p` as a child to `prnt` in a FIFO fashion.
 * \param[in] prnt The pcb where the new child shall be added.
 * \param[in] p The child to add.
 */
void insert_child(pcb_t *prnt, pcb_t *p);

/**
 * \brief Removes the first child of the pcb pointed by `p` in a FIFO fashion.
 * \param[in] p The pcb where the first child shall be removed.
 * \return Returns NULL if `p` has no child, otherwise returns the deleted
 * element.
 */
pcb_t *remove_child(pcb_t *p);

/**
 * \brief Removes the pcb pointed by `p` from the child list of its parent.
 * \param[in] p The element to be removed from the parent's child list.
 * \return Returns NULL if `p` doesn't have a parent, otherwise returns the
 * deleted element, `p`.
 */
pcb_t *out_child(pcb_t *p);

#endif /* PANDOS_PCB_H */
