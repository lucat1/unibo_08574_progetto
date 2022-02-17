/**
 * \file pcb.h
 * \brief PCB implementation
 *
 * \author Alessandro Frau
 * \author Gianmaria Rovelli
 * \date 17-01-2022
 *
 */

#ifndef PANDOS_PCB_H
#define PANDOS_PCB_H

#include "const.h"
#include "list.h"
#include "types.h"

#ifdef PANDOS_TESTING
/**
 * \brief Returns a pointer to the pcb table.
 * \return A pointer to the local static variable `pcb_table`.
 */
pcb_t *get_pcb_table();

/**
 * \brief Returns the list head for the list of free pcbs.
 * \return A pointer to the local static variable `pcb_free`.
 */
list_head *get_pcb_free();
#endif

/**
 * \brief   Initialize the list of free pcbs (pcb_free) from the table of pcbs
 *          (pcb_table) This function should be called only once during the
 *          initialization phase.
 */
void init_pcbs();

/**
 * \brief   Adds p to the list of free pcbs, if p is NULL then it returns false.
 * \param[in] p New pcb that needs to be added to the free list.
 */
void free_pcb(pcb_t *p);

/**
 * \brief   Gets the first free pcb from the list of pcbs.
 * \return  Returns null if the list of free pcbs is empty, the first free pcb
 *          otherwise.
 */
pcb_t *alloc_pcb();

/**
 *  \brief Checks if the pcb_free list contains a pcb.
 *  \param[in] p Pcb that needs to be checked.
 *  \return Returns true if the list contains the pcb, false otherwise (if pcb is NULL it returns false).
 */
bool pcb_free_contains(pcb_t *p);

/**
 * \brief   Creates an empty list of pcb.
 * \param[in] head  The head of the list.
 */
void mk_empty_proc_q(list_head *head);

/**
 * \brief     Checks if the list is empty.
 * \param[in] head  The head of the list.
 * \return  Returns true if the list is empty or the head is NULL, false otherwise.
 */
int empty_proc_q(list_head *head);

/**
 * \brief   Insert a pcb element in the tail of a pcb list (FIFO).
 * \param[in] head  The head of the list.
 * \param[in] p The elements that needs to be added to the list.
 */
void insert_proc_q(list_head *head, pcb_t *p);

/**
 * \brief   Returns the pointer to the first element of the list.
 * \param[in] head  The head of the list.
 * \return  Return the pointer to the first element of the list, if the head is NULL it returns NULL.
 */
pcb_t *head_proc_q(list_head *head);

/**
 * \brief   Remove the element from the process list pointed by `head` (FIFO)
 * \param[in] head  Head of the list (dummy element) where to remove first.
 *                  element.
 * \return  Return NULL if the list is empty or the head parameter is NULL, otherwise return the deleted
 *          element.
 */
pcb_t *remove_proc_q(list_head *head);

/**
 * \brief   Remove the process pointed by `p` from the list of process
 *          pointed by `head`.
 * \param[in] head  Head of the list.
 * \param[in] p Pointer of PCB to remove from list pointed by `head`.
 * \return  Return NULL if list pointed by head doesn't contain PCB pointed by
 *          `p` otherwise return the deleted element.
 */
pcb_t *out_proc_q(list_head *head, pcb_t *p);

/**
 * \brief   Check if PCB pointed by `p` has not children.
 * \param[in] p Pointer of PCB.
 * \return  Return TRUE if PCB pointed by `p` has not children or is NULL, otherwise return
 *          FALSE.
 */
int empty_child(pcb_t *p);

/**
 * \brief   Add a `p` as child to `prnt` (FIFO).
 * \param[in] prnt  PCP where to add the new child.
 * \param[in] p Child to be added.
 */
void insert_child(pcb_t *prnt, pcb_t *p);

/**
 * \brief   Remove the first child of PCB pointed by `p` (FIFO)
 * \param[in] p PCB where to remove first child element.
 * \return  Return NULL if `p` has not child, otherwise return the deleted
 *          element.
 */
pcb_t *remove_child(pcb_t *p);

/**
 * \brief   Remove the PCB pointed by `p` from the child (p_child) of the parent
 *          (p_parent).
 * \param[in] p Element to be removed from parent's child list.
 * \return  Return NULL if `p` has not a parent, otherwise return the deleted
 *          element (`p`).
 */
pcb_t *out_child(pcb_t *p);

#endif /* PANDOS_PCB_H */
