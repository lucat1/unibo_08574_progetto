/** \file
 * \brief Type definitions
 *
 * \author Luca Tagliavini
 * \date 17-01-2022
 */

#ifndef PANDOS_TYPES_H
#define PANDOS_TYPES_H

#include "const.h"
#include "list.h"
#include <umps/types.h>

typedef signed int cpu_t;
/* Avoid re-defining size_t on a modern architecture */
#ifdef __x86_64__
typedef unsigned long memaddr;
#include <stdbool.h>
#include <stddef.h>
#else
typedef unsigned int memaddr;
typedef unsigned int size_t;
#define bool _Bool
#define true 1
#define false 0
#endif

/* process table entry type */
typedef struct pcb_t {
    /* process queue  */
    list_head p_list;

    /* process tree fields */
    struct pcb_t *p_parent; /* ptr to parent	*/
    list_head p_child;      /* children list */
    list_head p_sib;        /* sibling list  */

    /* process status information */
    state_t p_s;  /* processor state */
    cpu_t p_time; /* cpu time used by proc */

    /* Pointer to the semaphore the process is currently blocked on */
    int *p_semAdd;
} pcb_t;

/* semaphore descriptor (SEMD) data structure */
typedef struct semd_t {
    /* Semaphore key */
    int *s_key;
    /* Queue of PCBs blocked on the semaphore */
    list_head s_procq;

    /* Semaphore list */
    list_head s_link;
} semd_t;

#endif /* PANDOS_TYPES_H */
