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
typedef unsigned int memaddr;
typedef unsigned int size_t;

/* process table entry type */
typedef struct pcb_t {
    /* process queue  */
    struct list_head p_list;

    /* process tree fields */
    struct pcb_t *p_parent;   /* ptr to parent	*/
    struct list_head p_child; /* children list */
    struct list_head p_sib;   /* sibling list  */

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
    struct list_head s_procq;

    /* Semaphore list */
    struct list_head s_link;
} semd_t;

#endif /* PANDOS_TYPES_H */
