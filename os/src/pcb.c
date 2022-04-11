/**
 * \file pcb.c
 * \brief Implementation \ref pcb.h
 *
 * \author Alessandro Frau
 * \author Gianmaria Rovelli
 * \date 17-01-2022
 */

#include "os/pcb.h"
#include "arch/processor.h"
#include "os/list.h"
#include "os/types.h"
#include "os/util.h"

static pcb_t pcb_table[MAX_PROC];
static list_head pcb_free;

#ifdef PANDOS_TESTING
const list_head *get_pcb_free() { return &pcb_free; }
#endif
const pcb_t *get_pcb_table() { return pcb_table; }

void init_pcbs()
{
    /* Initialize the list */
    INIT_LIST_HEAD(&pcb_free);

    /* Add pcb_table elements to the list */
    for (int i = 0; i < MAX_PROC; i++) {
        list_add(&pcb_table[i].p_list, &pcb_free);
        pcb_table[i].p_pid = -1;
    }
}

void free_pcb(pcb_t *p)
{
    if (p == NULL || list_contains(&p->p_list, &pcb_free))
        return;

    p->p_pid = -1;
  list_del(&p->p_list);
    list_add(&p->p_list, &pcb_free);
}

static inline pcb_t *null_pcb(pcb_t *t)
{
    if (t == NULL)
        return NULL;
    INIT_LIST_HEAD(&t->p_list);
    INIT_LIST_HEAD(&t->p_child);
    INIT_LIST_HEAD(&t->p_sib);
    t->p_parent = NULL;
    t->p_time = 0;
    t->p_sem_add = NULL;
    null_state(&t->p_s);
    /* New fields (phase2) */
    t->p_support = NULL;
    t->p_prio = 0;
    t->p_pid = -1;
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

void mk_empty_proc_q(list_head *head)
{
    if (head == NULL)
        return;
    INIT_LIST_HEAD(head);
}

int empty_proc_q(list_head *head)
{
    if (head == NULL)
        return true;
    return list_empty(head);
}

void insert_proc_q(list_head *head, pcb_t *p)
{
    if (p == NULL || head == NULL || list_contains(&p->p_list, head))
        return;
    list_del(&p->p_list);
    list_add_tail(&p->p_list, head);
}

pcb_t *head_proc_q(list_head *head)
{
    if (head == NULL || list_empty(head)) {
        return NULL;
    } else {
        return container_of(list_next(head), pcb_t, p_list);
    }
}

pcb_t *remove_proc_q(list_head *head)
{
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
    if (head == NULL || p == NULL || list_empty(head) ||
        !list_contains(&p->p_list, head))
        return NULL;

    /* Remove p element from list */
    list_del(&p->p_list);

    return p;
}

int empty_child(pcb_t *p)
{
    if (p == NULL)
        return true;
    return list_empty(&p->p_child);
}

void insert_child(pcb_t *prnt, pcb_t *p)
{
    /* Note: `list_contains` is a costly pedantic check on the input data. It
     * can be safely removed if performance is a concern. */
    if (prnt == NULL || p == NULL || p->p_parent != NULL ||
        list_contains(&p->p_sib, &prnt->p_child))
        return;

    /* Set new parent */
    p->p_parent = prnt;
    /* Add `p` to children list of `prnt` */
    list_add_tail(&p->p_sib, &prnt->p_child);
}

pcb_t *remove_child(pcb_t *p)
{
    if (p == NULL || list_empty(&p->p_child))
        return NULL;
    return out_child(container_of(list_next(&p->p_child), pcb_t, p_sib));
}

pcb_t *out_child(pcb_t *p)
{
    /* Note: list_contains could be removed if performance is required */
    if (p == NULL || p->p_parent == NULL || list_empty(&p->p_parent->p_child) ||
        !list_contains(&p->p_sib, &p->p_parent->p_child))
        return NULL;

    list_del(&p->p_sib);
    p->p_parent = NULL;
    return p;
}
