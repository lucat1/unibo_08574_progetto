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
#include "test/mock_semaphores.h"

#define MOCK_WRONG_CMD_ADDR 0x0F0F0F0F
iodev_t ok = {&semaphores[0], 0};
iodev_t err = {NULL, 0};

iodev_t get_iodev(size_t *cmd_addr)
{
    if (cmd_addr == (size_t *)MOCK_WRONG_CMD_ADDR)
        return err;

    return ok;
}

#endif /* PANDOS_TEST_MOCK_IODEV_H */
