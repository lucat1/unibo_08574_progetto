/**
 * \file glob.c
 * \brief Global variables
 *
 * \author Luca Tagliavini
 * \date 17-03-2022
 */

#ifndef PANDOS_GLOB_H
#define PANDOS_GLOB_H

#include "os/list.h"

extern int alive_pcbs;
extern int blocked_pcbs;
extern list_head ready_pcbs;

void init_glob();

#endif /* PANDOS_GLOB_H */
