/**
 * \file processor.h
 * \brief Architecture-dependant functions to interact with the cpu and its
 * state.
 *
 * \author Luca Tagliavini
 * \date 9-04-2022
 */

#ifndef PANDOS_ARCH_PROCESSOR_H
#define PANDOS_ARCH_PROCESSOR_H

#include "os/ctypes.h"

#ifdef __x86_64__
#define STATE_GPR_LEN 5
/* Mock a generic processor state with the bare mimum register for the nucleus
 */
typedef struct state {
    size_t cause;
    size_t status;
    size_t pc_epc;
    size_t gpr[STATE_GPR_LEN];
} state_t;
#define reg_v0 gpr[0]
#define reg_a0 gpr[1]
#define reg_a1 gpr[2]
#define reg_a2 gpr[3]
#define reg_a3 gpr[4]
#else
/* Use the architecture-provided state_t */
#include <umps/types.h>
#endif

extern bool is_user_mode();
extern void null_state(state_t *s);

extern void halt();
extern void panic();
extern void wait();

#endif /* PANDOS_ARCH_PROCESSOR_H */
