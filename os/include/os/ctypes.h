/**
 * \file processor.h
 * \brief Architecture-dependant functions to interact with the cpu and its
 * state.
 *
 * \author Luca Tagliavini
 * \date 9-04-2022
 */

#ifndef PANDOS_OS_CTYPES_H
#define PANDOS_OS_CTYPES_H

#ifdef __x86_64__
typedef unsigned long memaddr;
/* using stdbool.h would be nice here but casts to these types are automatically
 * mapped in the {0,1} set, and this wouldn't match the behaviour on umps3 */
typedef unsigned int bool;
#define true 1
#define false 0
#include <stddef.h>
#else
typedef unsigned int memaddr;
typedef unsigned int size_t;
typedef unsigned int bool;
#define true 1
#define false 0
#include <umps/const.h>
#endif

#endif /* PANDOS_OS_CTYPES_H */
