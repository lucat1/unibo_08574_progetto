/**
 * \file init.c
 * \brief Implementation \ref init.h
 *
 * \author Luca Tagliavini
 * \date 17-03-2022
 */

#include "init.h"
#include "glob.h"
#include "os/asl.h"
#include "os/pcb.h"
#include "os/util.h"

/* Initialize the table refill handlers to the given procedures until the
 * support level is implemented.
 */
static inline void init_tbl()
{ /* TODO */
}

inline void init()
{
    pandos_printf(":: init_tbl\n");
    init_tbl();
    pandos_printf(":: init_glob\n");
    init_glob();
    pandos_printf(":: init_pcbs & init_asl\n");
    init_pcbs();
    init_asl();
    /* TODO: read from 3.1.5 */
}
