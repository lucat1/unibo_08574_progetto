/**
 * \file mock_processor.h
 * \brief A mock of minimal firmware utilies for testing purpuses.
 *
 * \author Luca Tagliavini
 * \date 9-04-2022
 */

#ifndef PANDOS_TEST_MOCK_PROCESSOR_H
#define PANDOS_TEST_MOCK_PROCESSOR_H

#include "arch/processor.h"
#include "os/util.h"

#define MOCK_INTERRUPTS_ON_NUCLEUS 0x200   /* 10-th bit is on */
#define MOCK_INTERRUPTS_ON_PROCESS 0x400   /* 11-th bit is on */
#define MOCK_INTERRUPTS_LOCAL_TIMER 0x800  /* 12-th bit is on */
#define MOCK_KERNEL_MODE_ON_NUCLEUS 0x1000 /* 13-th bit is on */
#define MOCK_KERNEL_MODE_ON_PROCESS 0x2000 /* 14-th bit is on */
#define MOCK_RESERVED_INSTRUCTION 0x2000   /* 14-th bit is on */

bool user_mode;
int halt_count;
int panic_count;
int wait_count;
state_t processor_state;

bool is_user_mode() { return user_mode; }
void null_state(state_t *s)
{
    s->cause = 0;
    s->status = 0;
    s->pc_epc = 0;
    for (int i = 0; i < STATE_GPR_LEN; ++i)
        s->gpr[i] = 0;
}
void load_state(state_t *s)
{
    pandos_memcpy(&processor_state, s, sizeof(state_t));
}
void load_context(context_t *ctx)
{
    processor_state.reg_sp = ctx->pc;
    processor_state.status = ctx->status;
    processor_state.pc_epc = processor_state.reg_t9 = ctx->pc;
}
void store_state(state_t *s)
{
    pandos_memcpy(s, &processor_state, sizeof(state_t));
}

void halt() { ++halt_count; }
void panic() { ++panic_count; }
void wait() { ++wait_count; }

void set_status(size_t status) { processor_state.status = status; }
size_t get_status() { return processor_state.status; }
size_t get_cause() { return processor_state.cause; }
void status_interrupts_on_nucleus(size_t *prev)
{
    *prev |= MOCK_INTERRUPTS_ON_NUCLEUS;
}
void status_interrupts_on_process(size_t *prev)
{
    *prev |= MOCK_INTERRUPTS_ON_PROCESS;
}
void status_toggle_local_timer(size_t *prev)
{
    *prev ^= MOCK_INTERRUPTS_LOCAL_TIMER;
}
void status_kernel_mode_on_nucleus(size_t *prev)
{
    *prev |= MOCK_KERNEL_MODE_ON_NUCLEUS;
}
void status_kernel_mode_on_process(size_t *prev)
{
    *prev |= MOCK_KERNEL_MODE_ON_PROCESS;
}
void status_reserved_instruction(size_t *prev)
{
    *prev |= MOCK_RESERVED_INSTRUCTION;
}

void reset_processor()
{
    user_mode = false;
    halt_count = 0;
    panic_count = 0;
    wait_count = 0;
    null_state(&processor_state);
}

#endif /* PANDOS_TEST_MOCK_PROCESSOR_H */
