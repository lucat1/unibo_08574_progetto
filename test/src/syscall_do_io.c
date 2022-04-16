/**
 * \file syscall_do_io.c
 * \brief Tests around the DOIO system call.
 *
 * \author Luca Tagliavini
 * \date 13-04-2022
 */

#include "test/mock_init.h"
#include "test/mock_iodev.h"
#include "test/mock_syscall.h"
#include "test/test.h"
#include <stdlib.h>
#include <time.h>

int main()
{
    size_t dest;
    mock_init();
    srand(time(NULL));

    ensure("destination \"device\" doesn't have the same address as "
           "MOCK_WRONG_CMD_ADDR")
    {
        assert(&dest != (size_t *)MOCK_WRONG_CMD_ADDR);
    }
    ensure("do_io sanitizes input parameters")
    {
        active_process = spawn_process(false);
        assert(active_process->p_pid != NULL_PID);
        SYSCALL(DOIO, (size_t)NULL, 1, 0);
        assert(active_process->p_pid == NULL_PID);

        active_process = spawn_process(false);
        assert(active_process->p_pid != NULL_PID);
        SYSCALL(DOIO, (size_t)MOCK_WRONG_CMD_ADDR, 1, 0);
        assert(active_process->p_pid == NULL_PID);
    }

    ensure("do_io writes data to the device")
    {
        size_t value = rand();
        active_process = spawn_process(false);
        assert(active_process->p_pid != NULL_PID);
        SYSCALL(DOIO, (size_t)&dest, value, 0);
        assert(active_process->p_pid != NULL_PID);
        assert(dest == value);
        assert(head_blocked(get_iodev(&dest).semaphore) == active_process);

        assert(!kill_progeny(active_process));
    }

    ensure("do_io enables the appropriate interrupt lines")
    {
        size_t status;

        /* Ensure all interrupts are enabled on low priority processes */
        active_process = spawn_process(false);
        status = active_process->p_s.status;
        assert(active_process->p_pid != NULL_PID);
        SYSCALL(DOIO, (size_t)&dest, 1, 0);
        assert(active_process->p_pid != NULL_PID);
        status_il_on_all(&status);
        assert(active_process->p_s.status == status);
        assert(!kill_progeny(active_process));

        /* Same check with a high priority process */
        active_process = spawn_process(true);
        status = active_process->p_s.status;
        assert(active_process->p_pid != NULL_PID);
        SYSCALL(DOIO, (size_t)&dest, 1, 0);
        assert(active_process->p_pid != NULL_PID);
        status_il_on(&status, get_iodev(&dest).interrupt_line);
        assert(active_process->p_s.status == status);
        assert(!kill_progeny(active_process));
    }

    ensure("do_io fails on a busy device")
    {
        pcb_t *p1;
        size_t value;

        value = rand();
        p1 = active_process = spawn_process(false);
        assert(active_process->p_pid != NULL_PID);
        SYSCALL(DOIO, (size_t)&dest, value, 0);
        assert(active_process->p_pid != NULL_PID);
        assert(dest == value);

        value = rand();
        active_process = spawn_process(false);
        assert(active_process->p_pid != NULL_PID);
        assert(head_blocked(get_iodev(&dest).semaphore) == p1);
        SYSCALL(DOIO, (size_t)&dest, value, 0);
        assert(active_process->p_pid == NULL_PID);
        assert(p1->p_pid != NULL_PID);
        assert(dest != value);

        assert(!kill_progeny(p1));
    }

    return 0;
}
