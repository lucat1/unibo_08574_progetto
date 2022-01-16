#ifndef PANDOS_TYPES_H_INCLUDED
#define PANDOS_TYPES_H_INCLUDED

/****************************************************************************
 *
 * This header file contains utility types definitions.
 *
 ****************************************************************************/

#include <umps3/umps/types.h>
#include "pandos_const.h"
#include "listx.h"


typedef signed int   cpu_t;
typedef unsigned int memaddr;


/* process table entry type */
typedef struct pcb_t {
    /* process queue  */
    struct list_head p_list;

    /* process tree fields */
    struct pcb_t    *p_parent; /* ptr to parent	*/
    struct list_head p_child;  /* children list */
    struct list_head p_sib;    /* sibling list  */

    /* process status information */
    state_t p_s;    /* processor state */
    cpu_t   p_time; /* cpu time used by proc */

    /* Pointer to the semaphore the process is currently blocked on */
    int *p_semAdd;
} pcb_t, *pcb_PTR;


/* semaphore descriptor (SEMD) data structure */
typedef struct semd_t {
    /* Semaphore key */
    int *s_key;
    /* Queue of PCBs blocked on the semaphore */
    struct list_head s_procq;

    /* Semaphore list */
    struct list_head s_link;
} semd_t, *semd_PTR;

#endif
