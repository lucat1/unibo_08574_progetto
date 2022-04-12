/** \file
 * \brief Implements \ref asl.h
 *
 * \author Alessandro Frau
 * \author Gianmaria Rovelli
 * \date 17-01-2022
 */

#include "os/asl.h"
#include "arch/processor.h"
#include "os/asl_impl.h"
#include "os/util.h"

static semd_t semd_table[MAX_PROC];
static list_head semd_free;
static list_head semd_h;

#ifdef PANDOS_TESTING
semd_t *get_semd_table() { return semd_table; }
list_head *get_semd_free() { return &semd_free; }
list_head *get_semd_h() { return &semd_h; }
void set_semd_free(const list_head new) { semd_free = new; }
#endif

#ifndef PANDOS_TESTING
static inline
#endif
    semd_t *
    alloc_semd(int *sem_addr)
{
    semd_t *sem;

    if (sem_addr == NULL || list_empty(&semd_free))
        return NULL;

    sem = container_of(semd_free.next, semd_t, s_link);
    list_del(&sem->s_link);
    list_add(&sem->s_link, &semd_h);

    /* Fill the new structure */
    sem->s_key = sem_addr;
    INIT_LIST_HEAD(&sem->s_procq);
    return sem;
}

#ifndef PANDOS_TESTING
static inline
#endif
    bool
    free_semd(semd_t *sem)
{
    if (sem == NULL || !list_empty(&sem->s_procq))
        return true;

    list_del(&sem->s_link);
    list_add(&sem->s_link, &semd_free);
    return false;
}

static inline int key_cmp(const list_head *first, const list_head *second)
{
    return container_of(first, semd_t, s_link)->s_key -
           container_of(second, semd_t, s_link)->s_key;
}

#ifndef PANDOS_TESTING
static inline
#endif
    semd_t *
    find_semd(list_head *list, int *sem_addr)
{
    const list_head *item;
    semd_t dummy = {sem_addr};

    if (list == NULL || sem_addr == NULL ||
        (item = list_search(&dummy.s_link, list, key_cmp)) == NULL)
        return NULL;

    return container_of(item, semd_t, s_link);
}

void init_asl()
{
    size_t i;

    INIT_LIST_HEAD(&semd_free);
    INIT_LIST_HEAD(&semd_h);
    for (i = 0; i < MAX_PROC; ++i) {
        list_add(&semd_table[i].s_link, &semd_free);
    }
}

int insert_blocked(int *sem_addr, pcb_t *p)
{
    semd_t *sem;

    if (sem_addr == NULL)
        return 1;
    if (p == NULL)
        return 2;
    if (p->p_sem_add != NULL)
        return 3;

    /* Return an error when we run out of memory */
    if ((sem = find_semd(&semd_h, sem_addr)) == NULL &&
        (sem = alloc_semd(sem_addr)) == NULL)
        return 4;
    list_add_tail(&p->p_list, &sem->s_procq);
    p->p_sem_add = sem_addr;
    return 0;
}

pcb_t *out_blocked(pcb_t *pcb)
{
    semd_t *sem;
    if (pcb == NULL || (sem = find_semd(&semd_h, pcb->p_sem_add)) == NULL ||
        !list_contains(&pcb->p_list, &sem->s_procq))
        return NULL;

    list_sdel(&pcb->p_list);
    pcb->p_sem_add = NULL;
    if (list_empty(&sem->s_procq))
        free_semd(sem);
    return pcb;
}

pcb_t *head_blocked(int *sem_addr)
{
    semd_t *sem;
    if (sem_addr == NULL || (sem = find_semd(&semd_h, sem_addr)) == NULL)
        return NULL;
    /* Assumes that a semd in semd_h contains at least one pcb */
    return container_of(sem->s_procq.next, pcb_t, p_list);
}

pcb_t *remove_blocked(int *sem_addr)
{
    return out_blocked(head_blocked(sem_addr));
}
