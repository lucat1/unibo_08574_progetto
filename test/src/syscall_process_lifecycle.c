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

int main()
{
    state_t p_s;
    pcb_t *new_process;

    mock_init();
    null_state(&p_s);
    active_process = spawn_process(false);
    int pids[19];

    it("creates a new high priority process")
    {
        assert(active_process->p_pid != NULL_PID);
        SYSCALL(CREATEPROCESS, (size_t)&p_s, true, 0);
        assert(!panic_count);
        assert(active_process->p_s.reg_v0 != NULL_PID);
        assert(process_count == 2);
        assert((new_process = find_process(
                    (pandos_pid_t)active_process->p_s.reg_v0)) != NULL);
        assert((pandos_pid_t)active_process->p_s.reg_v0 == new_process->p_pid);
        assert(new_process->p_prio == true);
        printf("active_process: %d\n",
               head_proc_q(&active_process->p_child)->p_pid);
        printf("new_process: %d\n", &new_process->p_list);
        list_print(&active_process->p_child);
        assert(list_size(&active_process->p_child) == 1);
        assert(head_proc_q(&active_process->p_child)->p_pid ==
               new_process->p_pid);
        assert(!list_empty(&ready_queue_hi));
    }
    it("properly terminates the newly created process")
    {
        SYSCALL(TERMPROCESS, new_process->p_pid, 0, 0);
        assert(!panic_count);
        assert(process_count == 1);
        assert(new_process->p_pid == NULL_PID);
        assert(list_empty(&active_process->p_child));
        assert(list_empty(&ready_queue_hi));
    }
    it("correctly creates a new low priority process")
    {
        assert(active_process->p_pid != NULL_PID);
        SYSCALL(CREATEPROCESS, (size_t)&p_s, true, 0);
        assert(!panic_count);
        assert(active_process->p_s.reg_v0 != NULL_PID);
        assert(process_count == 2);
        assert((new_process = find_process(
                    (pandos_pid_t)active_process->p_s.reg_v0)) != NULL);
        assert((pandos_pid_t)active_process->p_s.reg_v0 == new_process->p_pid);
        assert(head_proc_q(&active_process->p_child) == new_process);
        assert(!list_empty(&ready_queue_lo));
    }
    it("correctly terminates the newly created process")
    {
        SYSCALL(TERMPROCESS, new_process->p_pid, 0, 0);
        assert(!panic_count);
        assert(process_count == 1);
        assert(new_process->p_pid == NULL_PID);
        assert(list_empty(&active_process->p_child));
        assert(list_size(&ready_queue_hi) == 1);
    }
    it("correctly gets out of memory and returns NULL_PID")
    {
        /* There is already 1 process allocated from previous code, so we need
         * To add at least 19 more processes to get out of memory
         */
        for (int i = 0; i < 19; i++) {
            SYSCALL(CREATEPROCESS, (size_t)rand(), true, 0);
            assert(list_size(&active_process->p_child) == i + 1);
            assert(process_count == i + 2);
            pids[i] = active_process->p_s.reg_v0;
        }
        assert(active_process->p_s.reg_v0 != NULL_PID);
        SYSCALL(CREATEPROCESS, (size_t)rand(), true, 0);
        assert(list_size(&active_process->p_child) == 19);
        assert(process_count == 20);
        assert(active_process->p_s.reg_v0 == NULL_PID);
    }
    ensure("create_process does not break if the input is broken")
    {
        /* Remove some processes to make space */
        for (int i = 1; i < 15; i++) {
            SYSCALL(TERMPROCESS, pids[i], 0, 0);
        }
        /* Ensure that it does not accept values except for true and false for
         * priority */
        SYSCALL(CREATEPROCESS, (size_t)rand(), 2, 0);
        assert(process_count == 0);
        /* TODO: Test the status and support parameters in the Createprocess,
         * idk how to test it */
    }
    ensure("terminate_process does not break if the input is broken")
    {
        active_process = spawn_process(false);
        state_t proc2;
        SYSCALL(CREATEPROCESS, (size_t)&proc2, true, 0);
        /* Test with a broken process id*/
        SYSCALL(TERMPROCESS, 99, 0, 0);
        assert(process_count == 0);
    }
    it("correctly terminates the active_process")
    {
        active_process = spawn_process(false);
        SYSCALL(TERMPROCESS, 0, 0, 0);
        assert(list_empty(&ready_queue_hi));
        assert(list_empty(&ready_queue_lo));
        assert(softblock_count == 0);
        assert(process_count == 0);
    }
    return 0;
}
