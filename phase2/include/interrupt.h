/**
 * \file init.h
 * \brief Implementation of internal init routines.
 *
 * \author Luca Tagliavini
 * \date 20-03-2022
 *
 */
#ifndef PANDOS_INTERRUPT_H
#define PANDOS_INTERRUPT_H

void exception_handler();

void syscall_create_process();
void syscall_verhogen();
void syscall_passeren();
void syscall_do_io();

#endif /* PANDOS_INTERRUPT_H */
