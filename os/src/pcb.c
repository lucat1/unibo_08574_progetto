/**
 * \file pcb.c
 * \brief Implementation \ref pcb.h
 *
 * \author Alessandro Frau
 * \author Gianmaria Rovelli
 * \date 17-01-2022
 *
 */

#include "os/pcb.h"
#include "os/list.h"
#include "os/types.h"
#include "os/util.h"

static pcb_t pcb_table[MAX_PROC];
static list_head pcb_free;

#ifdef PANDOS_TESTING
pcb_t *get_pcb_table() { return pcb_table; }
list_head *get_pcb_free() { return &pcb_free; }
#endif

/* This function should be called only once during the initialization phase */
void init_pcbs()
{
    /* Initialize the list */
    INIT_LIST_HEAD(&pcb_free);

    /* Add pcb_table elements to the list */
    for (int i = 0; i < MAX_PROC; i++) {
        list_add(&pcb_table[i].p_list, &pcb_free);
    }
}

bool pcb_free_contains(pcb_t *p)
{
    /* prevent SEGFAULT */
    if(p == NULL || &(p->p_list) == NULL)
    {
        return false;
    }
    return list_contains(&(p->p_list), &pcb_free);
}

void free_pcb(pcb_t *p)
{
    /* prevent SEGFAULT */
    if(p == NULL || &p->p_list == NULL || list_contains(&p->p_list, &pcb_free)){
        return;    
    }
    list_add(&p->p_list, &pcb_free);
}

pcb_t *null_pcb(pcb_t *t)
{
    /* prevent SEGFAULT */
    if (t == NULL) 
        return NULL;
    INIT_LIST_HEAD(&(t->p_list));
    INIT_LIST_HEAD(&(t->p_child));
    INIT_LIST_HEAD(&(t->p_sib));
    t->p_parent = NULL;
    t->p_time = 0;
    t->p_semAdd = NULL;
    t->p_s.entry_hi = 0;
    t->p_s.cause = 0;
    t->p_s.status = UNINSTALLED;
    t->p_s.pc_epc = 0;
    t->p_s.hi = 0;
    t->p_s.lo = 0;
    for (int i = 0; i < STATE_GPR_LEN; i++) {
        t->p_s.gpr[i] = 0;
    }
    return t;
}

pcb_t *alloc_pcb()
{
    if (list_empty(&pcb_free)) {
        return NULL;
    } else {
        pcb_t *first = container_of(pcb_free.next, pcb_t, p_list);
        list_del(pcb_free.next);
        return null_pcb(first);
    }
}

void mk_empty_proc_q(list_head *head) { if(head == NULL) return; INIT_LIST_HEAD(head); }

int empty_proc_q(list_head *head) { if(head == NULL) return true; return list_empty(head); }

void insert_proc_q(list_head *head, pcb_t *p)
{
    /* prevent SEGFAULT */
    if(p == NULL || head == NULL || &p->p_list == NULL) 
        return;
    if(list_contains(&p->p_list, head)) 
        return;
    list_add_tail(&(p->p_list), head);
}

pcb_t *head_proc_q(list_head *head)
{   
    /* prevent SEGFAULT */
    if (head == NULL || list_empty(head)) {
        return NULL;
    } else {
        return container_of(list_next(head), pcb_t, p_list);
    }
}

pcb_t *remove_proc_q(list_head *head)
{

    /* prevent SEGFAULT */
    if (head == NULL || list_empty(head))
        return NULL;

    /* get the first element of the list */
    list_head *to_remove = list_next(head);
    pcb_t *p = container_of(to_remove, pcb_t, p_list);

    /* return the pcb pointed by the deleted element */
    return out_proc_q(head, p);
}

pcb_t *out_proc_q(list_head *head, pcb_t *p)
{
    /* prevent SEGFAULT */
    if(head == NULL || p == NULL || list_empty(head)) 
        return NULL;

    list_head *iter = list_next(head);

    /* looking for p element */
    for (; container_of(iter, pcb_t, p_list) != (p) && iter != (head);
         iter = list_next(iter))
        ;

    /* completed a circle without finding p element */
    if (iter == head) {
        return NULL;
    }

    /* remove p element from list */
    list_del(iter);

    return container_of(iter, pcb_t, p_list);
}

int empty_child(pcb_t *p) { 
    /* prevent SEGFAULT */
    if(p == NULL || &(p->p_child) == NULL) 
        return true; 
    return list_empty(&(p->p_child)); 
}

void insert_child(pcb_t *prnt, pcb_t *p)
{
    /* prevent SEGFAULT */
    if(prnt == NULL || p == NULL) return;
    if(list_contains(&(p->p_list), &(prnt->p_child))) return;
    
    /* set new parent */
    p->p_parent = prnt;
    pcb_t *first_child = container_of(&((prnt->p_child)), pcb_t, p_child);
    
    /* add p to children list of prnt */
    list_add_tail(&(p->p_list), &(prnt->p_child));
    /* add p to the list of siblings */
    if (first_child != NULL) {
        /* if p is already in p_sib stop execution */
        if(list_contains(&(p->p_sib), &(first_child->p_sib))) 
            return;
        list_add_tail(&(p->p_sib), &(first_child->p_sib));
    }
}

pcb_t *remove_child(pcb_t *p)
{
    /* can't remove if p has no children */
    if (empty_child(p))
        return NULL;

    list_head *first_child_head = list_next(&(p->p_child));

    /* if was set container_of with p_child, this would have returned "p" */
    pcb_t *ret = container_of(first_child_head, pcb_t, p_list);

    /* list_next because for first one is useless */
    list_del(first_child_head);
    /* remove p from siblings list */
    list_del((&(ret->p_sib)));
    /* reset my parent */
    ret->p_parent = NULL;
    /* clear ret siblings list */
    INIT_LIST_HEAD(&(ret->p_sib));

    return ret;
}

pcb_t *out_child(pcb_t *p)
{
    /* prevent SEGFAULT */
    if (p->p_parent == NULL)
        return NULL;
    if(list_empty(&(p->p_parent)->p_child)) 
        return NULL;
    /* verifying that p is in p_child of his p_parent */
    if(!list_contains(&(p->p_list), &(p->p_parent)->p_child))
        return NULL;

    list_head *iter = list_next(&((p->p_parent)->p_child));

    /* looking for p element in p_child */
    for (; container_of(iter, pcb_t, p_list) != (p); iter = list_next(iter))
        ;

    pcb_t *ret = container_of(iter, pcb_t, p_list);

    /* list_next because for first one is useless */
    list_del(iter);
    /* remove p from siblings list */
    list_del((&(ret->p_sib)));
    /* reset my parent */
    ret->p_parent = NULL;
    /* clear ret siblings list */
    INIT_LIST_HEAD(&(ret->p_sib));

    return ret;
}
