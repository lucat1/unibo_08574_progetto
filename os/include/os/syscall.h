/**
 * \file syscall.h
 * \brief Syscalls.
 *
 * \author Alessandro Frau
 * \author Gianmaria Rovelli
 * \author Luca Tagliavini
 * \date 26-03-2022
 */

#ifndef PANDOS_SYSCALL_H
#define PANDOS_SYSCALL_H

#include "os/scheduler.h"

extern scheduler_control_t syscall_handler();

#endif /* PANDOS_SYSCALL_H */
