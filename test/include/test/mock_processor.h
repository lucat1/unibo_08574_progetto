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

bool user_mode;
int halt_count;
int panic_count;
int wait_count;

bool is_user_mode() { return user_mode; }
void null_state(state_t *s)
{
    s->cause = 0;
    s->status = 0;
    s->pc_epc = 0;
    for (int i = 0; i < STATE_GPR_LEN; ++i)
        s->gpr[STATE_GPR_LEN] = 0;
}

void halt() { ++halt_count; }
void panic() { ++panic_count; }
void wait() { ++wait_count; }

void reset_processor()
{
    user_mode = true;
    halt_count = 0;
    panic_count = 0;
    wait_count = 0;
}

#endif /* PANDOS_TEST_MOCK_PROCESSOR_H */
