#ifndef PANDOS_SUPPORT_H
#define PANDOS_SUPPORT_H

#include "arch/processor.h"
extern void support_tlb();

extern void support_generic();

void support_trap();

void support_syscall(support_t *current_support);

void sys_read_terminal();

void sys_write_terminal();

void sys_write_printer();

void sys_get_tod();

void syscall_writer(void *termid, char *msg, size_t len);

#endif /* PANDOS_SUPPORT_H */
