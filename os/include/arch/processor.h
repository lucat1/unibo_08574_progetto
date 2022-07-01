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

#include "os/const.h"
#include "os/ctypes.h"

#ifdef __x86_64__
#define STATE_GPR_LEN 7
/** Mock a generic processor state with the bare mimum register for the nucleus
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
#define reg_t9 gpr[5]
#define reg_sp gpr[6]
#else
/** Use the architecture-provided state_t */
#include <umps/types.h>
#endif

typedef signed int cpu_t;

/** Page Table Entry descriptor */
typedef struct pte_entry_t {
    unsigned int pte_entry_hi;
    unsigned int pte_entry_lo;
} pte_entry_t;

/** Support level context */
typedef struct context_t {
    unsigned int stack_ptr;
    unsigned int status;
    unsigned int pc;
} context_t;

/** Support level descriptor */
typedef struct support_t {
    int sup_asid;                    /* process ID */
    state_t sup_except_state[2];     /* old state exceptions */
    context_t sup_except_context[2]; /* new contexts for passing up	*/
    pte_entry_t sup_private_page_table[USERPGTBLSIZE]; /* user page table */
} support_t;

extern void init_puv(memaddr tlb_refill_handler, memaddr exception_handler);
extern bool is_user_mode();
extern void null_state(state_t *s);
extern void load_state(state_t *s);
extern void load_context(context_t *ctx);
extern void store_state(state_t *s);

extern void halt();
extern void panic();
extern void wait();

extern void set_status(size_t status);
extern size_t get_status();
extern size_t get_cause();
extern void status_interrupts_on_nucleus(size_t *prev);
extern void status_interrupts_off_nucleus(size_t *prev);
extern void status_interrupts_on_process(size_t *prev);
extern void status_local_timer_toggle(size_t *prev);
extern void status_local_timer_on(size_t *prev);
extern void status_kernel_mode_on_nucleus(size_t *prev);
extern void status_kernel_mode_on_process(size_t *prev);
extern void status_kernel_mode_off_process(size_t *prev);

extern void cause_clean(size_t *prev);
extern void cause_reserved_instruction(size_t *prev);

#endif /* PANDOS_ARCH_PROCESSOR_H */
