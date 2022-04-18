/**
 * \file syscall_process_lifecycle.c
 * \brief Tests around the CREATEPROCESS and TERMPROCESS system call.
 *
 * \author Alessandro Frau
 * \author Luca Tagliavini
 * \date 12-04-2022
 */

#include "test/mock_init.h"
#include "test/mock_iodev.h"
#include "test/mock_syscall.h"
#include "test/test.h"
#include <stdlib.h>
#include <time.h>

int main()
{
    state_t p_s;
    pcb_t *new_process;

    srand(time(NULL));
    mock_init();
    null_state(&p_s);

    ensure("create_process sanitizes the input")
    {
        active_process = spawn_process(false);
        SYSCALL(CREATEPROCESS, (size_t)&p_s, 2, 0);
        assert(active_process->p_pid == NULL_PID);

        active_process = spawn_process(false);
        SYSCALL(CREATEPROCESS, (size_t)NULL, true, 0);
        assert(active_process->p_pid == NULL_PID);
    }
    ensure("terminate_process does not break if the input is broken")
    {
        pandos_pid_t pid;
        while ((pid = rand()) == active_process->p_pid)
            ;

        active_process = spawn_process(false);
        SYSCALL(TERMPROCESS, pid, 0, 0);
        assert(active_process->p_pid == NULL_PID);
    }
    active_process = NULL;

    it("creates and kills a new high/low priority process")
    {
        for (bool i = 0; i < 2; ++i) {
            active_process = spawn_process(i);
            assert(process_count == 1);
            assert(active_process->p_pid != NULL_PID);
            SYSCALL(CREATEPROCESS, (size_t)&p_s, i, 0);
            assert(!panic_count);
            assert(process_count == 2);
            assert(list_size(i ? &ready_queue_hi : &ready_queue_lo) == 2);
            assert(active_process->p_s.reg_v0 != NULL_PID);
            assert((new_process = find_process(
                        (pandos_pid_t)active_process->p_s.reg_v0)) != NULL);
            assert((pandos_pid_t)active_process->p_s.reg_v0 ==
                   new_process->p_pid);
            assert(new_process->p_prio == i);
            assert(list_size(&active_process->p_child) == 1);
            assert(container_of(list_next(&active_process->p_child), pcb_t,
                                p_sib) == new_process);
            SYSCALL(TERMPROCESS, new_process->p_pid, 0, 0);
            assert(!panic_count);
            assert(process_count == 1);
            assert(list_size(i ? &ready_queue_hi : &ready_queue_lo) == 1);
            assert(new_process->p_pid == NULL_PID);
            assert(list_empty(&active_process->p_child));
            assert(head_proc_q(i ? &ready_queue_hi : &ready_queue_lo) ==
                   active_process);
            kill_progeny(active_process);
        }
        active_process = NULL;
    }
    it("correctly runs out of memory and returns NULL_PID")
    {
        active_process = spawn_process(false);
        /* There is already one process allocated to call the NSYS, so we need
         * MAX_PROC-1 more processes to run out of memory. */
        for (size_t i = 0; i < MAX_PROC - 1; ++i) {
            SYSCALL(CREATEPROCESS, (size_t)&p_s, false, 0);
            assert(active_process->p_s.reg_v0 != NULL_PID);
            assert(list_size(&active_process->p_child) == i + 1);
            assert(process_count == i + 2);
        }
        assert(process_count == 20);
        SYSCALL(CREATEPROCESS, (size_t)&p_s, false, 0);
        assert(process_count == 20);
        assert(list_size(&active_process->p_child) == 19);
        assert(active_process->p_s.reg_v0 == NULL_PID);
        kill_progeny(active_process);
        active_process = NULL;
    }
    it("correctly terminates the active_process")
    {
        active_process = spawn_process(false);
        SYSCALL(TERMPROCESS, 0, 0, 0);
        assert(active_process->p_pid == NULL_PID);
        assert(list_empty(&ready_queue_hi));
        assert(list_empty(&ready_queue_lo));
        assert(softblock_count == 0);
        assert(process_count == 0);
    }
    return 0;
}
