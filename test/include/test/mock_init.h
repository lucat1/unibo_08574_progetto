/**
 * \file mock_init.h
 * \brief Mock the nucleus init sequence to instantiate common structures needed
 * for tests.
 *
 * \author Luca Tagliavini
 * \date 10-04-2022
 */

#ifndef PANDOS_TEST_MOCK_INIT_H
#define PANDOS_TEST_MOCK_INIT_H

#include "os/asl.h"
#include "os/pcb.h"
#include "os/scheduler.h"
#include "os/scheduler_impl.h"
#include "os/semaphores.h"
#include "test/mock_devices.h"
#include "test/mock_processor.h"
#include "test/mock_semaphores.h"

void mock_init()
{
    init_scheduler();
    init_semaphores();
    init_pcbs();
    init_asl();
    reset_devices();
    reset_processor();
}

#endif /* PANDOS_TEST_MOCK_INIT_H */
