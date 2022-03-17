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

void init()
{
    pandos_printf(":: init_pcbs & init_asl\n");
    init_pcbs();
    init_asl();
    pandos_printf(":: init_glob\n");
    init_glob();
}
