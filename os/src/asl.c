/** \file
 * \brief Implements \ref asl.h
 *
 * \author Alessandro Frau
 * \author Gianmaria Rovelli
 * \date 17-01-2022
 */

#include "os/asl.h"
#include "os/util.h"
#include "umps/types.h"

static semd_t semd_table[MAX_PROC];
static list_head semd_free;
static list_head semd_h;

#ifdef PANDOS_TESTING
semd_t *get_semd_table() { return semd_table; }
list_head *get_semd_free() { return &semd_free; }
void set_semd_free(const list_head new) { semd_free = new; }
list_head *get_semd_h() { return &semd_h; }
#endif

#ifndef PANDOS_TESTING
static inline
#endif
    semd_t *
    alloc_semd(int *sem_addr)
{
    semd_t *sem;

    if (!sem_addr || list_empty(&semd_free))
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
    if (!sem || !list_empty(&sem->s_procq))
        return true;

    list_del(&sem->s_link);
    list_add(&sem->s_link, &semd_free);
    return false;
}

#ifndef PANDOS_TESTING
static inline
#endif
    semd_t *
    find_semd(list_head *list, int *sem_addr)
{
    list_head *iter;
    semd_t *sem;

    if (!list || !sem_addr)
        return NULL;
    for (iter = list->next; iter != list; iter = iter->next) {
        if ((sem = container_of(iter, semd_t, s_link))->s_key == sem_addr)
            return sem;
    }

    return NULL;
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

    if (!sem_addr || !p || p->p_semAdd)
        return 1;

    /* Return an error when we run out of memory */
    if (!(sem = find_semd(&semd_h, sem_addr)) && !(sem = alloc_semd(sem_addr)))
        return 2;
    list_add_tail(&p->p_list, &sem->s_procq);
    p->p_semAdd = sem_addr;
    return 0;
}

pcb_t *out_blocked(pcb_t *pcb)
{
    semd_t *sem;
    if (!pcb || !(sem = find_semd(&semd_h, pcb->p_semAdd)) ||
        !list_contains(&pcb->p_list, &sem->s_procq))
        return NULL;

    list_del(&pcb->p_list);
    pcb->p_semAdd = NULL;
    if (list_empty(&sem->s_procq))
        free_semd(sem);
    return pcb;
}

pcb_t *head_blocked(int *sem_addr)
{
    semd_t *sem;
    if (!sem_addr || !(sem = find_semd(&semd_h, sem_addr)))
        return NULL;
    /* Assumes that a semd in semd_h contains at least one pcb */
    return container_of(sem->s_procq.next, pcb_t, p_list);
}

pcb_t *remove_blocked(int *sem_addr)
{
    return out_blocked(head_blocked(sem_addr));
}
