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
pcb_t *get_pcb_table();
list_head *get_pcb_free();
#endif

/**
 * \brief   Initialize the list of free pcbs (pcbFree_h) from the table of pcbs
 * (pcbFree_table) This function should be called only once during the
 * initialization phase
 *
 * \todo    Test this function
 */
void init_pcbs();

/**
 * \brief   Adds p to the list of free pcbs
 * \param[in] p New pcb that needs to be added to the free list
 *
 * \todo    Test this function
 */
void free_pcb(pcb_t *p);

/**
 * \brief   Gets the first free pcb from the list of pcbs
 *
 * \return  Returns null if the list of free pcbs is empty, the first free pcb
 * otherwise
 *
 * \todo    Test this function
 */
pcb_t *alloc_pcb();

/**
 *  \brief Checks if the pcb_free list contains a pcb
 *
 *  \param[in] p Pcb that needs to be checked
 *
 *  \return Returns true if the list contains the pcb, false otherwise
 *
 *  \todo Test this function
 */
bool pcb_free_contains(pcb_t *p);

/**
 * \brief   creates an empty list of pcb
 * \param[in] head  The head of the list
 *
 * \todo    Test this function
 * \todo    Understand what's supposed to do
 *          because an empty list of pcb is just an empty list(?)
 */
void mk_empty_proc_q(list_head *head);

/**
 * \brief     Checks if the list is empty
 * \param[in] head  The head of the list
 *
 * \return  Returns true if the list is empty, false otherwise
 *
 * \todo    Test this function
 */
int empty_proc_q(list_head *head);

/**
 * \brief   Insert a pcb element in the tail of a pcb list (FIFO)
 * \param[in] head  The head of the list
 * \param[in] p The elements that needs to be added to the list
 *
 * \todo Test this function
 */
void insert_proc_q(list_head *head, pcb_t *p);

/**
 * \brief   Returns the pointer to the first element of the list
 * \param[in] head:       The head of the list
 *
 * \return  Return the pointer to the first element of the list
 *
 * \todo    check if the function needs to return a pointer or the actual pcb_t
 * element \todo    Test this function
 */
pcb_t *head_proc_q(list_head *head);

/**
 * \brief   Remove the element from the process list pointed by "head" (FIFO)
 * \param[in] head  Head of the list (dummy element) where to remove first
 * element
 *
 * \return  Return NULL if the list is empty, otherwise return the deleted
 * element
 *
 * \todo    Test this function
 */
pcb_t *remove_proc_q(list_head *head);

/**
 * \brief   Remove the process pointed by "p" from the list of process
 *          pointed by "head"
 *
 * \param[in] head  Head of the list
 * \param[in] p Pointer of PCB to remove from list pointed by "head"
 *
 * \return  Return NULL if list pointed by head doesn't contain PCB pointed by
 * "p" otherwise return the deleted element
 *
 * \todo    Test this function
 */
pcb_t *out_proc_q(list_head *head, pcb_t *p);

/**
 * \brief   Check if PCB pointed by p has not child
 * \param[in] p Pointer of PCB
 *
 * \return  Return TRUE if PCB pointed by "p" has not child, otherwise return
 * FALSE
 */
int empty_child(pcb_t *p);

/**
 * \brief   Add a new PCB as child to "prnt" (FIFO)
 * \param[in] prnt  PCP where to add the new child
 * \param[in] p Child to be added
 *
 * \todo    Test this function
 */
void insert_child(pcb_t *prnt, pcb_t *p);

/**
 * \brief   Remove the first child of PCB pointed by "p" (FIFO)
 * \param[in] p PCB where to remove first child element
 *
 * \return  Return NULL if "p" has not child, otherwise return the deleted
 * element
 *
 * \todo    Test this function
 */
pcb_t *remove_child(pcb_t *p);

/**
 * \brief   Remove the PCB pointed by "p" from the child (p_child) of the parent
 * (p_parent) \param[in] p Element to be removed from parent's child list
 *
 * \return  Return NULL if "p" has not a parent, otherwise return the deleted
 * element ("p")
 */
pcb_t *out_child(pcb_t *p);

#endif /* PANDOS_PCB_H */
