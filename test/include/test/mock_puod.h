/**
 * \file mock_syscall.h
 * \brief A mock of the BIOS SYSCALL procedure.
 *
 * \author Luca Tagliavini
 * \date 9-04-2022
 */

#ifndef PANDOS_TEST_MOCK_PUOD_H
#define PANDOS_TEST_MOCK_PUOD_H

#include "os/exception_impl.h"

int puod_count;

scheduler_control_t pass_up_or_die(memaddr cause)
{
    ++puod_count;
    return CONTROL_BLOCK;
}

void reset_puod() { puod_count = 0; }

#endif /* PANDOS_TEST_MOCK_PUOD_H */
