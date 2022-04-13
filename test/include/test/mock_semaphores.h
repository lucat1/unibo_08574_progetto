/**
 * \file mock_semaphores.h
 * \brief A mock of the architecture-specific functions to interact with
 * semaphores for devices.
 *
 * \author Luca Tagliavini
 * \date 13-04-2022
 */

#ifndef PANDOS_TEST_MOCK_SEMAPHORES_H
#define PANDOS_TEST_MOCK_SEMAPHORES_H

#include "os/semaphores.h"

#define MOCK_SEMAPHORES_LEN 5

int semaphores[MOCK_SEMAPHORES_LEN];

void init_semaphores()
{
    for (size_t i = 0; i < MOCK_SEMAPHORES_LEN; ++i)
        semaphores[i] = 0;
}

int *get_semaphore(int int_l, int dev_n, bool is_w)
{
    return semaphores + int_l + dev_n + (int)is_w;
}

int *get_timer_semaphore() { return semaphores + MOCK_SEMAPHORES_LEN - 1; }

#endif /* PANDOS_TEST_MOCK_SEMAPHORES_H */
