/**
 * \file pcb.c
 * \brief Implementation \ref pcb.h
 *
 * \author Alessandro Frau
 * \author Rovelli Gianmaria
 * \date 17-01-2022
 *
 */

#include "os/pcb.h"
#include "os/list.h"
#include "os/types.h"

// TODO: Change the names of pcbFree_table and pcbFree_h
static pcb_t pcb_table[MAX_PROC];
static list_head pcb_free;

// This function should be called only once during the initialization phase
void init_pcbs()
{
    // Initialize the list
    INIT_LIST_HEAD(&pcb_free);

    // Add pcbFree_table elements to the list
    for (int i = 0; i < MAX_PROC; i++) {
        // TODO: check what happens when the element of the array is undefined
        list_add(&pcb_table[i].p_list, &pcb_free);
    }
}

void freePcb(pcb_t *p)
{
    // TODO: Check if the element p is already contained in the list
    // (I don't know if it supposed to be already checked or not, so I'll just
    // leave it like this)
    list_add(&p->p_list, &pcb_free);
}

state_t null_state()
{
    // TODO: Search on the documentation if
    // there are constants rappresenting these values
    state_t null_value;
    null_value.entry_hi = 0;
    null_value.cause = 0;
    null_value.status = UNINSTALLED;
    null_value.pc_epc = 0;
    null_value.hi = 0;
    null_value.lo = 0;
    for (int i = 0; i < STATE_GPR_LEN; i++) {
        null_value.gpr[i] = 0;
    }
    return null_value;
}

pcb_t *allocPcb()
{
    if (list_empty(&pcb_free)) {
        return NULL;
    } else {
        pcb_t *first = container_of(pcb_free.next, pcb_t, p_list);
        list_del(pcb_free.next);
        INIT_LIST_HEAD(&(first->p_list));
        INIT_LIST_HEAD(&(first->p_child));
        INIT_LIST_HEAD(&(first->p_sib));
        first->p_parent = NULL;
        first->p_s = null_state();
        first->p_time = 0;
        first->p_semAdd = NULL;
        return first;
    }
}

void mkEmptyProcQ(list_head *head) { INIT_LIST_HEAD(head); }

int emptyProcQ(list_head *head) { return list_empty(head); }

void insertProcQ(list_head *head, pcb_t *p)
{
    list_add_tail(&(p->p_list), head);
}

pcb_t *headProcQ(list_head *head)
{
    if (list_empty(head)) {
        return NULL;
    } else {
        return container_of(head->next, pcb_t, p_list);
    }
}

pcb_t *removeProcQ(list_head *head)
{

    // check if list is empty
    if (list_empty(head))
        return NULL;

    // get the first element of the list
    list_head *to_remove = list_next(head);

    // delete element from list
    list_del(to_remove);

    // return the pcb pointed by the deleted element
    return container_of(to_remove, pcb_t, p_list);
}

pcb_t *outProcQ(list_head *head, pcb_t *p)
{
    list_head *iter = (head)->next;

    // looking for p element
    for (; container_of(iter, pcb_t, p_list) != (p) && iter != (head);
         iter = iter->next)
        ;

    // completed a circle without finding p element
    if (iter == head) {
        return NULL;
    }

    // remove p element from list
    list_del(iter);

    return container_of(iter, pcb_t, p_list);
}

int emptyChild(pcb_t *p)
{
    if (p->p_child.next == NULL) {
        return TRUE;
    }

    return FALSE;
}

void insertChild(pcb_t *prnt, pcb_t *p)
{
    list_add(&p->p_sib, &(prnt->p_child));
}

pcb_t *removeChild(pcb_t *p)
{
    if (emptyChild(p))
        return NULL;
    pcb_t *ret = container_of(list_next(&(p->p_child)), pcb_t, p_sib);
    list_del(&ret->p_sib);
    // TODO: remove from parent's list of children
    return ret;
}

pcb_t *outChild(pcb_t *p)
{
    if (p->p_parent == NULL)
        return NULL;

    // get the first element of p_child inside p_parent of p
    list_head *iter = ((p->p_parent)->p_child).next;

    // assume that p exists in p_child of p->p_parent
    for (; container_of(iter, pcb_t, p_child) != (p); iter = iter->next)
        ;

    list_del(iter);

    return container_of(iter, pcb_t, p_child);
}
