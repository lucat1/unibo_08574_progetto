/**
 * \file mock_iodev.h
 * \brief A mock of the `get_iodev` procedure for tests that don't care about
 * its behaviour.
 *
 * \author Luca Tagliavini
 * \date 10-04-2022
 */

#ifndef PANDOS_TEST_MOCK_IODEV_H
#define PANDOS_TEST_MOCK_IODEV_H

#include "arch/devices.h"
#include "os/semaphores.h"

iodev_t get_iodev(size_t *cmd_addr)
{
    iodev_t res = {get_semaphore(IL_TERMINAL, 0, true), 0};
    return res;
}

#endif /* PANDOS_TEST_MOCK_IODEV_H */
