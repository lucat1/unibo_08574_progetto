/**
 * \file glob.c
 * \brief Global variables
 *
 * \author Luca Tagliavini
 * \date 17-03-2022
 */

#include "glob.h"
#include "os/list.h"

int alive_pcbs;
int blocked_pcbs;
list_head ready_pcbs;

void init_glob()
{
    alive_pcbs = 0;
    blocked_pcbs = 0;
    INIT_LIST_HEAD(&ready_pcbs);
}
