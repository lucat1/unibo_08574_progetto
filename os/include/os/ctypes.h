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
#include <stdbool.h>
#include <stddef.h>
#else
typedef unsigned int memaddr;
typedef unsigned int size_t;
#define bool _Bool
#define true 1
#define false 0
#include <umps/const.h>
#endif

#endif /* PANDOS_OS_CTYPES_H */
