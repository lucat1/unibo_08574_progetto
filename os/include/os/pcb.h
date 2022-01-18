#ifndef PANDOS_PCB_H
#define PANDOS_PCB_H

#include "list.h"
#include "const.h"
#include "types.h"

/**
 * initPcbs -       Initialize the list of free pcbs (pcbFree_h) from the table of pcbs (pcbFree_table)
 *  
 * This function should be called only once during the initialization phase
 * 
 * TODO:    Test this function
 */
void initPcbs();

/**
 * freePcb -    Adds p to the list of free pcbs
 * @p   :       New pcb that needs to be added to the free list
 * 
 * TODO:        Test this function
 */
void freePcb(pcb_t *p);

/**
 * allocPcb -   Gets the first free pcb from the list of pcbs
 * 
 * Returns null if the list of free pcbs is empty, the first free pcb otherwise
 * 
 * TODO:    Test this function
 */
pcb_t* allocPcb();

/**
 * mkEmptyProcQ -   creates an empty list of pcb
 * @head:           The head of the list
 * 
 * TODO:    Test this function
 * TODO:    Understand what's supposed to do
 *          because an empty list of pcb is just an empty list(?)
 */
void mkEmptyProcQ(struct list_head *head);

/**
 * emptyProcQ -     Checks if the list is empty
 * @head:           The head of the list
 * 
 * Returns true if the list is empty, false otherwise
 * 
 * TODO: Test this function
 */
int emptyProcQ(struct list_head *head);

/**
 * insertProcQ -    Insert a pcb element in the tail of a pcb list
 * @head:           The head of the list
 * @p   :           The elements that needs to be added to the list
 * 
 * TODO: Test this function
 */
void insertProcQ(struct list_head *head, pcb_t *p);

/**
 * headProcQ -  Returns the pointer to the first element of the list
 * @head:       The head of the list
 * 
 * Return the pointer to the first element of the list
 * 
 * TODO: check if the function needs to return a pointer or the actual pcb_t element
 * TODO: Test this function
 */
pcb_t* headProcQ(struct list_head *head);

/**
 * removeProcQ  -   Remove the element from the process list pointed by "head"
 * @head:           Head of the list (dummy element) where to remove first element
 * 
 * Return NULL if the list is empty, otherwise return the deleted element
 * TODO:            Test this function
 */
pcb_t* removeProcQ(struct list_head* head);

/**
 * outProc -    Remove the process pointed by "p" from the list of process
 *              pointed by "head"
 * @head:       Head of the list 
 * @p:          Pointer of PCB to remove from list pointed by "head"
 * 
 * TODO:            Test this function
 * Return NULL if list pointed by head doesn't contain PCB pointed by "p"
 * otherwise return the deleted element
 */
pcb_t* outProcQ(struct list_head* head, pcb_t *p);

/**
 * emptyChild -     Check if PCB pointed by p has not child
 * @p:              Pointer of PCB
 *
 * Return TRUE if PCB pointed by "p" has not child, otherwise return FALSE
 */
int emptyChild(pcb_t *p);

/**
 * insertChild -    Add a new PCB as child to "prnt"
 * @prnt:           PCP where to add the new child
 * @p:              Child to be added
 *
 * Return void
 * TODO:            Test this function
 */
void insertChild(pcb_t *prnt, pcb_t *p);

/**
 * removeChild -    Remove the first child of PCB pointed by "p"
 * @p:              PCB where to remove first child element
 * 
 * Return NULL if "p" has not child, otherwise return the deleted element
 * TODO:            Test this function
 */
pcb_t* removeChild(pcb_t *p);

/**
 * outChild -       Remove the PCB pointed by "p" from the child (p_child) of the parent (p_parent)
 * @p:              Element to be removed from parent's child list
 * 
 * Return NULL if "p" has not a parent, otherwise return the deleted element ("p")
 */
pcb_t *outChild(pcb_t* p);

#endif /* PANDOS_PCB_H */
